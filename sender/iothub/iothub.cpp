// -*- coding: utf-8-unix -*-
#include "iothub.hpp"
#include "util.hpp"

csn::sender_iothub::sender_iothub(const std::string connection_string) : hub(connection_string) {
  aziot::iothub::log = [](const aziot::loglevel level, const std::string& msg) {
    switch (level) {
      case aziot::loglevel::trace:
      log_trace(msg);
      break;
      case aziot::loglevel::info:
      log_info(msg);
      break;
      case aziot::loglevel::debug:
      log_debug(msg);
      break;
      case aziot::loglevel::error:
      log_error(msg);
    }
  };
}

csn::sender_iothub::~sender_iothub() {
}

void csn::sender_iothub::send(std::map< std::string, std::string > prop, std::shared_ptr< std::vector< uint8_t > > data, std::function< void() >&& fallback) {
  return hub.send(prop, data, std::move(fallback));
}

void csn::sender_iothub::send(std::map< std::string, std::string > prop, std::shared_ptr< std::string > str, std::function< void() >&& fallback) {
  return hub.send(prop, str, std::move(fallback));
}
