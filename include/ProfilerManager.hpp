#pragma once

#undef IM_NEW
#undef IM_FREE

#include "ImGuiProfilerRenderer.h"
#include "ProfilerTask.h"

class ProfilerManager {
public:
  class ScopedTaskCpu {
  public:
    ScopedTaskCpu(size_t taskIdx);
    ~ScopedTaskCpu();

    void end();

  private:
    size_t taskIdx;
    std::chrono::steady_clock::time_point start;
    bool ended = false;
  };

  struct Query {
    GLuint q0, q1;
    std::string name{};

    Query() = default;
    Query(const std::string& name);

    double calcDuration() const;
  };

  class ScopedTaskGpu {
  public:
    ScopedTaskGpu(size_t taskIdx, const Query& q);
    ~ScopedTaskGpu();

    void end();

  private:
    GLuint64 t0;

    size_t taskIdx;
    const Query& q;
    bool ended = false;
  };

  ProfilerManager(size_t framesCount);

  // NOTE: Call this every frame
  void clearTasks();

  ProfilerManager::ScopedTaskCpu startScopedTaskCpu(const std::string& name, u32 color = 0);
  ProfilerManager::ScopedTaskGpu startScopedTaskGpu(const Query& q, u32 color = 0);

  void renderTasks(int graphWidth, int legendWidth, int height, int frameIndexOffset = 0);

private:
  std::vector<legit::ProfilerTask> cpuTasks;
  std::vector<legit::ProfilerTask> gpuTasks;
  ImGuiUtils::ProfilersWindow window;

private:
  static const u32& getColorBright(size_t i);
  static const u32& getColorDim(size_t i);

  void endTaskCpu(size_t i, double durationMs);
  void endTaskGpu(size_t i, double durationMs);
};

