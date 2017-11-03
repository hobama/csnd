// -*- coding: utf-8-unix -*-
#include "recorder.hpp"
#include "util.hpp"

csn::recorder::recorder(const int bucket_size, const std::string& out_dir) :
    bucket_size(bucket_size), _out_dir(out_dir), _pool(1),
    _bucket(new std::vector< std::shared_ptr< csn::accel_record > >()) {
  _bucket->reserve(bucket_size);
  _th = std::thread(&csn::recorder::run, this);
}

csn::recorder::~recorder() {
}

void csn::recorder::join() {
  if (_th.joinable()) {
    _th.join();
  }
  _last_ftr.wait();
}

void csn::recorder::stop() {
  log_info("RECORDER STOPS NOW");
  _stop = true;
  _cond.notify_one();
}

void csn::recorder::push(std::shared_ptr< csn::accel_record > ar) {
  if (_stop) {
    log_trace("PUSH TO STOPPED RECORDER");
    return;
  }
  {
    std::lock_guard< std::mutex > lock(_mtx);
    _bucket->push_back(ar);
  }
  if (_stop || _bucket->size() >= bucket_size) {
    _cond.notify_one();
  }
}

void csn::recorder::run() {
  while (!_stop) {
    std::shared_ptr< std::vector< std::shared_ptr< csn::accel_record > > > b;
    {
      std::unique_lock<std::mutex> lock(_mtx);
      _cond.wait(lock, [this] { return this->_stop || this->_bucket->size() >= this->bucket_size; });
      // _bucket を置き換えてすぐにアンロック
      b = _bucket;
      _bucket.reset(new std::vector< std::shared_ptr< csn::accel_record > >());
      _bucket->reserve(bucket_size);
    }

    // シリアライズと送信
    if (b->size() > 0) {
      _last_ftr = _pool.enqueue([this, b]() {
          if (this->flush) {
            log_trace("FLUSH {}", b->size());
            this->flush(b);
          }
        });
    }
  }
}
