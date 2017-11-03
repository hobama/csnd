// -*- coding: utf-8-unix -*-
#ifndef DETECTOR_SLR_H
#define DETECTOR_SLR_H

#include "detector.hpp"
#include "cb.hpp"

#include <future>

#include <ThreadPool.h>

namespace csn {

class detector_slr : public detector {
 public:
  detector_slr(std::string device_id);
  ~detector_slr();

  void push(std::shared_ptr<csn::accel_record> ar);

 private:
  const std::string _device_id;
  csn::circular_buffer<csn::accel_record> _lt_buf;
  float _lt_acc;
  int _trigger_count;
  ThreadPool _pool;
  std::future< void > _last_ftr;
};

}
#endif // DETECTOR_SLR_H
