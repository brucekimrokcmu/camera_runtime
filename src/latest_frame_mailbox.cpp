#include "latest_frame_mailbox.hpp"

#include <mutex>
#include <optional>

#include "frame_desc.hpp"

LatestFrameMailbox::LatestFrameMailbox() {};

void LatestFrameMailbox::push(FrameDesc frame) {
  {
    std::lock_guard<std::mutex> lock(mtx_);
    latest_ = std::move(frame);
  }
  cv_.notify_one();
}

std::optional<FrameDesc> LatestFrameMailbox::pop() {
  std::unique_lock<std::mutex> lock(mtx_);
  cv_.wait(lock, [this] { return latest_.has_value() || stop_flag_; });
  if (stop_flag_ || !latest_) {
    return std::nullopt;
  }
  FrameDesc frame = *latest_;
  latest_.reset();
  return frame;
}

void LatestFrameMailbox::stop() {
  {
    std::lock_guard<std::mutex> lock(mtx_);
    stop_flag_ = true;
  }
  cv_.notify_all();
}
