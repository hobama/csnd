// -*- coding: utf-8-unix -*-
#ifndef SERIALIZER_AVROCPP_H
#define SERIALIZER_AVROCPP_H

#include "serializer.hpp"

namespace csn {

class serializer_avrocpp : public serializer {
 public:
  serializer_avrocpp(avro_codec c);
  ~serializer_avrocpp();

  std::shared_ptr< std::vector<uint8_t> > serialize(std::shared_ptr< std::vector< std::shared_ptr< csn::accel_record > > > ars);
 private:
  class impl;
  std::unique_ptr<impl> _impl;
};

}
#endif // SERIALIZER_AVROCPP_H
