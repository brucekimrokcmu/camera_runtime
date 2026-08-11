#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

/*
What are the problems with the vanilla-FrameDescriptor?
1) Constructor invokes std::vector.resize(). resize method initializes all elements to 0, and the overwrites with data
2) each struct pass-by-value, which causes a heap allocation for each frame.
3) toCvMat() returns cv::Mat that borrows a raw pointer via pixels.data(). This creates a dangerous lifetime dependency, which may cause dangling pointers.
*/
struct FrameDescriptor {
	uint64_t sequence = 0;
	int height = 0;
	int width = 0;
	int channels = 3;	
	std::vector<uint8_t> pixels;

	FrameDescriptor() = default;

	FrameDescriptor(uint64_t seq, int h, int w, int c = 3 ) 
	: sequence(seq), height(h), width(w), channels(c) {
	  if (h > 0 && w > 0 && c > 0) {
	  	pixels.resize(static_cast<size_t>(h) * static_cast<size_t>(w) * static_cast<size_t>(c));
	  }
	} 
    
	cv::Mat toCvMat() {
      int type = (channels == 1) ? CV_8UC1 : CV_8UC3;
      return cv::Mat(height, width, type, pixels.data());   
	}
};

