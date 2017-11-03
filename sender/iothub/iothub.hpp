// -*- coding: utf-8-unix -*-
#ifndef SENDER_IOTHUB_H
#define SENDER_IOTHUB_H

#include "sender.hpp"

namespace csn {

class sender_iothub : public sender {
 public:
  sender_iothub(const std::string connection_string);
  sender_iothub() = delete;
  ~sender_iothub();
  void send_bytes(std::shared_ptr< std::vector< uint8_t > > data);
  void send_string(std::shared_ptr< std::string > str);
 private:
  class impl;
  std::unique_ptr<impl> _impl;
};

}
#endif // SENDER_IOTHUB_H
