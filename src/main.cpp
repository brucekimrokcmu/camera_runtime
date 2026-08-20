#include <chrono>

#include "camera_capture.hpp"
#include "preprocess_infer.hpp"
#include "thread_safe_queue.hpp"

int main() {
  // define pool_size, frame height, width
  // initialize manager
  // thread for camera producer and consumer

  constexpr size_t BUFFER_POOL_SIZE = 4;
  constexpr int FRAME_WIDTH = 1920;
  constexpr int FRAME_HEIGHT = 1080;

  FramePipelineManager pipeline(BUFFER_POOL_SIZE, FRAME_HEIGHT, FRAME_WIDTH, CV_8UC3);

  CameraCapture camera(pipeline);
  PreprocessInferConsumer consumer(pipeline);

  std::cout << "starting pipeline..." << std::endl;
  consumer.start();
  camera.start();

  // std::this_thread::sleep_for(std::chrono::seconds(3));
  std::cout << "Press Enter to stop..." << std::endl;
  std::cin.get();

  std::cout << "Shutting down the pipeline..." << std::endl;
  camera.stop();
  consumer.stop();

  std::cout << "Pipeline finished successfully." << std::endl;

  return 0;
}
