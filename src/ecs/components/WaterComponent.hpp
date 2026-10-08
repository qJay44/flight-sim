#pragma once

namespace ecs::component::terrain {

struct WaterComponent {
  float foamSharpness = 1.f;
  float heightScaleScale = 0.1f;
  int quadsPerAxis = 4;
};

} // namespace ecs::component::terrain

