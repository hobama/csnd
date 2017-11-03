// -*- coding: utf-8-unix -*-
#ifndef ACCEL_RECORD_H
#define ACCEL_RECORD_H

#include <chrono>

namespace csn {

class accel_record {
 public:
  accel_record(const float x, const float y, const float z, const int sample_count,
               std::chrono::high_resolution_clock::time_point start_time,
               std::chrono::high_resolution_clock::time_point end_time)
      : x(x), y(y), z(z), sample_count(sample_count), start_time(start_time), end_time(end_time) {};
  accel_record(const accel_record& o)
      : x(o.x), y(o.y), z(o.z), sample_count(o.sample_count), start_time(o.start_time), end_time(o.end_time) {};
  ~accel_record() {};
  const float x;
  const float y;
  const float z;
  const int sample_count;
  const std::chrono::high_resolution_clock::time_point start_time;
  const std::chrono::high_resolution_clock::time_point end_time;
};

}
#endif // ACCEL_RECORD_H
