// -*- coding: utf-8-unix -*-
#include "avroc.hpp"
#include "serializer_test.hpp"

int main(int argc, char* argv[]) {
  test_main(argc, argv, [](csn::avro_codec c) { return new csn::serializer_avroc(c); });
}
