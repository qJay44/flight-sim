#include "imgui.h"

#include "global.hpp"

namespace gui {

namespace {

size_t generalId = 0;

}

struct LoaderWidget {
  std::string bufLoad;
  std::string bufSave;
  size_t id;

  LoaderWidget(const std::string& name) {
    bufLoad.reserve(256);
    bufSave.reserve(256);
    bufLoad = bufSave =  name;
    id = generalId++;
  }

  ~LoaderWidget() {
    generalId--;
  }

  bool render(auto& cfg) {
    bool u = false;

    ImGui::PushID(id);

    ImGui::InputText(".json##0", bufLoad.data(), 256 * sizeof(char)); ImGui::SameLine();
    if (ImGui::Button("Load") && bufLoad[0]) {
      global::json::loadPreset(cfg, std::format("{}.json", bufLoad));
      u = true;
    }

    ImGui::InputText(".json##1", bufSave.data(), 256 * sizeof(char)); ImGui::SameLine();
    if (ImGui::Button("Save") && bufSave[0])
      global::json::savePreset(cfg, std::format("{}.json", bufSave));

    ImGui::PopID();

    return u;
  }
};

} // namespace gui

