// -*- coding: utf-8-unix -*-
#ifndef SERIALIZER_TEST_H
#define SERIALIZER_TEST_H

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <string>

#include <spdlog/spdlog.h>
#include <argagg/argagg.hpp>

int test_main(int argc, char* argv[], std::function<csn::serializer*(csn::avro_codec)> gen_serializer) {
  std::shared_ptr<spdlog::logger> log = spdlog::stdout_logger_mt("default");
  log->set_level(spdlog::level::debug);

  argagg::parser argparser {{
      { "help", {"-h", "--help"}, "shows this help message", 0},
      { "codec", {"-c", "--codec"}, "codec", 1},
      { "out", {"-o", "--out"}, "output file", 1},
      { "num", {"-n", "--num"}, "number", 1},
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
    fmt << "Usage: program [options]" << std::endl
        << argparser;
    return EXIT_SUCCESS;
  }

  csn::avro_codec c = csn::avro_codec::deflate;
  int nrecords = 300;
  std::string outfile("ars.avro");

  if (args["codec"]) {
    std::string codec = args["codec"];
    if (codec.compare("null") == 0) {
      c = csn::avro_codec::null;
    } else if (codec.compare("deflate") == 0) {
      c = csn::avro_codec::deflate;
    } else {
      log->error("UNSUPPORTED CODEC: {}", codec);
      return EXIT_FAILURE;
    }
  }

  try {
    if (args["num"]) {
      nrecords = args["num"].as<int>();
    }
  } catch (const std::exception& e) {
    log->error("ERROR! {}", e.what());
    return EXIT_FAILURE;
  }

  if (args["out"]) {
    outfile = args["out"].as<std::string>();
  }

  log->debug("NUM: {}, CODEC: {} => OUTPUT FILE: {}",
             nrecords,
             (c == csn::avro_codec::null ? "null" : "deflate"),
             outfile);

  std::random_device seed_gen;
  std::mt19937 mt(seed_gen());
  std::uniform_real_distribution<float> acc(-10.0, 10.0);

  auto ars = std::make_shared< std::vector< std::shared_ptr< csn::accel_record > > >();
  for (int i = 0; i < nrecords; i++) {
    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
    float x_acc = acc(mt);
    float y_acc = acc(mt);
    float z_acc = acc(mt);
    int sample_count = 5;
    std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
    std::shared_ptr<csn::accel_record> ar(new csn::accel_record(x_acc, y_acc, z_acc, sample_count, start_time, end_time));
    ars->push_back(ar);
  }

  csn::serializer* s = gen_serializer(c);
  std::shared_ptr< std::vector<uint8_t> > data = s->serialize(ars);
  delete s;

  std::ofstream ofs;
  ofs.open(outfile, std::ios::out);
  for (int i = 0; i < data->size(); i++) {
    ofs << data->at(i);
  }
  ofs.close();

  return EXIT_SUCCESS;
}
#endif // SERIALIZER_TEST_H
