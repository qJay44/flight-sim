#pragma once

#include <fstream>

#include "nlohmann/json.hpp"

namespace global {

static const union { vec3 right   {1.f, 0.f, 0.f}, red;   };
static const union { vec3 up      {0.f, 1.f, 0.f}, green; };
static const union { vec3 forward {0.f, 0.f, 1.f}, blue;  };

namespace json {
  // NLOHMANN_DEFINE_TYPE_INTRUSIVE(struct_t, field1, field2, ...)     // For private structs
  // NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(struct_t, field1, field2, ...) // For public structs

  void loadPreset(auto& cfg, std::string_view name) {
    fspath path = fspath("res/data/cfg") / name;

    std::ifstream f(path);

    if (f.is_open()) {
      nlohmann::json j;
      f >> j;
      j.get_to(cfg);
      f.close();
    } else {
      warning("[global::json::loadPreset] Could not open the file [{}]", path.string());
    }
  }

  void savePreset(auto& cfg, std::string_view name) {
    fspath path = fspath("res/data/cfg") / name;
    std::filesystem::create_directories(path.parent_path());

    std::ofstream f(path);

    if (f.is_open()) {
      nlohmann::json j = cfg;
      f << j.dump(2);
      f.close();
    } else {
      error("[global::json::savePreset] Could not open the file [{}]", path.string());
    }
  }

} // namespace json

} // namespace global

