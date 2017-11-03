// -*- coding: utf-8-unix -*-
#ifndef DETECTOR_H
#define DETECTOR_H

#include "accel.hpp"

#include <memory>
#include <vector>

namespace csn {

class detector {
 public:
  detector() {}
  detector(const detector&) = delete;
  detector& operator=(const detector&) = delete;
  detector(detector&&) = default;
  detector& operator=(detector&&) = default;

  virtual ~detector() {}

  virtual void push(std::shared_ptr<accel_record> ar) = 0;
  std::function<void(const std::string&)> callback;
};

}

#endif // DETECTOR_H
