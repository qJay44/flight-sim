#include "fjet/FighterJet.hpp"
#ifdef _WIN32
  #include <direct.h>
  #define CHDIR(p) _chdir(p);
#else
  #include <unistd.h>
  #define CHDIR(p) chdir(p);
#endif

#include "global.hpp"
#include "engine/gui/gui.hpp"
#include "engine/Camera.hpp"
#include "engine/Shader.hpp"
#include "engine/InputsHandler.hpp"
#include "engine/mesh/meshes.hpp"
#include "engine/Light.hpp"
#include "engine/texture/Texture2D.hpp"
#include "utils/clrp.hpp"

using global::window;

void GLAPIENTRY MessageCallback(
  GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam
) {
  static const clrp::clrp_t clrpError{clrp::ATTRIBUTE::BOLD, clrp::FG::RED};
  static const clrp::clrp_t clrpWarning{clrp::ATTRIBUTE::BOLD, clrp::FG::YELLOW};

  clrp::clrp_t clrpFinal = clrpError;

  switch (source) {
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      return; // Handled by the Shader class itself
    case GL_DEBUG_SOURCE_API:
      clrpFinal = clrpWarning; // "SIMD32 shader inefficient", skipping since occurs only on my laptop
  }

  fprintf(
    stderr, "GL CALLBACK: %s source = 0x%x, id = 0x%x type = 0x%x, severity = 0x%x, message = %s\n",
    (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), source, id, type, severity, clrp::format(message, clrpFinal).c_str()
  );
}

int main() {
  // Assuming the executable is launching from its own directory
  CHDIR("../../..");

  // GLFW init
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

  // Window init
  window = glfwCreateWindow(1600, 900, "MyProgram", NULL, NULL);
  global::profiler = new ProfilerManager(300);
  ivec2 winSize = global::getWinSize();
  dvec2 winCenter = dvec2(winSize) / 2.;

  if (!window) {
    printf("Failed to create GFLW window\n");
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwMakeContextCurrent(window);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + 2 * !global::guiFocused);
  glfwSetCursorPos(window, winCenter.x, winCenter.y);

  // GLAD init
  if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    return EXIT_FAILURE;
  }

  glViewport(0, 0, winSize.x, winSize.y);
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(MessageCallback, 0);

  gui::init();

  // ===== Shaders ============================================== //

  Shader::setDirectoryLocation("src/engine/shaders");

  Shader lightShader("light.vert", "light.frag");
  Shader linesShader("lines.vert", "lines.frag");
  Shader airplaneShader("f15.vert", "f15.frag");
  Shader billboardColorShader("billboardColor.vert", "billboardColor.frag");

  // ===== Cameras ============================================== //

  Camera cameraSpectate({85.f, 77.f, 76.f}, -2.385f, -0.582f);
  cameraSpectate.setFarPlane(1000.f);
  cameraSpectate.setSpeedDefault(50.f);

  // ===== Inputs Handler ======================================= //

  InputsHandler::mousePos = global::getWinCenter();
  glfwSetScrollCallback(window, InputsHandler::scrollCallback);
  glfwSetKeyCallback(window, InputsHandler::keyCallback);
  glfwSetCursorPosCallback(window, InputsHandler::cursorPosCallback);

  // ============================================================ //

  Light light({0.f, 200.f, 0.f});

  FighterJet f15("res/fbx/f15.fbx", 13000.f);
  f15.setCamDistance(30.f);
  f15.setCamSensitivity(100.f);
  f15.setMaxThrust(210000.f);

  Mesh axis = meshes::axis();
  axis.scale(1e4f);

  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  gui::camPtr = &cameraSpectate;
  gui::lightPtr = &light;
  gui::fjetPtr= &f15;

  global::drawGlobalAxis = true;

  // Render loop
  while (!glfwWindowShouldClose(window)) {
    static double titleTimer = glfwGetTime();
    static double prevTime = titleTimer;
    static double currTime = prevTime;
    const auto& activeCam = Camera::activeCam;

    constexpr double fpsLimit = 1. / 90.;
    currTime = glfwGetTime();
    global::dt = currTime - prevTime;

    // FPS cap
    if (global::dt < fpsLimit) continue;
    else prevTime = currTime;

    global::time += global::dt;

    if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
      InputsHandler::process();
      activeCam->update();
      InputsHandler::activeEntity = &(f15.isActive() ? f15 : (Moveable&)cameraSpectate);
    } else
      glfwSetCursorPos(window, winCenter.x, winCenter.y);

    // Update window title every 0.3 seconds
    if (currTime - titleTimer >= 0.3) {
      gui::fps = static_cast<u16>(1.f / global::dt);
      titleTimer = currTime;
    }

    global::profiler->clearTasks();

    f15.update();
    light.update();
    light.setUniforms(airplaneShader);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE); // Disable for plane meshes, enable for volumetric meshes
    glEnable(GL_DEPTH_TEST); // Disable to ignore depth (draw one object over another one without discarding the farthest)

    f15.draw(activeCam, airplaneShader);
    f15.drawDebug(activeCam, billboardColorShader);

    glDisable(GL_CULL_FACE);

    light.draw(activeCam, lightShader);

    if (global::drawGlobalAxis)
      axis.draw(activeCam, linesShader);

    // ============================================================ //

    gui::draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  gui::shutdown();
  glfwTerminate();

  return 0;
}

