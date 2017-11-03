// -*- coding: utf-8-unix -*-
#include "avroc.hpp"
#include "util.hpp"

#include <chrono>
#include <fstream>
#include <stdexcept>
#include <iostream>

using namespace std::chrono;

#include <fmt/format.h>

#include <stdio.h>
#include <time.h>

#include <avro.h>
#include <codec.h>     // add 'src' directory to include path
#include <encoding.h>  // add 'src' directory to include path

namespace {
const char MAGIC[] = {'O', 'b', 'j', '\x01'};
const char AR_SCHEMA[] =
    "{\"name\": \"accel_record\",\
      \"type\": \"record\",\
      \"fields\": [\
       {\"name\": \"x\", \"type\": \"float\"},\
       {\"name\": \"y\", \"type\": \"float\"},\
       {\"name\": \"z\", \"type\": \"float\"},\
       {\"name\": \"sample_count\", \"type\": \"int\"},\
       {\"name\": \"start_time\", \"type\": {\"type\": \"long\", \"logicalType\": \"timestamp-millis\"}},\
       {\"name\": \"end_time\",   \"type\": {\"type\": \"long\", \"logicalType\": \"timestamp-millis\"}}]}";
}

namespace csn {

class serializer_avroc::impl {
 public:
  impl(csn::avro_codec c);
  ~impl();
  std::shared_ptr< std::vector<uint8_t> > serialize(std::shared_ptr< std::vector< std::shared_ptr< csn::accel_record > > > ars);
 private:
  std::random_device _seed_gen;
  std::mt19937 _mt;
  avro_schema_t _ar_schema;
  avro_codec_t _codec;
  const avro_encoding_t *_enc;
  char* _header_buffer;
  size_t _header_buffer_size;
  int64_t _header_size;
};

}

csn::serializer_avroc::impl::impl(csn::avro_codec c) :
    _seed_gen(), _mt(_seed_gen()), _enc(&avro_binary_encoding), _header_buffer_size(0), _header_size(0), _header_buffer(NULL) {

  if (avro_schema_from_json_literal(AR_SCHEMA, &_ar_schema)) {
    throw std::runtime_error(fmt::format("FAILED TO PARSE AR SCHEMA ({})", avro_strerror()));
  }

  // create codec
  _codec = (avro_codec_t) avro_new(struct avro_codec_t_);
  if (!_codec) {
    throw std::runtime_error("FAILED TO ALLOCATE NEW CODEC");
  }
  switch (c) {
    case csn::avro_codec::null:
      if (::avro_codec(_codec, "null")) {
        throw std::runtime_error("FAILED TO SET CODEC (null)");
      }
      break;
    case csn::avro_codec::deflate:
      if (::avro_codec(_codec, "deflate")) {
        throw std::runtime_error("FAILED TO SET CODEC (deflate)");
      }
      break;
    default:
      throw std::runtime_error("UNSUPPORTED CODEC");
  }

  // create header data
  avro_writer_t header_writer = NULL;
  avro_writer_t schema_writer = NULL;
  char schema_buf[64 * 1024];

  scoped_guard guard([&] {
      if (header_writer) {
        avro_writer_reset(header_writer);
        avro_writer_free(header_writer);
      }
      if (schema_writer) {
        avro_writer_reset(schema_writer);
        avro_writer_free(schema_writer);
      }
      log_trace("AVRO_C WORKING DATA HAVE BEEN CLEARED");
    });

  schema_writer = avro_writer_memory(schema_buf, sizeof(schema_buf));
  if (!schema_writer) {
    throw std::runtime_error("FAILED TO CREATE SCHEMA WRITER");
  }
  if (avro_schema_to_json(_ar_schema, schema_writer)) {
    throw std::runtime_error("FAILED TO TRANSLATE SCHEMA TO JSON");
  }
  int64_t schema_len = avro_writer_tell(schema_writer);

  // create header buffer
  _header_buffer_size = 4 + 8 + 10 + strlen(_codec->name) + 11 + schema_len + 8;
  _header_buffer = (char*) avro_malloc(_header_buffer_size);
  if (!_header_buffer) {
    throw std::runtime_error("FAILED TO ALLOCATE HEADER BUFFER");
  }

  header_writer = avro_writer_memory(_header_buffer, _header_buffer_size);
  if (!header_writer) {
    throw std::runtime_error("FAILED TO CREATE HEADER WRITER");
  }

  // write header
  if (avro_write(header_writer, (void*)MAGIC, 4)) {
    throw std::runtime_error("FAILED TO WRITE MAGIC");
  }
  if (_enc->write_long(header_writer, 2)) {
    throw std::runtime_error("FAILED TO WRITE LONG (2)");
  }
  if (_enc->write_string(header_writer, "avro.codec")) {
    throw std::runtime_error("FAILED TO WRITE KEY 'avro.codec'");
  }
  if (_enc->write_bytes(header_writer, _codec->name, strlen(_codec->name))) {
    throw std::runtime_error("FAILED TO WRITE VALUE OF 'avro.codec'");
  }
  if (_enc->write_string(header_writer, "avro.schema")) {
    throw std::runtime_error("FAILED TO WRITE KEY 'avro.schema'");
  }
  if (_enc->write_bytes(header_writer, schema_buf, schema_len)) {
    throw std::runtime_error(fmt::format("FAILED TO WRITE VALUE OF 'avro.schema' ({})", schema_len));
  }
  if (_enc->write_long(header_writer, 0)) {
    throw std::runtime_error("FAILED TO WRITE LONG (0)");
  }
  _header_size = avro_writer_tell(header_writer);
}

