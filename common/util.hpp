// -*- coding: utf-8-unix -*-
#ifndef UTIL_H
#define UTIL_H

// ログ
#include <spdlog/spdlog.h>

template <typename Arg1, typename... Args> void log_trace(const char* fmt, const Arg1& arg, const Args&... args) {
  if (spdlog::get("default")) { spdlog::get("default")->trace(fmt, arg, args...); }
}
template <typename Arg1, typename... Args> void log_debug(const char* fmt, const Arg1& arg, const Args&... args) {
  if (spdlog::get("default")) { spdlog::get("default")->debug(fmt, arg, args...); }
}
template <typename Arg1, typename... Args> void log_info(const char* fmt, const Arg1& arg, const Args&... args) {
  if (spdlog::get("default")) { spdlog::get("default")->info(fmt, arg, args...); }
}
template <typename Arg1, typename... Args> void log_warn(const char* fmt, const Arg1& arg, const Args&... args) {
  if (spdlog::get("default")) { spdlog::get("default")->warn(fmt, arg, args...); }
}
template <typename Arg1, typename... Args> void log_error(const char* fmt, const Arg1& arg, const Args&... args) {
  if (spdlog::get("default")) { spdlog::get("default")->error(fmt, arg, args...); }
}
template <typename Arg1, typename... Args> void log_critical(const char* fmt, const Arg1& arg, const Args&... args) {
  if (spdlog::get("default")) { spdlog::get("default")->critical(fmt, arg, args...); }
}

template <typename T> void log_trace(const T& msg) {
  if (spdlog::get("default")) { spdlog::get("default")->trace(msg); }
}
template <typename T> void log_debug(const T& msg) {
  if (spdlog::get("default")) { spdlog::get("default")->debug(msg); }
}
template <typename T> void log_info(const T& msg) {
  if (spdlog::get("default")) { spdlog::get("default")->info(msg); }
}
template <typename T> void log_warn(const T& msg) {
  if (spdlog::get("default")) { spdlog::get("default")->warn(msg); }
}
template <typename T> void log_error(const T& msg) {
  if (spdlog::get("default")) { spdlog::get("default")->error(msg); }
}
template <typename T> void log_critical(const T& msg) {
  if (spdlog::get("default")) { spdlog::get("default")->critical(msg); }
}


// リソース開放
#include <functional>

class scoped_guard {
 public :
  explicit scoped_guard(std::function<void()> f) : f(f) {}
  scoped_guard(scoped_guard const &) = delete ;
  void operator = (scoped_guard const &) = delete ;
  ~scoped_guard() { f(); }
 private:
  std::function<void()> f;
};

// 文字列分割
#include <vector>
#include <string>
#include <sstream>

inline std::vector<std::string> split(const std::string &str, char sep) {
  std::vector<std::string> v;
  std::stringstream ss(str);
  std::string buf;
  while (std::getline(ss, buf, sep)) {
    v.push_back(buf);
  }
  return v;
}

// 現在時刻カウント
#include <chrono>
inline int64_t int64now() {
  return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

#endif // UTIL_H
