// -*- coding: utf-8-unix -*-
#include "config.hpp"
#include "util.hpp"

#include <stdexcept>

#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

namespace {

const std::string TRACE("trace");
const std::string DEBUG("debug");
const std::string INFO("info");
const std::string WARNING("warning");
const std::string ERROR("error");
const std::string CRITICAL("critical");
const std::string OFF("off");

const std::string LOGGER_CONSOLE("console");
const std::string LOGGER_FILE("file");

}

csn::config::config(const std::string& config_file) :
    _pid_file("csnd.pid"),
    _out_dir("out"),
    _logging_level(spdlog::level::trace),
    _logging_logger(csn::logger::CONSOLE_LOGGER),
    _logging_file_settings_filename("csnd.log"),
    _logging_file_settings_max_file_size(131072),
    _logging_file_settings_max_files(8),
    _iothub_conn_string("YOUR_CONNECTION_STRING"),
    _device_id("SET_DEVICE_NAME"),
    _iothub_bucket_size(18000),
    _sensor_device_file("/dev/ttyACM0"),
    _offline_mode(true) {

  YAML::Node config = YAML::LoadFile(config_file);

  // mode
  if (config["offline_mode"]) {
    _offline_mode = config["offline_mode"].as<bool>();
  } else {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'offline_mode'");
  }

  // PID file
  if (config["pid_file"]) {
    _pid_file = config["pid_file"].as<std::string>();
  } else {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'pid_file'");
  }

  // output directory
  if (config["out_dir"]) {
    _out_dir = config["out_dir"].as<std::string>();
  } else {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'out_dir'");
  }

  // logging
  if (config["logging"] == nullptr) {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'logging'");
  }

  // logging.level
  if (config["logging"]["level"]) {
    std::string lv = config["logging"]["level"].as<std::string>();
    if (lv.compare(TRACE) == 0) {
      _logging_level = spdlog::level::trace;
    } else if (lv.compare(DEBUG) == 0) {
      _logging_level = spdlog::level::debug;
    } else if (lv.compare(INFO) == 0) {
      _logging_level = spdlog::level::info;
    } else if (lv.compare(WARNING) == 0) {
      _logging_level = spdlog::level::warn;
    } else if (lv.compare(ERROR) == 0) {
      _logging_level = spdlog::level::err;
    } else if (lv.compare(CRITICAL) == 0) {
      _logging_level = spdlog::level::critical;
    } else if (lv.compare(OFF) == 0) {
      _logging_level = spdlog::level::off;
    } else {
      throw std::invalid_argument(fmt::format("INVALID LOGGING LEVEL {}", lv.c_str()));
    }
  } else {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'logging.level'");
  }

  // logging.logger
  if (config["logging"]["logger"]) {
    std::string lgr = config["logging"]["logger"].as<std::string>();
    if (lgr.compare(LOGGER_CONSOLE) == 0) {
      _logging_logger = csn::logger::CONSOLE_LOGGER;
    } else if (lgr.compare(LOGGER_FILE) == 0) {
      _logging_logger = csn::logger::FILE_LOGGER;
    } else {
      throw std::invalid_argument(fmt::format("INVALID LOGGER {}", lgr.c_str()));
    }
  } else {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'logging.logger'");
  }

  // logging.file_settings
  if (config["logging"]["file_settings"] == nullptr) {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'logging.file_settings'");
  }

  // logging.file_settings.filename
  if (config["logging"]["file_settings"]["filename"]) {
    _logging_file_settings_filename = config["logging"]["file_settings"]["filename"].as<std::string>();
  } else {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'logging.file_settings.filename'");
  }

  // logging.file_settings.max_file_size
  if (config["logging"]["file_settings"]["max_file_size"]) {
    _logging_file_settings_max_file_size = config["logging"]["file_settings"]["max_file_size"].as<int>();
  } else {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'logging.file_settings.max_file_size'");
  }

  // logging.file_settings.max_files
  if (config["logging"]["file_settings"]["max_files"]) {
    _logging_file_settings_max_files = config["logging"]["file_settings"]["max_files"].as<int>();
  } else {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'logging.file_settings.max_files'");
  }

  // sensor
  if (config["sensor"] == nullptr) {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'sensor'");
  }

  // sensor.device_file
  if (config["sensor"]["device_file"]) {
    _sensor_device_file = config["sensor"]["device_file"].as<std::string>();
  } else {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'sensor.device_file'");
  }

  // iothub
  if (config["iothub"] == nullptr) {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'iothub'");
  }

  // iothub.connection_string
  if (config["iothub"]["connection_string"]) {
    _iothub_conn_string = config["iothub"]["connection_string"].as<std::string>();
    // deviceId をパースする
    std::vector<std::string> token = split(_iothub_conn_string, ';');
    if (token.size() > 2) {
      std::vector<std::string> kv = split(token.at(1), '=');
      if (kv.size() > 2) {
        _device_id = kv.at(1);
      }
    }
  } else {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'iothub.connection_string'");
  }

  // iothub.bucket_size
  if (config["iothub"]["bucket_size"]) {
    _iothub_bucket_size = config["iothub"]["bucket_size"].as<int>();
  } else {
    throw std::invalid_argument("UNDEFINED CONFIG ERROR: 'iothub.bucket_size'");
  }
}
