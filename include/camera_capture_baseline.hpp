
#ifndef CAMERA_CAPTURE_BASELINE_HPP
#define CAMERA_CAPTURE_BASELINE_HPP

#include <atomic>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <string>
#include <thread>

#include "latest_frame_mailbox.hpp"

class CameraCaptureBaseline {
 public:
  explicit CameraCaptureBaseline(LatestFrameMailbox& mailbox);
  explicit CameraCaptureBaseline(LatestFrameMailbox& mailbox, int device_id);
  ~CameraCaptureBaseline();

  bool isOpened() const;
  void start();
  void stop();

 private:
  LatestFrameMailbox& mailbox_;
  std::thread thread_;
  std::string device_path_;
  int device_id_{-1};
  cv::VideoCapture cap_;
  std::atomic<bool> running_{false};
  void run();
};
#endif  // CAMERA_CAPTURE_BASELINE_HPP
