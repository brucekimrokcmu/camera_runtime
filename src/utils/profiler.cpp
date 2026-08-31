#include "utils/profiler.hpp"

#include <chrono>
#include <fstream>

namespace utils {
// TODO: MemoryMetric Profiler::get_memory_usage() {}

void Profiler::dump_latency_csv(const std::string &filepath,
                                const std::vector<LatencyMetric> &metrics) {
  std::ofstream file(filepath);
  if (!file.is_open()) {
    return;
  }
  file << "name, frame_id, start_us, end_us, latency_ms\n";
  for (const auto &m : metrics) {
    auto start_us =
        std::chrono::duration_cast<std::chrono::microseconds>(m.start_time.time_since_epoch())
            .count();
    auto end_us =
        std::chrono::duration_cast<std::chrono::microseconds>(m.end_time.time_since_epoch())
            .count();
    file << m.name << "," << m.frame_id << "," << start_us << "," << end_us << "," << m.latency_ms()
         << "\n";
  }
}

}  // namespace utils
