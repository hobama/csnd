// -*- coding: utf-8-unix -*-
#include "recorder.hpp"
#include "util.hpp"

#include <chrono>
#include <fstream>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <spdlog/spdlog.h>

using namespace std::chrono;

namespace {

bool prepare(const char* path) {
  struct stat sb;
  if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
    return true;
  } else {
    if (mkdir(path, S_IRUSR | S_IRGRP | S_IXUSR | S_IXGRP | S_IWUSR | S_IWGRP) == 0) {
      return true;
    } else {
      switch (errno) {
        case EACCES:
          log_error("Access denied (EACCES): {}", path);
          break;
        case EDQUOT:
          log_error("Ran out of disk quota (EDQUOT): {}", path);
          break;
        case EEXIST:
          log_error("File exist (EEXIST): {}", path);
          break;
        case EFAULT:
          log_error("Bad address (EFAULT): {}", path);
          break;
        case ELOOP:
          log_error("Cannot translate name (ELOOP): {}", path);
          break;
        case EMLINK:
          log_error("Too many links (EMLINK): {}", path);
          break;
        case ENAMETOOLONG:
          log_error("File name too long (ENAMETOOLONG) {}", path);
          break;
        case ENOENT:
          log_error("No such file or directory (ENOENT) {}", path);
          break;
        case ENOMEM:
          log_error("Cannot allocate memory (ENOMEM) {}", path);
          break;
        case ENOSPC:
          log_error("No space left on device (ENOSPC) {}", path);
          break;
        case ENOTDIR:
          log_error("Not a directory (ENOTDIR) {}", path);
          break;
        case EPERM:
          log_error("Operation not permitted (EPERM) {}", path);
          break;
        case EROFS:
          log_error("Read-only file system (EROFS) {}", path);
          break;
        default:
          log_error("UNKNOWN MKDIR ERROR ({}) {}", errno, path);
      }
      return false;
    }
  }
}

}

std::future< bool > csn::recorder::record_bytes(std::shared_ptr< std::vector< uint8_t > > data) {
  return _pool.enqueue([this, data]() {
      if (prepare(this->_out_dir.c_str())) {
        try {
          int64_t now = int64now();
          std::string filename(_out_dir);
          filename += "/" + std::to_string(now) + ".avro";
          std::ofstream out_file;
          out_file.open(filename, std::ios::out | std::ios::binary);
          out_file.write((char*)&(*data)[0], data->size());
          out_file.close();
          return true;
        } catch (const std::exception& ex) {
          log_error("FAILED TO WRITE AVRO FILE: {}", ex.what());
          return false;
        }
      } else {
        return false;
      }
    });
}

std::future< bool > csn::recorder::record_string(std::shared_ptr< std::string > str) {
  return _pool.enqueue([this, str]() {
      if (prepare(this->_out_dir.c_str())) {
        try {
          int64_t now = int64now();
          std::string filename(_out_dir);
          filename += "/" + std::to_string(now) + ".json";
          std::ofstream out_file;
          out_file.open(filename, std::ios::out);
          out_file << *str;
          out_file.close();
          return true;
        } catch (const std::exception& ex) {
          log_error("FAILED TO WRITE JSON FILE: {}", ex.what());
          return false;
        }
      } else {
        return false;
      }
    });
}
