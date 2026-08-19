#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#include "frame_desc.hpp"

class FramePipelineManager {
 public:
  FramePipelineManager(size_t pool_size, int frame_height, int frame_width,
                       int cv_type);

  // Camera Producer Interface
  std::shared_ptr<FrameDesc> acquire_buffer();
  void push_latest(std::shared_ptr<FrameDesc> new_frame);
  void return_buffer(std::shared_ptr<FrameDesc> frame);
  int getWidth() const noexcept;
  int getHeight() const noexcept;

  // OpenCV Consumer Interface
  std::shared_ptr<FrameDesc> pop_latest();
  void release_buffer(std::shared_ptr<FrameDesc> frame);
  void stop();

 private:
  std::mutex mtx_;
  std::condition_variable cv_consumer_;

  std::vector<std::shared_ptr<FrameDesc>> pool_storage_;
  std::queue<std::shared_ptr<FrameDesc>> free_pool_;
  std::shared_ptr<FrameDesc> latest_slot_ = nullptr;
  bool stop_flag_ = false;
  int frame_width_{1920};
  int frame_height_{1080};
};

#endif  // THREAD_SAFE_QUEUE_HPP
