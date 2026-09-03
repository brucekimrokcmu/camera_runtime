#ifndef PROFILER_HPP
#define PROFILER_HPP

#include <chrono>
#include <mutex>
#include <ratio>
#include <string>
#include <vector>

namespace utils {

struct LatencyMetric {
  std::string name;
  uint64_t frame_id;
  std::chrono::steady_clock::time_point start_time;
  std::chrono::steady_clock::time_point end_time;

  double latency_ms() const {
    return std::chrono::duration<double, std::milli>(end_time - start_time).count();
  }
};

// MemoryMetric
struct MemoryMetric {
  size_t rss_kb{0};
  size_t pss_kb{0};
};

class Profiler {
 public:
  static Profiler& instance();

  static std::chrono::steady_clock::time_point now() { return std::chrono::steady_clock::now(); }

  static MemoryMetric get_memory_usage();

  void record_metric(LatencyMetric metric);

  void dump_latency_csv(const std::string& filepath);

  void clear();

 private:
  Profiler() = default;
  ~Profiler() = default;
  Profiler(const Profiler&) = delete;
  Profiler& operator=(const Profiler&) = delete;

  std::mutex mutex_;
  std::vector<LatencyMetric> metrics_;
};

// RAII Timer Scope Helper
class ScopedTimer {
 public:
  ScopedTimer(std::string name, uint64_t frame_id)
      : name_(name), frame_id_(frame_id), start_(Profiler::now()) {};

  ~ScopedTimer() {
    auto end = Profiler::now();
    Profiler::instance().record_metric({name_, frame_id_, start_, end});
  };

 private:
  std::string name_;
  uint64_t frame_id_;
  std::chrono::steady_clock::time_point start_;
};

}  // namespace utils

#endif  // PROFILER_HPP
