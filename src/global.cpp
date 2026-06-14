#include "global.hpp"

namespace global {

GLFWwindow* window = nullptr;
ProfilerManager* profiler = nullptr;

float dt = 0.f;
float time = 0.f;

bool wireframeMode = false;
bool guiFocused = false;
bool drawGrid = false;

bool jetDrawHUD = true;
bool jetDrawDebugMass = false;
bool jetDrawDebugHitboxes = false;

}// global

