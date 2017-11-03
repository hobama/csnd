// -*- coding: utf-8-unix -*-
#ifndef SENSOR_ONAVI_H
#define SENSOR_ONAVI_H

#include "sensor.hpp"

namespace csn {

class sensor_onavi : public sensor {
 public:
  sensor_onavi(const std::string& device_file);
  ~sensor_onavi() { close_port(); }
  int precision() { return _precision; }
  std::shared_ptr<csn::accel_record> sample();
 private:
  std::string _device_file;
  int _device_fd;
  int _precision = 0;
  float x_last = 0.0f; // keep the last values
  float y_last = 0.0f; // keep the last values
  float z_last = 0.0f; // keep the last values
  void close_port();
  bool read_xyz(float& x, float& y, float& z);
};

}

#endif // SENSOR_ONAVI_H
