// -*- coding: utf-8-unix -*-
#ifndef CONFIG_H
#define CONFIG_H

#include <spdlog/spdlog.h>

namespace csn {

enum class logger { CONSOLE_LOGGER, FILE_LOGGER };

class config {
public:
  config(const std::string& config_file);
  config() = delete;
  virtual ~config() = default;
  spdlog::level::level_enum logging_level() { return _logging_level; };
  csn::logger logging_logger() { return _logging_logger; }
  std::string logging_file_settings_filename() { return _logging_file_settings_filename; }
  size_t logging_file_settings_max_file_size() { return _logging_file_settings_max_file_size; }
  size_t logging_file_settings_max_files() { return _logging_file_settings_max_files; }
  std::string sensor_device_file() { return _sensor_device_file; }
  std::string iothub_conn_string() { return _iothub_conn_string; }
  int iothub_bucket_size() { return _iothub_bucket_size; }
  std::string pid_file() { return _pid_file; }
  std::string device_id() { return _device_id; }
  std::string out_dir() { return _out_dir; }
  bool offline_mode() { return _offline_mode; }

private:
  spdlog::level::level_enum _logging_level;
  csn::logger _logging_logger;
  std::string _logging_file_settings_filename;
  size_t _logging_file_settings_max_file_size;
  size_t _logging_file_settings_max_files;
  std::string _sensor_device_file;
  std::string _iothub_conn_string;
  int _iothub_bucket_size;
  std::string _pid_file;
  std::string _device_id;
  std::string _out_dir;
  bool _offline_mode;
};

}
#endif // CONFIG_H
