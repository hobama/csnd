// -*- coding: utf-8-unix -*-
#ifndef SENDER_IOTHUB_H
#define SENDER_IOTHUB_H

#include "sender.hpp"
#include <azure/iothub.hpp>

namespace csn {

class sender_iothub : public sender {
 public:
  sender_iothub(const std::string connection_string);
  sender_iothub() = delete;
  ~sender_iothub();
  void send(std::map< std::string, std::string > prop, std::shared_ptr< std::vector< uint8_t > > data);
  void send(std::map< std::string, std::string > prop, std::shared_ptr< std::string > str);
 private:
  aziot::iothub hub;
};

}
#endif // SENDER_IOTHUB_H
