#ifndef CAMERA_CAPTURE_HPP
#define CAMERA_CAPTURE_HPP

#include <atomic>
#include <thread>

#include "thread_safe_queue.hpp"

class CameraCapture {
 private:
  FramePipelineManager& pipeline_;
  std::atomic<bool> running_{false};
  std::thread thread_;

  void run();

 public:
  explicit CameraCapture(FramePipelineManager& pipeline);
  ~CameraCapture();

  void start();
  void stop();
};

#endif  // CAMERA_CAPTURE_HPP
