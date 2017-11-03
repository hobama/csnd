// -*- coding: utf-8-unix -*-
#include "avrocpp.hpp"
#include "serializer_test.hpp"

int main(int argc, char* argv[]) {
  test_main(argc, argv, [](csn::avro_codec c) { return new csn::serializer_avrocpp(c); });
}
