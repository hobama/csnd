// -*- coding: utf-8-unix -*-
#ifndef SENDER_H
#define SENDER_H

#include <map>
#include <memory>
#include <vector>
#include <future>

namespace csn {

class sender {
 public:
  sender() = default;
  sender(const sender&) = delete;
  sender& operator=(const sender&) = delete;
  sender(sender&&) = default;
  sender& operator=(sender&&) = default;

  virtual ~sender() {}
  virtual void send(std::map< std::string, std::string > prop, std::shared_ptr< std::vector< uint8_t > > data) = 0;
  virtual void send(std::map< std::string, std::string > prop, std::shared_ptr< std::string > str) = 0;

  std::function<std::future< bool >(std::shared_ptr< std::vector< uint8_t > > data) > send_bytes_fallback = nullptr;
  std::function<std::future< bool >(std::shared_ptr< std::string > str) > send_string_fallback = nullptr;
};

}
#endif // SENDER_H
