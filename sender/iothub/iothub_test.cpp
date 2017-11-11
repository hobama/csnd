// -*- coding: utf-8-unix -*-
#include "iothub.hpp"
#include <iostream>
#include <memory>
#include <fmt/format.h>
#include <argagg/argagg.hpp>

int main(int argc, char* argv[]) {
  argagg::parser argparser {{
      { "conn_str", {"-c", "--connection-string"}, "connection string for IoT Hub", 1},
  }};

  argagg::parser_results args;
  try {
    args = argparser.parse(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  std::string connection_string;
  if (args["conn_str"]) {
    connection_string = args["conn_str"].as<std::string>();
  } else {
    std::cerr << "Error: mandatory argument '-c'" << std::endl;
    return EXIT_FAILURE;
  }

  csn::sender_iothub sender(connection_string);

  std::shared_ptr<std::string> data_str = std::make_shared<std::string>("{\"name\": \"test\", \"age\": 20}");
  std::shared_ptr<std::vector<uint8_t>> data(new std::vector<uint8_t>(data_str->begin(), data_str->end()));

  std::map<std::string, std::string> prop;

  try {
    sender.send(prop, data, [data]() { std::cerr << "FAILED TO SEND DATA: " << std::to_string(data->size()) << std::endl; });
    prop.insert(std::make_pair("kind", "earthquake"));
    sender.send(prop, data_str, [data_str]() { std::cerr << "FAILED TO SEND STR: " << *data_str << std::endl; });
    return EXIT_SUCCESS;
  } catch(...) {
    try {
      std::rethrow_exception(std::current_exception());
    } catch (std::exception& ex) {
      std::cerr << "ERROR: " << ex.what() << std::endl;
    }
    return EXIT_FAILURE;
  }
}
