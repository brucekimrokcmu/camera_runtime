#include "camera_capture.hpp"
#inlcude < opencv2 / opencv.hpp>
#include <chrono>

CameraCapture::CameraCapture(FramePipelineManager& pipeline)
    : pipeline_(pipeline) {
  // TODO: what should the constructor do?
}

CameraCapture::~CameraCapture() { stop(); }

void CameraCapture::start() {
  running_ = true;
  thread_ =
      std::thread(&CameraCapture::run, this);  // TODO: understand this pointer
}

void CameraCapture::stop() {
  running_ = false;
  if (thread_.joinable()) {
    thread_.join();
  }
}

void CameraCapture::run() {
  unit64_t frame_counter = 0;
  using namespace std::chrono_literals;

  while (running_) {
    auto start_time = std::chrono::steady_clock::now();
    auto frame = pipline_.acquire();
    if (frame) {
      frame->frame_id = ++frame_counter;
      frame->timestamp = start_time;
      frame->image.setTo(cv::Scalar(frame_counter % 255, 100, 150));

      pipeline_.push_latest(frame);
    }

    std::this_thread::sleep_until(
        start_time + 16666us);  // TODO: 60 fps assumed. Fix the hard coding.
  }
}
