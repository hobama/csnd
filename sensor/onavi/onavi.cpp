// -*- coding: utf-8-unix -*-
#include "onavi.hpp"
#include "util.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory.h>
#include <termios.h>
#include <fmt/format.h>

using namespace std::chrono;

namespace {

const int g_ciLen = 9;  // ##XXYYZZC
const float FLOAT_LINUX_ONAVI_FACTOR = 7.629394531250e-05f;
const float EARTH_G = 9.78033f;
const milliseconds SAMPLING_INTERVAL = milliseconds(2);
const milliseconds SAMPLING_DURATION = milliseconds(20);

}

csn::sensor_onavi::sensor_onavi(const std::string& device_file) : _device_file(device_file), _device_fd(-1) {
  // OSからはUSBシリアルチップが見えるのみなので、その先のセンサーが動くか確認する
  _device_fd = open(_device_file.c_str(), O_RDWR | O_NOCTTY);
  if (_device_fd == -1) {
    throw std::runtime_error(fmt::format("COULDN'T ACCESS THE DEVICE ({0}): {1}", _device_fd, _device_file));
  }

  // setup basic modem I/O
  struct termios options;
  memset(&options, 0x00, sizeof(options));
  options.c_cflag = B115200 | CRTSCTS | CS8 | CLOCAL | CREAD;
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;

  // read blocking conditions - force to read the whole g_ciLen otherwise we get bad truncation errors
  //  use VTIME so it doesn't block forever (ie 1 second timeout)
  options.c_cc[VTIME]    = 10;  // VTIME is in .1 secs, so this times out after a second (sensor should reset)
  options.c_cc[VMIN]     = g_ciLen;

  tcflush(_device_fd, TCIFLUSH);

  // 検出処理
  if (tcsetattr(_device_fd, TCSANOW, &options) == -1) {
    close_port();
    throw std::runtime_error(fmt::format("COULDN'T SET ATTRS: {0}", _device_file));
  }

  // try to read a value and get the sensor bit-type (& hence sensor type)
  float x,y,z;
  if (read_xyz(x, y, z) && _precision > 0) {
    switch(_precision) {
      case 12: case 16: case 24:
        break;
      default:
        close_port();
        throw std::runtime_error(fmt::format("UNKNOWN ONAVI SENSOR TYPE {0} BITS", _precision));
    }
  } else {
    close_port();
    throw std::runtime_error(fmt::format("ERROR IN READ_XYZ {0}, {1}, {2} - {3} BITS", x, y, z, _precision));
  }
}

void csn::sensor_onavi::close_port() {
  if (_device_fd > -1) {
    close(_device_fd);
  }
  _device_fd = -1;
  _precision = 0;
}

std::shared_ptr<csn::accel_record> csn::sensor_onavi::sample() {
  float x, y, z;
  float x_acc, y_acc, z_acc;
  int sample_count = 0;
  high_resolution_clock::time_point start_time = high_resolution_clock::now();
  high_resolution_clock::time_point limit = start_time + SAMPLING_DURATION - (SAMPLING_INTERVAL + milliseconds(1));
  high_resolution_clock::time_point end_time = start_time;
  x_acc = y_acc = z_acc = 0.0f;
  while (end_time < limit) {
    x = y = z = 0.0f;
    if (read_xyz(x, y, z)) {
      x_acc += x;
      y_acc += y;
      z_acc += z;
      sample_count++;
      std::this_thread::sleep_for(SAMPLING_INTERVAL);
      end_time = high_resolution_clock::now();
    } else {
      throw std::runtime_error("READ SENSOR ERROR");
    }
  }

  x_acc /= (float)sample_count;
  y_acc /= (float)sample_count;
  z_acc /= (float)sample_count;
  return std::make_shared<csn::accel_record>(x_acc, y_acc, z_acc, sample_count, start_time, end_time);
}

