// -*- coding: utf-8-unix -*-
#include "sampler.hpp"
#include "util.hpp"

#include <chrono>

using namespace std::chrono;

namespace {

microseconds DURATION(20000);

}

void csn::sampler::stop() {
  log_info("SAMPLER STOPS NOW");
  _stop = true;
}

void csn::sampler::sample1() {
  log_info("BEGIN SAMPLING");
  high_resolution_clock::time_point next_time = high_resolution_clock::now() + DURATION;
  while (!_stop) {
    if (sample) {
      sample();
    }
    std::this_thread::sleep_until(next_time);
    next_time += DURATION;
  }
  log_info("SAMPLER HAS BEEN STOPPED");
}

void csn::sampler::run() {
  if (!_th.joinable()) {
    _th = std::thread(&csn::sampler::sample1, this);
  } else {
    throw std::runtime_error("csn::sampler::run() THE THREAD IS BUSY");
  }
}

void csn::sampler::join() {
  if (!_th.joinable()) {
    throw std::runtime_error("THE THREAD ISN'T JOINABLE");
  }

  try {
    _th.join();
  } catch (...) {
    log_error("SENSOR ERROR OCCURRED");
    std::rethrow_exception(std::current_exception());
  }
}
