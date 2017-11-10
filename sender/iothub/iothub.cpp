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
  hub.send_bytes_fallback = [this](std::shared_ptr< std::vector< uint8_t > > data) {
    if (this->send_bytes_fallback) {
      log_trace("CALL send_bytes_fallback");
      return this->send_bytes_fallback(data);
    } else {
      log_trace("CAN'T CALL send_bytes_fallback");
      return std::async(std::launch::async, [](){ return true; });
    }
  };
  hub.send_string_fallback = [this](std::shared_ptr< std::string > str) {
    if (this->send_string_fallback) {
      log_trace("CALL send_string_fallback");
      return this->send_string_fallback(str);
    } else {
      log_trace("CAN'T CALL send_string_fallback");
      return std::async(std::launch::async, [](){ return true; });
    }
  };
}

csn::sender_iothub::~sender_iothub() {
}

void csn::sender_iothub::send(std::map< std::string, std::string > prop, std::shared_ptr< std::vector< uint8_t > > data) {
  return hub.send(prop, data);
}

void csn::sender_iothub::send(std::map< std::string, std::string > prop, std::shared_ptr< std::string > str) {
  return hub.send(prop, str);
}
