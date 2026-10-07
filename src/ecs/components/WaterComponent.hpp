#pragma once

namespace ecs::component::terrain {

struct WaterComponent {
  float foamSharpness = 1.f;
  float sunIntensity = 7.f;
  float heightScaleScale = 0.1f;
};

} // namespace ecs::component::terrain

