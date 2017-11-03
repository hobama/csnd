// -*- coding: utf-8-unix -*-
#include "cb.hpp"

#include <iostream>
#include <string>

#include <fmt/format.h>

void test_circular() {
  csn::circular_buffer<std::string> foo(5);

  for (int i = 0; i < 10; i++) {
    std::shared_ptr<std::string> p = std::make_shared<std::string>(fmt::format("TEST VALUE {}", i));
    std::shared_ptr<std::string> old_value = foo.push(p);
    if (old_value == nullptr) {
      std::cout << fmt::format(" NOT SPILLED ({})", i) << std::endl;
    } else {
      std::cout << fmt::format(" SPILLED VALUE ({}) -> \"{}\"", i, *old_value) << std::endl;
    }
  }
}

struct X {
  X(const int x) : _x(x) {}
  ~X() { std::cout << fmt::format("  DESTRUCTED ({})", _x) << std::endl; }
 private:
  int _x;
};

void test_destructor() {
  csn::circular_buffer<X> foo(5);
  csn::circular_buffer<X> bar(10);

  for (int i = 0; i < 15; i++) {
    std::cout << fmt::format(" PUSH ({})", i) << std::endl;
    std::shared_ptr<X> p = std::make_shared<X>(i);
    foo.push(p);
    bar.push(p);
  }
  std::cout << " FINISH" << std::endl;

}

void test_access() {
  csn::circular_buffer<std::string> foo(5);
  for (int i = 0; i < 15; i++) {
    std::shared_ptr<std::string> p = std::make_shared<std::string>(fmt::format("TEST VALUE {}", i));
    foo.push(p);
  }
  for (int i = 0; i < 5; i++) {
    std::cout << fmt::format("foo[{}] = {}", i, *foo[i]) << std::endl;
  }
  try {
    foo[5];
  } catch (const std::exception& ex) {
    std::cout << fmt::format("ERROR: {}", ex.what()) << std::endl;
  }
}

int main(const int argc, const char* argv[]) {
  std::cout << "test_circular:" << std::endl;
  test_circular();
  std::cout << "test_destructor:" << std::endl;
  test_destructor();
  std::cout << "test_access:" << std::endl;
  test_access();
}
