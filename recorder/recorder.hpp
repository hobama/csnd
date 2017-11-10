// -*- coding: utf-8-unix -*-
#ifndef RECORDER_H
#define RECORDER_H

#include "accel.hpp"

#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <ThreadPool.h>

namespace csn {

class recorder {
 public:
  recorder(const int bucket_size, const std::string& out_dir);
  virtual ~recorder();

  const int bucket_size;

  void push(std::shared_ptr<csn::accel_record> ar);
  void stop();
  void join();

  std::function< void(std::shared_ptr< std::vector< std::shared_ptr< csn::accel_record > > >) > flush;

  std::future< bool > record(std::shared_ptr< std::vector< uint8_t > > data);
  std::future< bool > record(std::shared_ptr< std::string > data);

 private:
  std::condition_variable _cond;
  std::mutex _mtx;
  std::thread _th;
  bool _stop = false;
  std::shared_ptr< std::vector< std::shared_ptr< csn::accel_record > > > _bucket;
  void run();
  ThreadPool _pool;
  std::string _out_dir;
  std::future< void > _last_ftr;
};

}
#endif // RECORDER_H