bool csn::sensor_onavi::read_xyz(float& x, float& y, float& z) {
  /*
    We tried to keep the data as simple as possible. The data looks like:

    ##xxyyzzs

    Two ASCII '#' (x23) followed by the X value upper byte, X value lower byte, Y value upper byte, Y value lower byte,
    Z value upper byte, Z value lower byte and an eight bit checksum.
    The bytes are tightly packed and there is nothing between the data values except for the sentence start ##.
    The X and Y axis will have a 0g offset at 32768d (x8000) and the Z axis offset at +1g 45874d (xB332)
    when oriented in the X/Y horizontal and Z up position.  The  s  value is a one byte checksum.
    It is a sum of all of the bytes, truncated to the lower byte.  This firmware does not transmit the temperature value.
    Finding g as a value:
    g  = x - 32768 * (5 / 65536)
    Where: x is the data value 0 - 65536 (x0000 to xFFFF).
    Values >32768 are positive g and <32768 are negative g.
    The sampling rate is set to 200Hz, and the analog low-pass filters are set to 50Hz.
    The data is oversampled 2X over Nyquist.
    We are going to make a new version of the module, with 25Hz LP analog filters and dual sensitivity 2g / 6g shortly.
    Same drivers, same interface. I'll get you one as soon as I we get feedback on this and make a set of those.
  */

  // first check for valid port
  if (_device_fd < 0) {
    return false;
  }
  bool result = false;

  unsigned char bytesIn[g_ciLen+1];  // note pad bytesIn with null \0
  unsigned char cs;
  int x_raw = 0, y_raw = 0, z_raw = 0;
  int iCS = 0;
  int iRead = 0;
  x = x_last; y = y_last; z = z_last;  // use last good values
  const char cWrite = '*';

  if ((iRead = write(_device_fd, &cWrite, 1)) == 1) {   // send a * to the device to get back the data
    memset(bytesIn, 0x00, g_ciLen+1);

    if ((iRead = read(_device_fd, bytesIn, g_ciLen)) == g_ciLen) { // read the g_ciLen
      // good data length read in, now test for appropriate characters ie start with * # $
      if (bytesIn[g_ciLen] == 0x00 &&
          ((bytesIn[0] == 0x2A && bytesIn[1] == 0x2A)
           || (bytesIn[0] == 0x23 && bytesIn[1] == 0x23)
           || (bytesIn[0] == 0x24 && bytesIn[1] == 0x24)) ) {
        // format is ##XXYYZZC\0
        // we found both, the bytes in between are what we want (really bytes after lOffset[0]
        if (_precision == 0) { // need to find sensor bit type i.e. 12/16/24-bit ONavi
          switch(bytesIn[0]) {
            case 0x2A:
              _precision = 12; break;
            case 0x23:
              _precision = 16; break;
            case 0x24:
              _precision = 24; break;
          }
        }
        x_raw = (bytesIn[2] * 255) + bytesIn[3];
        y_raw = (bytesIn[4] * 255) + bytesIn[5];
        z_raw = (bytesIn[6] * 255) + bytesIn[7];
        cs   = bytesIn[8];

        // convert to g decimal value
        // g  = x - 32768 * (5 / 65536)
        // Where: x is the data value 0 - 65536 (x0000 to xFFFF).
        x = ((float) x_raw - 32768.0f) * FLOAT_LINUX_ONAVI_FACTOR * EARTH_G;
        y = ((float) y_raw - 32768.0f) * FLOAT_LINUX_ONAVI_FACTOR * EARTH_G;
        z = ((float) z_raw - 32768.0f) * FLOAT_LINUX_ONAVI_FACTOR * EARTH_G;
        x_last = x; y_last = y; z_last = z;  // preserve values
        result = true;
      } else { //mismatched start bytes ie not ** or ## or $$
        log_warn("ERROR IN READ_XYZ - MISMATCHED START BYTES {0}, {1}", bytesIn[0], bytesIn[1]);
      }
    } else {
      log_warn("ERROR IN READ_XYZ - READ() RETURNED {0} -- ERRNO = {1} : {2}", iRead, errno, strerror(errno));
    }  // read bytesIn
  } else {
    log_warn("ERROR IN READ_XYZ - WRITE() RETURNED {0} -- ERRNO = {1} : {2}", iRead, errno, strerror(errno));
  }  // write *
  return result;
}
