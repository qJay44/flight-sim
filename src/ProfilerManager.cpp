#include "ProfilerManager.hpp"
#include "global.hpp"

// ----- ScopedTaskCpu ----------------------------------------------------------------------------------------------------------- //

using ScopedTaskCpu = ProfilerManager::ScopedTaskCpu;

ScopedTaskCpu::ScopedTaskCpu(size_t taskIdx) : taskIdx(taskIdx), start(std::chrono::steady_clock::now()) {}

ScopedTaskCpu::~ScopedTaskCpu() {
  end();
}

void ScopedTaskCpu::end() {
  if (std::exchange(ended, true))
    return;

  using namespace std::chrono;

  auto end = steady_clock::now();
  auto dur = end - start;
  auto durationSec = duration_cast<duration<double>>(dur).count();

  assert(global::profiler);
  global::profiler.endTaskCpu(taskIdx, durationSec);
}

// ----- ScopedTaskGpu ----------------------------------------------------------------------------------------------------------- //

using ScopedTaskGpu = ProfilerManager::ScopedTaskGpu;

ScopedTaskGpu::ScopedTaskGpu(size_t taskIdx, const Query& q) : taskIdx(taskIdx), q(q) {
  glQueryCounter(q.q0, GL_TIMESTAMP);
}

ScopedTaskGpu::~ScopedTaskGpu() {
  end();
}

void ScopedTaskGpu::end() {
  if (std::exchange(ended, true))
    return;

  glQueryCounter(q.q1, GL_TIMESTAMP);

  assert(global::profiler);
  global::profiler.endTaskGpu(taskIdx, q.calcDuration());
}

// ----- Qurie ------------------------------------------------------------------------------------------------------------------- //

ProfilerManager::Query::Query(const std::string& name) : name(name) {
  glGenQueries(1, &q0);
  glGenQueries(1, &q1);
}

double ProfilerManager::Query::calcDuration() const {
  constexpr double toSecondsInv = 1.0 / 1e9;

  GLuint64 t0;
  GLuint64 t1;
  glGetQueryObjectui64v(q0, GL_QUERY_RESULT, &t0);
  glGetQueryObjectui64v(q1, GL_QUERY_RESULT, &t1);

  return static_cast<double>(t1 - t0) * toSecondsInv;
}

// --------------------------------------------------------------------------------------------------------------------------------- //

ProfilerManager::ProfilerManager(size_t framesCount) : window(1.f / framesCount) {}

// NOTE: Call this every frame
void ProfilerManager::clearTasks() {
  cpuTasks.clear();
  gpuTasks.clear();
}

ProfilerManager::ScopedTaskCpu ProfilerManager::startScopedTaskCpu(const std::string& name, u32 color) {
  assert(cpuTasks.size() < 32);
  size_t taskIdx = cpuTasks.size();

  legit::ProfilerTask task;
  task.name = name;
  task.color = color ? RGBA_LE(color): getColorBright(cpuTasks.size());
  task.startTime = 0.0;

  cpuTasks.push_back(task);

  return taskIdx;
}

ProfilerManager::ScopedTaskGpu ProfilerManager::startScopedTaskGpu(const Query& q, u32 color) {
  assert(gpuTasks.size() < 32);
  size_t taskIdx = gpuTasks.size();

  legit::ProfilerTask task;
  task.name = q.name;
  task.color = color ? RGBA_LE(color): getColorBright(gpuTasks.size());
  task.startTime = 0.0;

  gpuTasks.push_back(task);

  return {taskIdx, q};
}

void ProfilerManager::renderTasks(int graphWidth, int legendWidth, int height, int frameIndexOffset) {
  window.cpuGraph.LoadFrameData(cpuTasks.data(), cpuTasks.size());
  window.gpuGraph.LoadFrameData(gpuTasks.data(), gpuTasks.size());
  window.Render();
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

void ProfilerManager::endTaskCpu(size_t i, double durationMs) {
  assert(i < cpuTasks.size());
  cpuTasks[i].endTime = durationMs;
}

void ProfilerManager::endTaskGpu(size_t i, double durationMs) {
  assert(i < gpuTasks.size());
  gpuTasks[i].endTime = durationMs;
}

