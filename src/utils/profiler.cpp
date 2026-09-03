#include "utils/profiler.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>

namespace utils {
// TODO: MemoryMetric Profiler::get_memory_usage() {}

Profiler &Profiler::instance() {
  static Profiler inst;
  return inst;
}

void Profiler::record_metric(LatencyMetric metric) {
  std::lock_guard<std::mutex> lock(mutex_);
  metrics_.push_back(std::move(metric));
}

void Profiler::clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  metrics_.clear();
}

void Profiler::dump_latency_csv(const std::string &filepath) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::ofstream file(filepath);
  if (!file.is_open()) {
    return;
  }
  file << "name, frame_id, start_us, end_us, latency_ms\n";
  for (const auto &m : metrics_) {
    auto start_us =
        std::chrono::duration_cast<std::chrono::microseconds>(m.start_time.time_since_epoch())
            .count();
    auto end_us =
        std::chrono::duration_cast<std::chrono::microseconds>(m.end_time.time_since_epoch())
            .count();
    file << m.name << "," << m.frame_id << "," << start_us << "," << end_us << "," << m.latency_ms()
         << "\n";
  }
  std::cout << "[Profiler] Successfully saved metrics to " << filepath << std::endl;
}

}  // namespace utils
