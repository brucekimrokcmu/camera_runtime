#include "opencv_process_infer.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "latest_frame_mailbox.hpp"
#include "utils/profiler.hpp"

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
    std::optional<FrameDesc> frame;
    {
      utils::ScopedTimer timer("mailbox_pop_wait", 0);
      frame = mailbox_.pop();
    }

    if (!frame) {
      break;
    }
    process(*frame);
  }
}

void OpenCVProcessInfer::process(FrameDesc& frame) {
  uint64_t fid = frame.frame_id;
  // 1. Time from camera capture to start of process()
  auto proc_start = utils::Profiler::now();
  utils::Profiler::instance().record_metric(
      {"mailbox_queue_delay", fid, frame.capture_time, proc_start});

  // main processing
  {
    utils::ScopedTimer timer("consumer_e2e_process", fid);
    cv::Mat gray;
    cv::cvtColor(frame.image, gray, cv::COLOR_BGR2GRAY);
    frame.image = std::move(gray);
  }
  // static int saved_count = 0;
  // if (saved_count < 100) {
  //   std::cout << "iamge write #:" << saved_count << std::endl;
  //   cv::imwrite("/tmp/frame_gray_" + std::to_string(saved_count) + ".png", gray);
  //   ++saved_count;
  // }
  //

  auto proc_end = utils::Profiler::now();
  utils::Profiler::instance().record_metric(
      {"total_e2e_pipeline_latency", fid, frame.capture_time, proc_end});
}
