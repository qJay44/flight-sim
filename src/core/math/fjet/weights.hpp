#pragma once

namespace core::math::fjet::weights {

enum Part {
  IDX_FUSELAGE,
  IDX_NOSE,
  IDX_COCKPIT,
  IDX_UPPER_FUSELAGE,
  IDX_ENGINES,
  IDX_WINGS,
  IDX_LEFT_AILERON,
  IDX_RIGHT_AILERON,
  IDX_LEFT_FLAP,
  IDX_RIGHT_FLAP,
  IDX_LEFT_ELEVATOR,
  IDX_RIGHT_ELEVATOR,
  IDX_RUDDERS,
  IDX_LEFT_RUDDER,
  IDX_RIGHT_RUDDER,
  IDX_CANOPY,
  IDX_AIRBRAKE,
};

static const std::unordered_map<std::string, Part> strToPart = {
  {"Fuselage"     , IDX_FUSELAGE},
  {"Nose"         , IDX_NOSE},
  {"Cockpit"      , IDX_COCKPIT},
  {"UpperFuselage", IDX_UPPER_FUSELAGE},
  {"Engines"      , IDX_ENGINES},
  {"Wings"        , IDX_WINGS},
  {"LeftAileron"  , IDX_LEFT_AILERON},
  {"RightAileron" , IDX_RIGHT_AILERON},
  {"LeftFlap"     , IDX_LEFT_FLAP},
  {"RightFlap"    , IDX_RIGHT_FLAP},
  {"LeftElevator" , IDX_LEFT_ELEVATOR},
  {"RightElevator", IDX_RIGHT_ELEVATOR},
  {"Rudders"      , IDX_RUDDERS},
  {"LeftRudder"   , IDX_LEFT_RUDDER},
  {"RightRudder"  , IDX_RIGHT_RUDDER},
  {"Canopy"       , IDX_CANOPY},
  {"Airbrake"     , IDX_AIRBRAKE},
};

constexpr float weights[17] = {
  0.17f,
  0.04f,
  0.07f,
  0.18f,
  0.18f,
  0.14f,
  0.01f,
  0.01f,
  0.01f,
  0.01f,
  0.01f,
  0.01f,
  0.04f,
  0.02f,
  0.02f,
  0.07f,
  0.01f,
};

namespace {

constexpr float total =
  weights[IDX_FUSELAGE      ] +
  weights[IDX_NOSE          ] +
  weights[IDX_COCKPIT       ] +
  weights[IDX_UPPER_FUSELAGE] +
  weights[IDX_ENGINES       ] +
  weights[IDX_WINGS         ] +
  weights[IDX_LEFT_AILERON  ] +
  weights[IDX_RIGHT_AILERON ] +
  weights[IDX_LEFT_FLAP     ] +
  weights[IDX_RIGHT_FLAP    ] +
  weights[IDX_LEFT_ELEVATOR ] +
  weights[IDX_RIGHT_ELEVATOR] +
  weights[IDX_RUDDERS       ] +
  weights[IDX_LEFT_RUDDER   ] +
  weights[IDX_RIGHT_RUDDER  ] +
  weights[IDX_CANOPY        ] +
  weights[IDX_AIRBRAKE      ];
static_assert(total > 0.999f && total < 1.001f);

} // namespace

} // namespace core::math::fjet::weights

