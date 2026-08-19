#include <chrono>
#include <iostream>

#include "preprocess_infer.hpp"

PreprocessInferConsumer::PreprocessInferConsumer(FramePipelineManager& pipeline)
    : pipeline_(pipeline) {}

PreprocessInferConsumer::~PreprocessInferConsumer() { stop(); }

void PreprocessInferConsumer::start() {
  running_ = true;
  thread_ = std::thread(&PreprocessInferConsumer::run, this);
}

void PreprocessInferConsumer::stop() {
  running_ = false;
  pipeline_.stop();
  if (thread_.joinable()) {
    thread_.join();
  }
}

void PreprocessInferConsumer::run() {
  using namespace std::chrono_literals;
  bool debug_image_saved = false;
  while (running_) {
    auto frame = pipeline_.pop_latest();
    if (!frame) {
      break;
    }

    std::cout << "[Consumer 20Hz] Processing Frame ID: " << frame->frame_id
              << " | Mat size: " << frame->image.cols << "x"
              << frame->image.rows << std::endl;
    if (!debug_image_saved && !frame->image.empty()) {
      std::string debug_path = "/tmp/consumed_frame.jpg";

      if (cv::imwrite(debug_path, frame->image)) {
        std::cout
            << "[PreprocessInferConsumer] Successfully saved debug image to "
            << debug_path << " (Frame ID: " << frame->frame_id << ")"
            << std::endl;
        debug_image_saved = true;  // Save once to avoid hammering disk I/O
      } else {
        std::cerr << "[PreprocessInferConsumer] Failed to save debug image to "
                  << debug_path << std::endl;
      }
    }
    std::this_thread::sleep_for(50ms);  // TODO: fix the hardcoding
    pipeline_.release_buffer(frame);
  }
}