csn::serializer_avroc::impl::~impl() {
  avro_schema_decref(_ar_schema);
  if (_header_buffer) { avro_free(_header_buffer, _header_buffer_size); }
  if (_codec) {
    avro_codec_reset(_codec);
    avro_freet(struct avro_codec_t_, _codec);
  }
}

std::shared_ptr< std::vector<uint8_t> > csn::serializer_avroc::impl::serialize(std::shared_ptr< std::vector< std::shared_ptr< csn::accel_record > > > ars) {
  if (ars == nullptr || ars->size() == 0) {
    throw std::invalid_argument("NO DATA TO SERIALIZE");
  }

  avro_writer_t datum_writer;
  char* datum_buffer; // ワーキングバッファ
  size_t datum_buffer_size = ars->size() * (sizeof(float) * 3 + sizeof(int32_t) + sizeof(int64_t) * 2);

  scoped_guard guard_datum_writer([&] {
      if (datum_writer) {
        avro_writer_reset(datum_writer);
        avro_writer_free(datum_writer);
      }
      if (datum_buffer) { avro_free(datum_buffer, datum_buffer_size); }
      log_trace("AVRO_C WORKING DATA HAVE BEEN CLEARED");
    });

  datum_buffer = (char*) avro_malloc(datum_buffer_size);
  if (!datum_buffer) {
    throw std::runtime_error("FAILED TO ALLOCATE DATUM BUFFER");
  }

  datum_writer = avro_writer_memory(datum_buffer, datum_buffer_size);
  if (!datum_writer) {
    throw std::runtime_error("FAILED TO CREATE DATUM WRITER");
  }

  // generate sync
  char sync[16];
  for (int i = 0; i < 16; i++) {
    sync[i] = _mt();
  }

  // create data block to working buffer
  int block_count = 0;
  for (const std::shared_ptr<csn::accel_record> a : *ars) {
    avro_datum_t ar = avro_record(_ar_schema);
    avro_datum_t x = avro_float(a->x);
    avro_datum_t y = avro_float(a->y);
    avro_datum_t z = avro_float(a->z);
    avro_datum_t sample_count = avro_int32(a->sample_count);
    avro_datum_t start_time = avro_int64(duration_cast<milliseconds>(a->start_time.time_since_epoch()).count());
    avro_datum_t end_time = avro_int64(duration_cast<milliseconds>(a->end_time.time_since_epoch()).count());

    scoped_guard guard_ar_datum([&] {
        avro_datum_decref(x);
        avro_datum_decref(y);
        avro_datum_decref(z);
        avro_datum_decref(sample_count);
        avro_datum_decref(start_time);
        avro_datum_decref(end_time);
        avro_datum_decref(ar);
        log_trace("AVRO_DATUM_DECREF COMPLETED");
      });

    if (avro_record_set(ar, "x", x) ||
        avro_record_set(ar, "y", y) ||
        avro_record_set(ar, "z", z) ||
        avro_record_set(ar, "sample_count", sample_count) ||
        avro_record_set(ar, "start_time", start_time) ||
        avro_record_set(ar, "end_time", end_time)) {
      throw std::runtime_error("UNABLE TO CREATE AR DATUM");
    }

    if (avro_write_data(datum_writer, _ar_schema, ar)) {
      throw std::runtime_error("UNABLE TO APPEND AR DATUM");
    }
    block_count++;
  }

  if (block_count <= 0) {
    throw std::runtime_error(fmt::format("NO DATA TO SERIALIZE ({})", block_count));
  }

  // encode data block
  size_t block_size = avro_writer_tell(datum_writer);
  if (avro_codec_encode(_codec, datum_buffer, block_size)) {
    throw std::runtime_error("FAILED TO ENCODE THE BLOCK");
  }
  log_debug("BLOCK COUNT: {}, BLOCK SIZE: {} -> {}", block_count, block_size, _codec->used_size);

  avro_writer_t writer;
  char* buffer;
  size_t buffer_size = _header_size + 4 + 4 + sizeof(sync) * 2 + _codec->used_size;

  scoped_guard guard_writer([&] {
      if (writer) {
        avro_writer_reset(writer);
        avro_writer_free(writer);
      }
      if (buffer) { avro_free(buffer, buffer_size); }
      log_trace("AVRO_C WRITER AND BUFFER HAVE BEEN CLEARED");
    });

  // allocate buffer
  buffer = (char*) avro_malloc(buffer_size);
  if (!buffer) {
    throw std::runtime_error("FAILED TO ALLOCATE BUFFER");
  }
  writer = avro_writer_memory(buffer, buffer_size);
  if (!writer) {
    throw std::runtime_error("FAILED TO CREATE WRITER");
  }

  // write header
  if (avro_write(writer, _header_buffer, _header_size)) {
    throw std::runtime_error(fmt::format("FAILED TO WRITE HEADER BLOCK ({})", _header_size));
  }
  if (avro_write(writer, sync, sizeof(sync))) {
    throw std::runtime_error("FAILED TO WRITE SYNC (HEADER)");
  }
  // write data block from working buffer to buffer
  if (_enc->write_long(writer, block_count)) {
    throw std::runtime_error("FAILED TO WRITE THE DATA BLOCK COUNT");
  }
  if (_enc->write_long(writer, _codec->used_size)) {
    throw std::runtime_error("FAILED TO WRITE THE DATA BLOCK SIZE");
  }
  if (avro_write(writer, _codec->block_data, _codec->used_size)) {
    throw std::runtime_error("FAILED TO WRITE THE DATA BLOCK");
  }
  if (avro_write(writer, sync, sizeof(sync))) {
    throw std::runtime_error("FAILED TO WRITE SYNC MARKER (DATA BLOCK)");
  }

  int64_t written = avro_writer_tell(writer);
  std::shared_ptr< std::vector<uint8_t> >result(new std::vector<uint8_t>(buffer, buffer + written));
  return result;
}

csn::serializer_avroc::serializer_avroc(csn::avro_codec c) :
    csn::serializer(c),
    _impl(new csn::serializer_avroc::impl(c)) {
}

csn::serializer_avroc::~serializer_avroc() {
}

std::shared_ptr< std::vector<uint8_t> > csn::serializer_avroc::serialize(std::shared_ptr< std::vector< std::shared_ptr< csn::accel_record > > > ars) {
  return _impl->serialize(ars);
}
