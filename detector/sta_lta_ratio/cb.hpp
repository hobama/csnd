// -*- coding: utf-8-unix -*-
#ifndef CIRCULAR_BUFFER_H
#define CURCULAR_BUFFER_H

#include <memory>

#include <fmt/format.h>

namespace csn {

template <typename T>
class circular_buffer {
 public:
  circular_buffer(const size_t capacity) : _buf(std::vector<std::shared_ptr<T>>()), _idx_zero(0), _capacity(capacity) {}
  std::shared_ptr<T> push(const std::shared_ptr<T>& val) {
    if (_buf.size() < _capacity) {
      _buf.push_back(val);
      return nullptr;
    } else {
      std::shared_ptr<T> old = _buf.at(_idx_zero);
      _buf[_idx_zero++] = val;
      _idx_zero = _idx_zero == _capacity ? 0 : _idx_zero;
      return old;
    }
  }
  std::shared_ptr<T> operator[](size_t idx) {
    if (idx < _capacity) {
      return _buf.at((_idx_zero + idx) % _capacity);
    } else {
      throw std::out_of_range(fmt::format("OUT OF RANGE ({} >= {})", idx, _capacity));
    }
  }
  size_t size() {
    return _buf.size();
  }
 private:
  size_t _idx_zero;
  size_t _capacity;
  std::vector<std::shared_ptr<T>> _buf;
};

}
#endif // CIRCULAR_BUFFER_H
