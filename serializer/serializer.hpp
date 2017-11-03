// -*- coding: utf-8-unix -*-
#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "accel.hpp"

#include <memory>
#include <vector>

namespace csn {

enum class avro_codec { null, deflate };

class serializer {
 public:
  serializer() = delete;
  serializer(avro_codec c) {}
  serializer(const serializer&) = delete;
  serializer& operator=(const serializer&) = delete;
  serializer(serializer&&) = default;
  serializer& operator=(serializer&&) = default;

  virtual ~serializer() {}

  virtual std::shared_ptr< std::vector<uint8_t> > serialize(std::shared_ptr< std::vector< std::shared_ptr< csn::accel_record > > > ars) = 0;
};

}
#endif // SERIALIZER_H
