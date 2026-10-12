#pragma once

#include "../ecs/components/CameraComponent.hpp"
#include "../ecs/components/TransformComponent.hpp"
#include "../ecs/components/TerrainComponent.hpp"
#include "../ecs/components/WaterComponent.hpp"
#include "../ecs/components/MeshComponent.hpp"
#include "../ecs/components/InputComponent.hpp"
#include "../ecs/components/VelocityComponent.hpp"
#include "../core/ActiveCamera.hpp"

namespace gui {

using namespace ecs::component;

void drawMeshComponent(MeshComponent& component);
void drawTransformComponent(TransformComponent& component);
void drawCameraComponent(CameraComponent& component, core::ActiveCamera& activeCam);
void drawInputComponent(InputComponent& component);
void drawVelocityComponent(VelocityComponent& component);
void drawTerrainComponent(terrain::TerrainComponent& component);
void drawWaterComponent(terrain::WaterComponent& component);

} // namespace gui

