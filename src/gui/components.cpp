#include "components.hpp"

#include "imgui.h"
#include <glm/ext/quaternion_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gui {

using namespace ecs::component;

void drawMeshComponent(MeshComponent& component) {
  if (ImGui::TreeNode("Mesh component")) {
    ImGui::Checkbox("Disabled", &component.disabled);
    ImGui::BeginDisabled(component.disabled);

    ImGui::SeparatorText("Mesh");

    ImGui::Text("Polygon mode");
    {
      static auto rbPolygonMode = GL_FILL;
      bool u = false;

      if (ImGui::RadioButton("GL_FILL", rbPolygonMode == GL_FILL))   rbPolygonMode = GL_FILL , u = true;
      if (ImGui::RadioButton("GL_LINE", rbPolygonMode == GL_LINE))   rbPolygonMode = GL_LINE , u = true;
      if (ImGui::RadioButton("GL_POINT", rbPolygonMode == GL_POINT)) rbPolygonMode = GL_POINT, u = true;
      if (u)
        component.mesh->setPolygonMode(rbPolygonMode);
    }

    ImGui::EndDisabled();

    ImGui::SeparatorText("Shader");
    if (ImGui::Button("Reload"))
      component.shader->reload();

    ImGui::TreePop();
  }
}

void drawTransformComponent(TransformComponent& component) {
  if (ImGui::TreeNode("Transform component")) {
    static float rad = 0.f;
    auto& quat = component.rotation;

    ImGui::Text("Quaternion: [%.2f, %.2f, %.2f, %.2f]", quat.x, quat.y, quat.z, quat.w);
    ImGui::DragFloat3("Position", glm::value_ptr(component.pos));
    ImGui::DragFloat3("Scale", glm::value_ptr(component.scale));

    ImGui::SeparatorText("Rotate by");
    ImGui::SliderAngle("degrees", &rad);

    ImGui::Spacing();
    ImGui::Text("Around");

    if (ImGui::Button("Right"))   {quat = glm::rotate(quat, rad, vec3(1.f, 0.f, 0.f));} ImGui::SameLine();
    if (ImGui::Button("Up"))      {quat = glm::rotate(quat, rad, vec3(0.f, 1.f, 0.f));} ImGui::SameLine();
    if (ImGui::Button("Forward")) {quat = glm::rotate(quat, rad, vec3(0.f, 0.f, 1.f));}

    ImGui::TreePop();
  }
}

void drawCameraComponent(CameraComponent& component, core::ActiveCamera& activeCam) {
  bool isActive = component.cam == activeCam.cam;

  ImGui::PushID("custom_tree");
  ImGui::SetNextItemOpen(isActive, ImGuiCond_FirstUseEver);
  bool open = ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_SpanAvailWidth);

  ImGui::SameLine(0.f, 0.f);
  ImGui::Text("Camera component ");

  ImGui::SameLine(0.f, 0.f);
  ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "%s", isActive ? "(Active)" : "");

  if (open) {
    auto* cam = component.cam;

    ImGui::Text("Up: [%.2f, %.2f, %2.f]", cam->up.x, cam->up.y, cam->up.z);
    ImGui::Text("Orientation: [%.2f, %.2f, %2.f]", cam->orientation.x, cam->orientation.y, cam->orientation.z);

    ImGui::SliderFloat("Near", &cam->nearPlane, 0.01f, 10.f);
    ImGui::DragFloat("Far", &cam->farPlane, 1.f, 1e5f);
    ImGui::SliderAngle("Fov", &cam->fov, 0.f, 120.f);
    ImGui::SliderAngle("Yaw", &cam->yaw, -180.f, 180.f);
    ImGui::SliderAngle("Pitch", &cam->pitch, -89.f, 89.f);
    ImGui::SliderFloat("Sensitivity", &cam->sensitivity, 0.1f, 10.f);

    if (!isActive) {
      if (ImGui::Button("Make active")) {
        activeCam.cam = cam;
      }
    }

    ImGui::TreePop();
  }

  ImGui::PopID();
}

void drawInputComponent(InputComponent& component) {
  if (ImGui::TreeNode("Input component")) {
    ImGui::Checkbox("Disabled", &component.disabled);
    ImGui::BeginDisabled(component.disabled);
    ImGui::SliderFloat("Shift multiplier", &component.shiftMultiplier, 0.f, 50.f);
    ImGui::EndDisabled();
    ImGui::TreePop();
  }
}

void drawVelocityComponent(VelocityComponent& component) {
  if (ImGui::TreeNode("Velocity component")) {
    const auto& v = component.velocity;
    ImGui::Text("Velocity: [%.2f, %.2f, %.2f]", v.x, v.y, v.z);
    ImGui::SliderFloat("Scale", &component.scale, 0.f, 50.f);
    ImGui::TreePop();
  }
}

void drawTerrainComponent(terrain::TerrainComponent& component) {
  if (ImGui::TreeNode("Terrain component")) {
    float nodesUsed = static_cast<float>(component.activeLeafs) / TERRAIN_MAX_NODES;
    vec4 nodesColor = nodesUsed > 0.8f ? vec4(1.f, 0.f, 0.f, 1.f) : nodesUsed > 0.5f ? vec4(1.f, 1.f, 0.f, 1.f) : vec4(1.f);
    ImGui::TextColored(nodesColor, "Active leafs: [%zu] / [%d]", component.activeLeafs, TERRAIN_MAX_NODES);

    ImGui::Spacing();

    ImGui::SliderFloat("Sea threshold", &component.seaThreshold, 0.f, 1.f);
    ImGui::SliderFloat("Sand threshold", &component.sandThreshold, 0.f, 1.f);
    ImGui::SliderFloat("Moutain threshold", &component.mountainThreshold, 0.f, 1.f);
    ImGui::SliderInt("Quadtree max depth", &component.qtMaxDepth, 1, 20);
    ImGui::SliderFloat("Quadtree split threshold", &component.qtSplitThreshold, 0.f, 1.f);

    ImGui::TreePop();
  }
}

void drawWaterComponent(terrain::WaterComponent& component) {
  if (ImGui::TreeNode("Water component")) {
    ImGui::SliderFloat("Foam sharpness", &component.foamSharpness, 0.f, 20.f);
    ImGui::SliderFloat("Scale of height scale", &component.heightScaleScale, 0.f, 1.f);
    ImGui::SliderFloat("Tile scale", &component.tileScale, 0.f, 50.f);
    ImGui::SliderFloat("Displacement scale", &component.displacementScale, 1.f, 10.f);

    ImGui::TreePop();
  }
}

} // namespace gui

