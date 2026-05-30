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
#include "Environment.hpp"
#include "other/markup.hpp"
#include "utils/clrp.hpp"
#include "other/Grid.hpp"
#include "other/Sun.hpp"

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
  markup::init();

  // ===== Shaders ============================================== //

  Shader::setDirectoryLocation("src/shaders");

  Shader environmentShader("environment.vert", "environment.frag");
  Shader colorShader("color.vert", "color.frag");
  Shader hudShader("hud.vert", "hud.frag");
  Shader gridShader("grid.vert", "grid.frag");
  Shader textShader("text.vert", "text.frag");
  Shader markupShader("markup.vert", "markup.frag");

  Shader airplaneShader("f15/f15.vert", "f15/f15.frag");
  Shader massShader("f15/mass.vert", "f15/mass.frag");
  Shader hitboxShader("f15/hitbox.vert", "f15/hitbox.frag", "f15/hitbox.geom");

  // ===== Cameras ============================================== //

  Camera cameraSpectate({85.f, 77.f, 76.f}, -2.385f, -0.582f);
  cameraSpectate.setFarPlane(1000.f);
  cameraSpectate.setSpeedDefault(50.f);

  // ===== Inputs Handler ======================================= //

  InputsHandler::mousePos = global::getWinCenter();
  glfwSetScrollCallback(window, InputsHandler::scrollCallback);
  glfwSetKeyCallback(window, InputsHandler::keyCallback);
  glfwSetCursorPosCallback(window, InputsHandler::cursorPosCallback);

  // ===== Jet ================================================== //

  Font textFont("res/fonts/FiraCodeNerdFontMono-Bold.ttf", 20);

  FighterJet f15("res/fbx/f15.fbx", 13000.f, &textFont, &textShader);
  f15.setCamDistance(30.f);
  f15.setCamSensitivity(1.f);

  auto& f15Config = f15.getBodyConfig();
  f15Config.stiffness = 500.f;
  f15Config.airbrakeDrag = 10.f;
  f15Config.maxThrust = 6e5f;
  f15Config.flapsLiftPower = 2.f;
  f15Config.flapsAOABias = 10.f;
  f15Config.liftPower = 20.f;
  f15Config.rudderPower = 50.f;
  f15Config.meshScale = 0.01f;
  f15Config.inducedDrag = 10.5f;
  f15Config.turnSpeed = vec3(40.f, 20.f, 120.f);
  f15Config.turnAcceleration = vec3(5e5f, 2e5f, 1e6f);

  // ============================================================ //

  Grid grid{};

  Environment env = Environment::createDefault("res/tex/Cubemaps/Cubemap_Sky_04-512x512.png");

  gui::camPtr = &cameraSpectate;
  gui::sunPtr = &env.sun;
  gui::fjetPtr = &f15;
  InputsHandler::controlledPlane = &f15;

  global::drawGlobalAxis = true;

  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

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

    dvec2 winCenter = global::getWinCenter();
    if (!global::guiFocused)
      glfwSetCursorPos(global::window, winCenter.x, winCenter.y);

    // Update window title every 0.3 seconds
    if (currTime - titleTimer >= 0.3) {
      gui::fps = static_cast<u16>(1.f / global::dt);
      titleTimer = currTime;
    }

    global::profiler->clearTasks();

    f15.update();

    env.sun.setUniforms(airplaneShader);
    env.sun.setUniforms(environmentShader);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    env.draw(activeCam, environmentShader);

    grid.draw(activeCam, gridShader);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    f15.draw(activeCam, airplaneShader);

    if (global::jetDrawDebugMass)
      f15.drawDebugMass(activeCam, massShader);

    if (global::jetDrawDebugHitboxes)
      f15.drawDebugHitboxes(activeCam, hitboxShader);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    if (global::jetDrawHUD)
      f15.drawHUD(activeCam, hudShader);

    // markup::drawCross(activeCam, markupShader);

    // ============================================================ //

    gui::draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  gui::shutdown();
  glfwTerminate();

  return 0;
}

