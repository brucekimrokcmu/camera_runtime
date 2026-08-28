#ifndef UTILS_CAMERA_UTILS_HPP
#define UTILS_CAMERA_UTILS_HPP

#include <string>
#include <vector>

/*
 * UVC Camera (046d:0825) (usb-0000:00:14.0-6):
        /dev/video4
        /dev/video5
        /dev/media2
 *
 * */

namespace utils {

struct CameraDeviceInfo {
  std::string device_path;
  std::string card_name;
  std::string bus_info;
  bool is_usb;
};

std::string DetectUsbCameraDevice(bool require_sub_bus = true);

std::vector<CameraDeviceInfo> GetAvailableCameraDevice();

}  // namespace utils

#endif  // UTILS_CAMERA_UTILS_HPP
