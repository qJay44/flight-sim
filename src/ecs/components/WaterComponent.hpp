#pragma once

namespace ecs::component::terrain {

struct WaterComponent {
  float foamSharpness = 1.f;
  float heightScaleScale = 0.1f;
  float tileScale = 1.f;
};

} // namespace ecs::component::terrain

