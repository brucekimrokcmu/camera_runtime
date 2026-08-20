#include "camera_capture.hpp"

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

CameraCapture::CameraCapture(FramePipelineManager& pipeline, const std::string& device_path)
    : pipeline_(pipeline), device_path_(device_path) {}

CameraCapture::CameraCapture(FramePipelineManager& pipeline, int device_id)
    : pipeline_(pipeline), device_id_(device_id) {}

CameraCapture::~CameraCapture() { stop(); }

bool CameraCapture::isOpened() const { return cap_.isOpened(); }

void CameraCapture::start() {
  if (running_) return;

  if (!device_path_.empty()) {
    cap_.open(device_path_, cv::CAP_V4L2);
  } else {
    cap_.open(device_id_, cv::CAP_V4L2);
  }

  if (!cap_.isOpened()) {
    std::string err_msg = "Error: Unable to open camera device at " +
                          (device_path_.empty() ? std::to_string(device_id_) : device_path_);
    std::cerr << err_msg << std::endl;
    throw std::runtime_error(err_msg);
  }

  cap_.set(cv::CAP_PROP_FRAME_WIDTH, pipeline_.getWidth());
  cap_.set(cv::CAP_PROP_FRAME_HEIGHT, pipeline_.getHeight());

  cv::Mat test_frame;
  if (!cap_.read(test_frame) || test_frame.empty()) {
    cap_.release();
    std::string err_msg = "Error: Camera opened, but failed to read initial frame from " +
                          (device_path_.empty() ? std::to_string(device_id_) : device_path_);
    std::cerr << err_msg << std::endl;
    throw std::runtime_error(err_msg);
  }

  running_ = true;
  thread_ = std::thread(&CameraCapture::run, this);
}

void CameraCapture::stop() {
  running_ = false;
  if (thread_.joinable()) {
    thread_.join();
  }
  if (cap_.isOpened()) {
    cap_.release();
  }
}

void CameraCapture::run() {
  uint64_t frame_counter = 0;

  double fps = cap_.get(cv::CAP_PROP_FPS);

  std::cout << "[CameraCatpure] FPS: " << fps << std::endl;
  std::cout << "[CameraCapture] Width: " << cap_.get(cv::CAP_PROP_FRAME_WIDTH) << std::endl;
  std::cout << "[CameraCapture] Height: " << cap_.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

  if (fps <= 0) {
    fps = 60.0;
  }

  auto frame_duration = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(1.0 / fps));

  while (running_) {
    auto start_time = std::chrono::steady_clock::now();
    auto frame = pipeline_.acquire_buffer();
    if (frame) {
      if (!cap_.read(frame->image) || frame->image.empty()) {
        std::cerr << "[CameraCapture] Frame capture drop/error." << std::endl;
        pipeline_.release_buffer(frame);  // Return buffer if capture fails
        continue;
      }

      frame->frame_id = ++frame_counter;
      frame->timestamp = start_time;
      // frame->image.setTo(cv::Scalar(frame_counter % 255, 100, 150));

      pipeline_.push_latest(frame);
    }

    std::this_thread::sleep_until(start_time + frame_duration);
  }
}
