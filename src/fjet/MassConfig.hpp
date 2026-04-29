#pragma once

namespace MassConfig {

constexpr float fuselage      = 0.17f;
constexpr float nose          = 0.04f;
constexpr float cockpit       = 0.07f;
constexpr float upperFuselage = 0.18f;
constexpr float engines       = 0.18f;
constexpr float wings         = 0.14f;
constexpr float leftAileron   = 0.01f;
constexpr float rightAileron  = 0.01f;
constexpr float leftFlap      = 0.01f;
constexpr float rightFlap     = 0.01f;
constexpr float leftElevator  = 0.01f;
constexpr float rightElevator = 0.01f;
constexpr float rudders       = 0.04f;
constexpr float leftRudder    = 0.02f;
constexpr float rightRudder   = 0.02f;
constexpr float canopy        = 0.07f;
constexpr float airbrake      = 0.01f;

constexpr float total =
  fuselage      +
  nose          +
  cockpit       +
  upperFuselage +
  engines       +
  wings         +
  leftAileron   +
  rightAileron  +
  leftFlap      +
  rightFlap     +
  leftElevator  +
  rightElevator +
  rudders       +
  leftRudder    +
  rightRudder   +
  canopy        +
  airbrake;

static_assert(total > 0.999f && total < 1.001f);

}

