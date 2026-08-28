#include "opencv_process_infer.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "latest_frame_mailbox.hpp"

OpenCVProcessInfer::OpenCVProcessInfer(LatestFrameMailbox& mailbox) : mailbox_(mailbox) {}

OpenCVProcessInfer::~OpenCVProcessInfer() { stop(); }

void OpenCVProcessInfer::start() {
  if (running_.exchange(true)) {
    return;
  }
  thread_ = std::thread(&OpenCVProcessInfer::run, this);
}

void OpenCVProcessInfer::stop() {
  if (running_.exchange(false)) {
    return;
  }
  running_ = false;
  if (thread_.joinable()) {
    thread_.join();
  }
}

void OpenCVProcessInfer::run() {
  while (running_) {
    auto frame = mailbox_.pop();

    if (!frame) {
      break;
    }
    process(*frame);
  }
}

void OpenCVProcessInfer::process(FrameDesc& frame) {
  // pipeline verification
  cv::Mat gray;
  cv::cvtColor(frame.image, gray, cv::COLOR_BGR2GRAY);
  static int saved_count = 0;

  if (saved_count < 5) {
    cv::imwrite("/tmp/frame_gray_" + std::to_string(saved_count) + ".png", gray);
    ++saved_count;
  }

  frame.image = std::move(gray);

  // 1. detect target color
  //  cv::Mat mask = detect_color(frame.image);
  // 2. find target
  // 3. estimate 3d pose
}
