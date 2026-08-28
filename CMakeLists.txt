cmake_minimum_required(VERSION 3.14)
project(camera_pipeline CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED COMPONENTS core highgui imgproc videoio)
find_package(Threads REQUIRED)

include_directories(include)

add_executable(camera_pipeline
	src/main.cpp
  src/camera_capture_baseline.cpp
  src/latest_frame_mailbox.cpp
  src/opencv_process_infer.cpp
)

target_include_directories(camera_pipeline PRIVATE include)

target_link_libraries(camera_pipeline PRIVATE
	${OpenCV_LIBS}
	Threads::Threads
)
