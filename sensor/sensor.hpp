// -*- coding: utf-8-unix -*-
#ifndef SENSOR_H
#define SENSOR_H

#include "accel.hpp"

#include <memory>

namespace csn {

class sensor {
 public:
  sensor() {}
  sensor(const sensor&) = delete;
  sensor& operator=(const sensor&) = delete;
  sensor(sensor&&) = default;
  sensor& operator=(sensor&&) = default;

  virtual ~sensor() {}

  virtual std::shared_ptr<csn::accel_record> sample() = 0;
};

}

#endif // SENSOR_H
