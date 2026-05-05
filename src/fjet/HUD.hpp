#pragma once

#include "../engine/texture/Texture2D.hpp"
#include "../engine/text/Text.hpp"

class HUD {
public:
  HUD(Font* font);

  void updateSpeed(float s);
  void updateAltitude(float a);

  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;

private:
  struct Indicator {
    Mesh mesh;
    Texture2D* tex;
    vec3 color{0.f, 1.f, 0.f};

    void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;
  };

  Texture2D indicatorTex;

  Indicator speedIndicator;
  Indicator altitudeIndicator;

  Text speedText;
  Text altitudeText;
};

