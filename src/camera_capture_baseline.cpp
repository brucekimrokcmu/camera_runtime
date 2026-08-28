#include "camera_capture_baseline.hpp"

#include <chrono>
#include <cstdint>
#include <opencv2/videoio.hpp>

#include "latest_frame_mailbox.hpp"

CameraCaptureBaseline::CameraCaptureBaseline(LatestFrameMailbox& mailbox, std::string device_path)
    : mailbox_(mailbox), device_path_(device_path) {}

CameraCaptureBaseline::CameraCaptureBaseline(LatestFrameMailbox& mailbox, int device_id)
    : mailbox_(mailbox), device_id_(device_id) {}

CameraCaptureBaseline::~CameraCaptureBaseline() { stop(); }

bool CameraCaptureBaseline::isOpened() const { return cap_.isOpened(); }

void CameraCaptureBaseline::start() {
  if (running_.exchange(true)) {
    return;
  }
  thread_ = std::thread(&CameraCaptureBaseline::run, this);
};

void CameraCaptureBaseline::stop() {
  if (running_.exchange(false)) {
    return;
  }
  running_ = false;
  if (thread_.joinable()) {
    thread_.join();
  }
  cap_.release();
}

void CameraCaptureBaseline::run() {
  bool is_opened = false;
  if (!device_path_.empty()) {
    is_opened = cap_.open(device_path_, cv::CAP_V4L2);
  } else {
    is_opened = cap_.open(device_id_, cv::CAP_V4L2);
  }

  if (!is_opened) {
    running_ = false;
    return;
  }

  uint64_t frame_id = 0;
  while (running_) {
    cv::Mat image;
    if (!cap_.read(image)) {
      continue;
    }

    FrameDesc frame;
    frame.frame_id = frame_id++;
    frame.capture_time = std::chrono::steady_clock::now();
    frame.image = image.clone();

    mailbox_.push(std::move(frame));
  }

  cap_.release();
}
