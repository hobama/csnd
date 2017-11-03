// -*- coding: utf-8-unix -*-
#include "onavi.hpp"
#include <iostream>
#include <fmt/format.h>

int main(int argc, char* argv[]) {
  std::string device_name("/dev/ttyACM0");
  csn::sensor_onavi onavi(device_name);
  csn::sensor& s = onavi;
  std::cout << fmt::format("PRECISION: {}", onavi.precision()) << std::endl;
  std::shared_ptr<csn::accel_record> ar = s.sample();
  std::cout << fmt::format("AR: {},{},{}", ar->x, ar->y, ar->z, ar->sample_count) << std::endl;
  std::exit(EXIT_SUCCESS);
}
