// -*- coding: utf-8-unix -*-
#ifndef SAMPLER_H
#define SAMPLER_H

#include <memory>
#include <thread>

namespace csn {

class sampler {
 public:
  sampler() : _stop(false) {}
  virtual ~sampler() = default;
  void run();
  void stop();
  void join();

  std::function< void() > sample;

 private:
  std::thread _th;
  bool _stop;

  void sample1();
};

}
#endif // SAMPLER_H
