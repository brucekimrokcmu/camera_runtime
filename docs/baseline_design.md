## Vanilla design 
### Baseline design without optimization

Producer                     Consumer
   │                            │
   │ push(frame)                │
   ▼                            │
┌─────────────────────┐         │
│     latest frame    │────────►│ pop()
└─────────────────────┘

CameraCapture
  cv::VideoCapture
  std::thread
  local cv::Mat
  frame.clone()

LatestFrameMailbox
  mutex
  condition_variable
  optional<FrameDesc>

PreprocessInferConsumer
  std::thread
  pop()
  OpenCV processing

FrameDesc
  frame_id
  capture_time
  cv::Mat


Limitations
