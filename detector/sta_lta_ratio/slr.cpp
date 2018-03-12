// -*- coding: utf-8-unix -*-
#include "slr.hpp"
#include "util.hpp"

#include <chrono>
#include <memory>
#include <string>

using namespace std::chrono;

namespace {

const int BUCKET_SIZE = 500;
const size_t LTA_BUCKET_SIZE = 500; // 10sec * 50hz
const size_t STA_BUCKET_SIZE = 150; // 3sec * 50hz
const float STA_LTA_RATIO = 3.0f;
const int TRIGGER_THRESHOLD = 3;

}

csn::detector_slr::detector_slr(std::string device_id) :
    _lt_buf(csn::circular_buffer< csn::accel_record >(LTA_BUCKET_SIZE)),
    _lt_acc(0), _trigger_count(0), _device_id(device_id),
    _pool(1) { // 本当は並列で処理したいが _trigger_count があるため直列で処理せざるを得ない
}

csn::detector_slr::~detector_slr() {
  _last_ftr.wait();
}

void csn::detector_slr::push(const std::shared_ptr< csn::accel_record > ar) {
  _last_ftr = _pool.enqueue([this, ar]() {
      this->_lt_acc += ar->z; // x, y 軸はノイズが多いため z 軸だけ使っているとのこと

      std::shared_ptr< csn::accel_record > olar = this->_lt_buf.push(ar);

      if (olar != nullptr) {
        this->_lt_acc -= olar->z;
      } else {
        return; // _lt_buf がいっぱいになるまで以降の処理は不要
      }

      float offset = this->_lt_acc / (float)LTA_BUCKET_SIZE; // 重力分をオフセットする
      float st_sum = 0;
      float lt_sum = 0;
      for (int i = LTA_BUCKET_SIZE - 1; i >= 0; i--) {
        lt_sum += fabs(this->_lt_buf[i]->z - offset);
        if (i == STA_BUCKET_SIZE) {
          st_sum = lt_sum; // 後ろから 3sec 分
        }
      }
      float sta = st_sum / STA_BUCKET_SIZE;
      float lta = lt_sum / LTA_BUCKET_SIZE;

      if (sta != 0.0f && lta != 0.0f) {
        //log_trace("STA/LTA RATIO {}, ({} / {})", sta/lta, sta, lta);
        if (sta > lta * STA_LTA_RATIO) {
          this->_trigger_count++;
          if (this->_trigger_count > TRIGGER_THRESHOLD) {
            this->_trigger_count = 0;
            // イベントJSON生成
            // TODO JSON ライブラリの利用
            std::string s("{\"type\":\"quake\",\"method\":\"sta-lta-ratio\",");
            s += "\"deviceId\":\"" + _device_id + "\",";
            s += "\"time\":" + std::to_string(duration_cast<milliseconds>(ar->start_time.time_since_epoch()).count()) + ",";
            s += "\"data\":{\"sta\":" + std::to_string(sta) + ",\"lta\":" + std::to_string(lta) + "}}";
            //log_debug("!!!QUAKE!!! {}", s);
            if (callback) {
              callback(s);
            }
          }
        } else {
          this->_trigger_count = 0; // 途切れたらクリア
        }
      } else {
        log_debug("STA {}, LTA {}", sta, lta);
      }
    });
}
