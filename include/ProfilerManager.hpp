#pragma once

#undef IM_NEW
#undef IM_FREE

#include "ImGuiProfilerRenderer.h"
#include "ProfilerTask.h"

class ProfilerManager {
public:
  class ScopedTask {
  public:
    ScopedTask(size_t taskIdx);
    ~ScopedTask();

    void end();

  private:
    size_t taskIdx;
    std::chrono::steady_clock::time_point start;
    bool ended = false;
  };

  ProfilerManager(size_t framesCount);

  // NOTE: Call this every frame
  void clearTasks();

  ProfilerManager::ScopedTask startScopedTask(const std::string& name, u32 color = 0);

  void renderTasks(int graphWidth, int legendWidth, int height, int frameIndexOffset = 0);

private:
  std::vector<legit::ProfilerTask> tasks;
  ImGuiUtils::ProfilerGraph graph;

private:
  static const u32& getColorBright(size_t i);
  static const u32& getColorDim(size_t i);

  void endTask(size_t i, float durationMs);
};

