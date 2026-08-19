#include "thread_safe_queue.hpp"

FramePipelineManager::FramePipelineManager(size_t pool_size, int frame_height,
                                           int frame_width, int cv_type) {
  pool_storage_.reserve(pool_size);
  for (size_t i = 0; i < pool_size; ++i) {
    auto frame = std::make_shared<FrameDesc>();
    frame->image.create(frame_height, frame_width, cv_type);
    pool_storage_.push_back(frame);
    free_pool_.push(frame);
  }
}

std::shared_ptr<FrameDesc> FramePipelineManager::acquire_buffer() {
  std::lock_guard<std::mutex> lock(mtx_);
  if (free_pool_.empty()) {
    return nullptr;
  }
  auto frame = free_pool_.front();
  free_pool_.pop();
  return frame;
}

void FramePipelineManager::push_latest(std::shared_ptr<FrameDesc> new_frame) {
  std::shared_ptr<FrameDesc> frame_to_recycle = nullptr;
  {
    std::lock_guard<std::mutex> lock(mtx_);
    if (latest_slot_) {
      frame_to_recycle = latest_slot_;
    }
    latest_slot_ = new_frame;
    if (frame_to_recycle) {
      free_pool_.push(frame_to_recycle);
    }
  }
  cv_consumer_.notify_one();
}

void FramePipelineManager::return_buffer(std::shared_ptr<FrameDesc> frame) {
  if (!frame) {
    return;
  }
  std::lock_guard<std::mutex> lock(mtx_);
  free_pool_.push(frame);
}

std::shared_ptr<FrameDesc> FramePipelineManager::pop_latest() {
  std::unique_lock<std::mutex> lock(mtx_);
  cv_consumer_.wait(lock,
                    [this] { return latest_slot_ != nullptr || stop_flag_; });

  if (stop_flag_ && !latest_slot_) {
    return nullptr;
  }

  auto frame = latest_slot_;
  latest_slot_ = nullptr;
  return frame;
}

void FramePipelineManager::release_buffer(std::shared_ptr<FrameDesc> frame) {
  if (!frame) {
    return;
  }
  std::lock_guard<std::mutex> lock(mtx_);
  free_pool_.push(frame);
}

void FramePipelineManager::stop() {
  {
    std::lock_guard<std::mutex> lock(mtx_);
    stop_flag_ = true;
  }
  cv_consumer_.notify_all();
}

int FramePipelineManager::getWidth() const noexcept { return frame_width_; }

int FramePipelineManager::getHeight() const noexcept { return frame_height_; }
