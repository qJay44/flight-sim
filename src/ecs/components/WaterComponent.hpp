#pragma once

namespace ecs::component::terrain {

struct WaterComponent {
  vec3 oceanDeepColorLinear;
  vec3 oceanShallowColorLinear;
  float foamSharpness;
  float heightScaleScale;
  float waterDensity;
  float refractionDistortScale;
  float ambient;
  float diffusePowScale;
  float specularPowScale;
  float triplanarSharpness;
  float triplanarScale;
};

} // namespace ecs::component::terrain

