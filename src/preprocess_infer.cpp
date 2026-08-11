#include <chrono>
#include <iostream>

#include "preprocess_infer.hpp"

PreprocessInferConsumer::PreprocesInferConsumer(FramePipelineManager& pipeline)
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

  while (running_) {
    auto frame = pipeline_.pop_latest();
    if (!frame) {
      break;
    }

    std::cout << "[Consumer 20Hz] Processing Frame ID: " << frame->frame_id
              << " | Mat size: " << frame->image.cols << "x"
              << frame->image.rows << std::endl;
    std::this_thread::sleep_for(50ms);  // TODO: fix the hardcoding
    pipeline_.release_buffer(frame);
  }
}
