#pragma once

#include "../engine/texture/Texture2D.hpp"
#include "../engine/text/Text.hpp"
#include "../engine/mesh/MeshElements.hpp"
#include "../engine/mesh/meshes.hpp"

class HUD {
public:
  HUD() = default;
  HUD(Font* font, Shader* textShader);

  void updateSpeed(float s);
  void updateAltitude(float a);

  void draw(const Camera* camera, Shader& shader) const;

private:
  Shader* shaderText = nullptr;

  Texture2D indicatorTex;

  MeshElements speedImg = meshes::rectangle();
  MeshElements altitudeImg = meshes::rectangle();

  Text speedText;
  Text altitudeText;
};

