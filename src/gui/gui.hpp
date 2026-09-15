#pragma once

namespace gui {

void init(GLFWwindow* window);

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

void toggleConfig();
void toggleInfo();

void render(entt::registry& registry);

void shutdown();

} // namespace gui

