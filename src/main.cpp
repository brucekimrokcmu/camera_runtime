#include <iostream>

#include "camera_capture_baseline.hpp"
#include "latest_frame_mailbox.hpp"
#include "opencv_process_infer.hpp"
#include "utils/profiler.hpp"

// util func to auto detect usb webcam - use ioctl

int main() {
  std::string DEVICE_PATH = "/dev/video4";

  LatestFrameMailbox latest_mailbox;
  CameraCaptureBaseline camera_capture(latest_mailbox, DEVICE_PATH);
  OpenCVProcessInfer opencv_process(latest_mailbox);

  std::cout << "starting camera capture pipeline..." << std::endl;
  opencv_process.start();
  camera_capture.start();

  std::cout << "Press Enter to stop..." << std::endl;
  std::cin.get();

  std::cout << "Stopping the camera capture pipeline" << std::endl;
  camera_capture.stop();
  opencv_process.stop();

  utils::Profiler::instance().dump_latency_csv("outputs/baseline_profile.csv");

  std::cout << "Camera capture pipeline finished successfully." << std::endl;

  return 0;
}
