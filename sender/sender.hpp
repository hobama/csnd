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
  virtual void send(std::map< std::string, std::string > prop, std::shared_ptr< std::vector< uint8_t > > data, std::function< void() >&& fallback) = 0;
  virtual void send(std::map< std::string, std::string > prop, std::shared_ptr< std::string > str, std::function< void() >&& fallback) = 0;
};

}
#endif // SENDER_H
