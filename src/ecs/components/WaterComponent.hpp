#pragma once

namespace ecs::component::terrain {

struct WaterComponent {
  vec3 oceanDeepColorLinear;
  vec3 oceanShallowColorLinear;
  float foamSharpness;
  float heightScaleScale;
  float tileScale;
  float displacementScale;
  float waterDensity;
  float refractionDistortScale;
  float normalToScatterScale;
  float ambient;
  float diffusePowScale;
  float specularPowScale;
};

} // namespace ecs::component::terrain

