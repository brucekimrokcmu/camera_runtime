#include <memory>
#include <mutext>
#include <opencv2/opencv.hpp>

class FrameBufferPool {
 public:
  struct Frame {
    cv::Mat mat;
    uint64_t timestamp_ns{0};
    uint64_t frame_id{0};
    bool is_valid{false};
  };

  explicit FrameBufferPool(size_t pool_size, int height, int width,
                           int channels)
      : {}

 private:
  size_t m_pool_size;
};
