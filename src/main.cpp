// -*- coding: utf-8-unix -*-
#include "config.hpp"
#include "sampler.hpp"
#include "common/accel.hpp"
#include "recorder/recorder.hpp"

#include "detector/sta_lta_ratio/slr.hpp"
#include "sensor/onavi/onavi.hpp"
#include "sender/iothub/iothub.hpp"

#ifdef USE_AVROCPP
#include "serializer/avro_cpp/avrocpp.hpp"
#else
#include "serializer/avro_c/avroc.hpp"
#endif

#include <chrono>
#include <csignal>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include <unistd.h>
#include <sys/types.h>

#include <argagg/argagg.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

std::shared_ptr< csn::recorder > recorder;
std::shared_ptr< csn::sampler > sampler;

void signal_handler(int signal) {
  spdlog::get("default")->info("CAUGHT A SIGNAL ({})", signal);
  if (sampler) {
    sampler->stop();
  }
  if (recorder) {
    recorder->stop();
  }
}

int main(int argc, char const *argv[]) {
  std::shared_ptr<spdlog::logger> log;

  argagg::parser argparser {{
      { "help", {"-h", "--help"}, "shows this help message", 0},
      { "config", {"-c", "--config"}, "configuration file", 1},
      { "daemon", {"-d", "--daemon"}, "run on daemon mode", 0},
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

  std::string config_file;
  if (args["config"]) {
    config_file = args["config"].as<std::string>();
  } else {
    std::cerr << "Error: mandatory argument '-c'" << std::endl;
    return EXIT_FAILURE;
  }

  if (args["daemon"]) {
    std::cout << "Running on daemon mode" << std::endl;
    if (std::signal(SIGHUP, SIG_IGN) == SIG_ERR) {
      std::cerr << "Error: failed to let ignore SIGUP" << std::endl;
    }
    if (daemon(0, 0)) {
      std::cerr << "Error: failed to daemon" << std::endl;
      return EXIT_FAILURE;
    }
    if (std::signal(SIGHUP, SIG_DFL) == SIG_ERR) {
      std::cerr << "Error: failed to let handle SIGUP" << std::endl;
    }
  } else {
    std::cout << "Running on foreground" << std::endl;
  }

  try {
    // 設定ファイル読み込み
    std::unique_ptr< csn::config > cfg(new csn::config(config_file));

    // PIDファイル作成
    std::remove(cfg->pid_file().c_str());
    std::ofstream pid_file(cfg->pid_file());
    pid_file << getpid();
    pid_file.close();

    // ロガー生成
    switch (cfg->logging_logger()) {
      case csn::logger::CONSOLE_LOGGER:
        log = spdlog::stdout_logger_mt("default");
        break;
      case csn::logger::FILE_LOGGER:
        log = spdlog::rotating_logger_mt("default",
                                         cfg->logging_file_settings_filename(),
                                         cfg->logging_file_settings_max_file_size(),
                                         cfg->logging_file_settings_max_files());
        break;
      default:
        throw std::runtime_error("UNSUPPORTED LOGGER");
    }

    log->set_level(cfg->logging_level());
    log->info("CONFIG ({}): {}, {}, {}", cfg->device_id(), cfg->iothub_bucket_size(),
              cfg->sensor_device_file(), cfg->out_dir());

#ifdef USE_AVROCPP
    std::shared_ptr< csn::serializer > serializer(new csn::serializer_avrocpp(csn::avro_codec::deflate));
#else
    std::shared_ptr< csn::serializer > serializer(new csn::serializer_avroc(csn::avro_codec::deflate));
#endif

    std::shared_ptr< csn::sender > sender;
    if (cfg->offline_mode()) {
      log->info("OFFLINE MODE");
    } else {
      log->info("ONLINE MODE");
      sender.reset(new csn::sender_iothub(cfg->iothub_conn_string()));
    }

    std::shared_ptr< csn::sensor > sensor(new csn::sensor_onavi(cfg->sensor_device_file()));
    std::shared_ptr< csn::detector > detector(new csn::detector_slr(cfg->device_id()));
    recorder.reset(new csn::recorder(cfg->iothub_bucket_size(), cfg->out_dir()));
    sampler.reset(new csn::sampler());

    // コンポーネント間の連携はここで記述する。
    // 個々のコンポーネントは相手を知る必要はない。

    // センサーからサンプリングした際の振る舞い
    sampler->sample = [&]() {
      std::shared_ptr< csn::accel_record > ar = sensor->sample();
      recorder->push(ar);
      detector->push(ar);
    };

    // 記録する際の振る舞い
    recorder->flush = [&](std::shared_ptr< std::vector< std::shared_ptr< csn::accel_record > > > records) {
      std::shared_ptr< std::vector<uint8_t> > data = serializer->serialize(records);
      log->info("SERIALIZED {} RECORDS TO {} BYTES", records->size(), data->size());
      if (cfg->offline_mode()) {
        recorder->record(data, ".avro");
      } else {
        std::map<std::string, std::string> prop;
        prop.insert(std::make_pair("batch", "false"));
        sender->send(prop, data, [data, log]() {
            log->warn("CALLED FALLBACK FUNCTION TO WRITE {} BYTES", data->size());
            return recorder->record(data, "avro");
          });
      }
    };

    // 揺れを検知した際の振る舞い
    detector->callback = [&](const std::string& s) {
      log->info("QUAKE!");
      std::shared_ptr< std::string > str = std::make_shared< std::string >(s);
      if (cfg->offline_mode()) {
        recorder->record(str, "_quake.json");
      } else {
        std::map<std::string, std::string> prop;
        // 揺れイベントは IoT Hub 側で Event Hub にルーティングするためプロパティを付与
        prop.insert(std::make_pair("kind", "quake"));
        prop.insert(std::make_pair("batch", "false"));
        sender->send(prop, str, [str, log]() {
            log->warn("CALLED FALLBACK FUNCTION TO WRITE {} BYTES", str->size());
            return recorder->record(str, "_quake.json");
          });
      }
    };

    // サンプリング開始
    if (std::signal(SIGTERM, signal_handler) == SIG_ERR) {
      throw std::runtime_error(fmt::format("FAILED TO SET HANDLER FOR SIGTERM ({})", errno));
    }
    if (std::signal(SIGINT,  signal_handler) == SIG_ERR) {
      throw std::runtime_error(fmt::format("FAILED TO SET HANDLER FOR SIGTERM ({})", errno));
    }

    log->info("CSND START");
    sampler->run();

    sampler->join();
    recorder->join();

    log->info("CSND STOP");
    //spdlog::drop_all();
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    if (log) {
      log->error(ex.what());
      //spdlog::drop_all();
    } else {
      std::cout << "ERROR: " << ex.what() << std::endl;
    }
    return EXIT_FAILURE;
  }
}
