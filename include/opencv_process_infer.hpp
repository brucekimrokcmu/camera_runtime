#ifndef OPENCV_PROCESS_INFER_HPP
#define OPENCV_PROCESS_INFER_HPP

#include <atomic>
#include <opencv2/core/types.hpp>
#include <optional>
#include <thread>

#include "frame_desc.hpp"
#include "latest_frame_mailbox.hpp"

class OpenCVProcessInfer {
 public:
  explicit OpenCVProcessInfer(LatestFrameMailbox& mailbox);
  ~OpenCVProcessInfer();

  void start();
  void stop();

 private:
  LatestFrameMailbox& mailbox_;
  std::atomic<bool> running_{false};
  std::thread thread_;
  bool stopped_{false};
  // thread loop
  void run();

  // Pipeline
  void process(FrameDesc& frame);

  // Vision Stage
  cv::Mat detect_color(const cv::Mat& frame);
  std::optional<cv::Rect> get_bounding_box(const cv::Mat& mask);
  std::optional<cv::Point3f> estimate_3d(const cv::Rect& bbox, const FrameDesc& frame);

  cv::Scalar lower_hsv_;
  cv::Scalar upper_hsv_;
};

#endif  // OPENCV_PROCESS_INFER_HPP
