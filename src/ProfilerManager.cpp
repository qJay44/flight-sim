#include "ProfilerManager.hpp"
#include "global.hpp"

using ScopedTask = ProfilerManager::ScopedTask;

ScopedTask::ScopedTask(size_t taskIdx) : taskIdx(taskIdx), start(std::chrono::steady_clock::now()) {}

ScopedTask::~ScopedTask() {
  end();
}

void ScopedTask::end() {
  if (std::exchange(ended, true))
    return;

  using namespace std::chrono;

  auto end = steady_clock::now();
  auto dur = end - start;
  float durationSec = duration_cast<duration<float>>(dur).count();

  assert(global::profiler);
  global::profiler->endTask(taskIdx, durationSec);
}

ProfilerManager::ProfilerManager(size_t framesCount) : graph(framesCount) {}

// NOTE: Call this every frame
void ProfilerManager::clearTasks() {
  tasks.clear();
}

ProfilerManager::ScopedTask ProfilerManager::startScopedTask(const std::string& name, u32 color) {
  assert(tasks.size() < 20);
  size_t taskIdx = tasks.size();

  legit::ProfilerTask task;
  task.name = name;
  task.color = color ? RGBA_LE(color): getColorBright(tasks.size());
  task.startTime = 0.f;
  tasks.push_back(task);

  return taskIdx;
}

void ProfilerManager::renderTasks(int graphWidth, int legendWidth, int height, int frameIndexOffset) {
  graph.LoadFrameData(tasks.data(), tasks.size());
  graph.RenderTimings(graphWidth, legendWidth, height, frameIndexOffset, 1.f / 60.f);
}

const u32& ProfilerManager::getColorBright(size_t i) {
  using namespace legit::Colors;

  static constexpr u32 colors[8] = {
    turqoise, emerald, peterRiver, amethyst, sunFlower, carrot, alizarin, clouds
  };

  return colors[i % 8];
}

const u32& ProfilerManager::getColorDim(size_t i) {
  using namespace legit::Colors;

  static constexpr u32 colors[8] = {
    greenSea, nephritis, belizeHole, wisteria, orange, pumpkin, pomegranate, silver
  };

  return colors[i % 8];
}

void ProfilerManager::endTask(size_t i, float durationMs) {
  assert(i < tasks.size());
  tasks[i].endTime = durationMs;
}

