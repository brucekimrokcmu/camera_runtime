#ifndef LATEST_FRAME_MAILBOX
#define LATEST_FRAME_MAILBOX

#include <condition_variable>
#include <mutex>
#include <optional>

#include "frame_desc.hpp"

class LatestFrameMailbox {
 public:
  LatestFrameMailbox();

  void push(FrameDesc frame);
  std::optional<FrameDesc> pop();
  void stop();

 private:
  std::mutex mtx_;
  std::condition_variable cv_;
  std::optional<FrameDesc> latest_;
  bool stop_flag_{false};
};

#endif  // LATEST_FRAME_MAILBOX
