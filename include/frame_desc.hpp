#ifndef FRAME_DESC_HPP
#define FRAME_DESC_HPP

#include <chrono>
#include <opencv2/opencv.hpp>

struct FrameDesc {
  uint64_t frame_id{0};
  std::chrono::steady_clock::time_point timestamp;
  cv::Mat image;
};

#endif  // FRAME_DESC_HPP
