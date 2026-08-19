#ifndef CAMERA_CAPTURE_HPP
#define CAMERA_CAPTURE_HPP

#include <atomic>
#include <opencv2/videoio.hpp>
#include <string>
#include <thread>

#include "thread_safe_queue.hpp"

class CameraCapture {
 public:
  explicit CameraCapture(FramePipelineManager& pipeline,
                         const std::string& device_path =
                             "/dev/video4");  // TODO: remove hardcoded path
  explicit CameraCapture(FramePipelineManager& pipeline, int device_id);
  ~CameraCapture();

  bool isOpened() const;

  void start();
  void stop();

 private:
  FramePipelineManager& pipeline_;
  std::string device_path_;
  int device_id_{-1};

  std::atomic<bool> running_{false};
  std::thread thread_;
  cv::VideoCapture cap_;

  void run();
};

#endif  // CAMERA_CAPTURE_HPP
