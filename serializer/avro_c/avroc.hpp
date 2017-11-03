// -*- coding: utf-8-unix -*-
#ifndef SERIALIZER_AVROC_H
#define SERIALIZER_AVROC_H

#include "serializer.hpp"

namespace csn {

class serializer_avroc : public serializer {
 public:
  serializer_avroc(avro_codec c);
  ~serializer_avroc();

  std::shared_ptr< std::vector<uint8_t> > serialize(std::shared_ptr< std::vector< std::shared_ptr< csn::accel_record > > > ars);
 private:
  class impl;
  std::unique_ptr<impl> _impl;
};

}
#endif // SERIALIZER_AVROC_H
