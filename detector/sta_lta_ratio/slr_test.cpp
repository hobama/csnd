// -*- coding: utf-8-unix -*-
#include "detector.hpp"
#include "sensor.hpp"
#include "sampler.hpp"

#include "slr.hpp"
#include "onavi.hpp"

#include <csignal>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <unistd.h>
#include <sys/types.h>

#include <argagg/argagg.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

std::shared_ptr<csn::sampler> sampler;

void signal_handler(int signal) {
  spdlog::get("default")->info("CAUGHT A SIGNAL ({})", signal);
  if (sampler) {
    sampler->stop();
  }
}

int main(int argc, char const *argv[]) {
  std::shared_ptr<spdlog::logger> logger;

  argagg::parser argparser {{
      { "help", {"-h", "--help"}, "shows this help message", 0},
      { "sensor", {"-s", "--sensor"}, "sensor device file", 1},
  }};

  argagg::parser_results args;
  try {
    args = argparser.parse(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  if (args["help"]) {
    argagg::fmt_ostream fmt(std::cerr);
    fmt << "Usage: csnd [options]" << std::endl
        << argparser;
    return EXIT_SUCCESS;
  }

  std::string sensor_file;
  if (args["sensor"]) {
    sensor_file = args["sensor"].as<std::string>();
  } else {
    std::cerr << "Error: mandatory argument '-s'" << std::endl;
    return EXIT_FAILURE;
  }

  try {
    // ロガー生成
    logger = spdlog::stdout_logger_mt("default");
    logger->set_level(spdlog::level::trace);
    logger->trace("SENSOR DEVICE: {}", sensor_file);

    std::shared_ptr<csn::sensor> sensor(new csn::sensor_onavi(sensor_file));

    std::shared_ptr<csn::detector> detector(new csn::detector_slr(std::string("bogus_device")));
    detector->callback = [&](const std::string& s) {
      logger->info("EARTHQUAKE!!! {}", s);
    };

    sampler.reset(new csn::sampler());

    sampler->sample = [&]() {
      std::shared_ptr<csn::accel_record> ar = sensor->sample();
      try {
        detector->push(ar);
      } catch (const std::exception& ex) {
        logger->error("DETECTOR ERROR: {}", ex.what());
      }
    };

    // サンプリング開始
    if (std::signal(SIGTERM, signal_handler) == SIG_ERR) {
      throw std::runtime_error(fmt::format("FAILED TO SET HANDLER FOR SIGTERM ({})", errno));
    }
    if (std::signal(SIGINT,  signal_handler) == SIG_ERR) {
      throw std::runtime_error(fmt::format("FAILED TO SET HANDLER FOR SIGTERM ({})", errno));
    }
    logger->info("CSND START");
    sampler->run();

    sampler->join();

    logger->info("CSND STOP");
    spdlog::drop_all();
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    if (logger) {
      logger->error(ex.what());
      spdlog::drop_all();
    } else {
      std::cout << "ERROR: " << ex.what() << std::endl;
    }
    return EXIT_FAILURE;
  }
}
