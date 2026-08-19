#ifndef PREPROCESS_INFER_HPP
#define PREPROCESS_INFER_HPP

#include <atomic>
#include <thread>

#include "thread_safe_queue.hpp"

class PreprocessInferConsumer {
 public:
  explicit PreprocessInferConsumer(FramePipelineManager& pipeline);
  ~PreprocessInferConsumer();

  void start();
  void stop();

 private:
  FramePipelineManager& pipeline_;
  std::atomic<bool> running_{false};
  std::thread thread_;

  void run();
};

#endif  // PREPROCESS_INFER_HPP
