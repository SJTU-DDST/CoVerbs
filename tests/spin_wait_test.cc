#include "coverbs_rpc/detail/logger.hpp"
#include "coverbs_rpc/utils/spin_wait.hpp"

#include <cassert>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>
#include <string>

using namespace coverbs_rpc::utils;
using coverbs_rpc::detail::get_logger;

cppcoro::task<void> test_void() { co_return; }

cppcoro::task<int> test_int(int val) { co_return val; }

cppcoro::task<std::string> test_string(std::string s) { co_return s; }

cppcoro::task<int &> test_ref(int &val) { co_return val; }

cppcoro::task<void> test_exception() {
  throw std::runtime_error("test exception");
  co_return;
}

int main() {
  get_logger()->info("Starting spin_wait tests...");

  // Test void
  spin_wait(test_void());
  get_logger()->info("test_void passed");

  // Test int
  int val = spin_wait(test_int(42));
  assert(val == 42);
  get_logger()->info("test_int passed: {}", val);

  // Test string
  std::string s = spin_wait(test_string("hello"));
  assert(s == "hello");
  get_logger()->info("test_string passed: {}", s);

  // Test reference
  int x = 100;
  int &ref = spin_wait(test_ref(x));
  assert(&ref == &x);
  ref = 200;
  assert(x == 200);
  get_logger()->info("test_ref passed");

  // Test exception
  try {
    spin_wait(test_exception());
    assert(false && "Should have thrown an exception");
  } catch (const std::runtime_error &e) {
    assert(std::string(e.what()) == "test exception");
    get_logger()->info("test_exception passed: {}", e.what());
  }

  get_logger()->info("All spin_wait tests passed!");

  return 0;
}
