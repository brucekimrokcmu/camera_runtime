#include "camera_capture_baseline.hpp"

#include <chrono>
#include <cstdint>

#include "latest_frame_mailbox.hpp"

CameraCaptureBaseline::CameraCaptureBaseline(LatestFrameMailbox& mailbox) : mailbox_(mailbox) {}

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
}
