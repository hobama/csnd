/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef ___SRC_AR_HPP_3839782900__H_
#define ___SRC_AR_HPP_3839782900__H_


#include <sstream>
#include "boost/any.hpp"
#include "avro/Specific.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"

namespace avro_serialize {
struct accel_record {
    float x;
    float y;
    float z;
    int32_t sample_count;
    int64_t start_time;
    int64_t end_time;
    accel_record() :
        x(float()),
        y(float()),
        z(float()),
        sample_count(int32_t()),
        start_time(int64_t()),
        end_time(int64_t())
        { }
};

}
namespace avro {
template<> struct codec_traits<avro_serialize::accel_record> {
    static void encode(Encoder& e, const avro_serialize::accel_record& v) {
        avro::encode(e, v.x);
        avro::encode(e, v.y);
        avro::encode(e, v.z);
        avro::encode(e, v.sample_count);
        avro::encode(e, v.start_time);
        avro::encode(e, v.end_time);
    }
    static void decode(Decoder& d, avro_serialize::accel_record& v) {
        if (avro::ResolvingDecoder *rd =
            dynamic_cast<avro::ResolvingDecoder *>(&d)) {
            const std::vector<size_t> fo = rd->fieldOrder();
            for (std::vector<size_t>::const_iterator it = fo.begin();
                it != fo.end(); ++it) {
                switch (*it) {
                case 0:
                    avro::decode(d, v.x);
                    break;
                case 1:
                    avro::decode(d, v.y);
                    break;
                case 2:
                    avro::decode(d, v.z);
                    break;
                case 3:
                    avro::decode(d, v.sample_count);
                    break;
                case 4:
                    avro::decode(d, v.start_time);
                    break;
                case 5:
                    avro::decode(d, v.end_time);
                    break;
                default:
                    break;
                }
            }
        } else {
            avro::decode(d, v.x);
            avro::decode(d, v.y);
            avro::decode(d, v.z);
            avro::decode(d, v.sample_count);
            avro::decode(d, v.start_time);
            avro::decode(d, v.end_time);
        }
    }
};

}
#endif
