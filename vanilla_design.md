## Vanilla design 

Camera --> preprocessing 

     Camera thread             <------ Buffer Pool
          |                                     ^
          | push to queue                       |
          |                                     |
    latest_only_queue                           |   return processed / dropped
          |                                     |
          | pop queue                           |
          |                                     |
    opencv consumer thread                      |
          |                                     |
          ---------------------------------------

* is this thread-safe when producer and consumer has different freq?
say producer has 60fps, and consumer has 20hz frequency.
Thus, make the buffer thread-safe.

What should be the size of buffer pool?
num_min_buffers = producer (1) + queue (1) + consumer(1)
+1 safety margin 


                      Buffer Pool
                                    ^
                |                   |
                V           
        Camera Thread           Dropped Frames
                |                   ^
                |                   |
                V push latest       |
                    Queue (capacity = 1 cv::Mat)
                |                   |
                | pop latest        |
                v                   |
        OpenCV Consumer thread ------ release after processing 







- CMakeLists.txt
- include/
    - frame_desc.hpp
    - thread_safe_queue.hpp
    - camera_capture.hpp
    - preprocess_infer.hpp
- src/

struct Frame {
  uint64_t sequence = 0;
  int height;
  int width;
  std::vector<uint8_t> pixels;  <-- convert this to cv::Mat

  // default constructor
  // constructor that gets seq, h, w as arg and reserve pixles by size_t h * w

};

--> pixels are using std::vector, which under the hood uses std::allocator that uses operator::new(). 





CameraCapture
class that opens a device and runs a loop on a dedicated thread.




BoundedQueue design 
This is a very naive approach because when producer frequency is higher than that of consumer, consumer will end up consuming outdated data.
an easy way to avoid this to set the queue size = 1 and overwrites it constantly. However, this assumes that the consumer demands only 1 frame
For vanilla runtime, I will use a bounded queue and then optimize it later.


Design preprocessor
How am I going to design this?








