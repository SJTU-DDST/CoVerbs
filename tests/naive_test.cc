#include <coverbs_rpc/detail/logger.hpp>
#include <cppcoro/task.hpp>
#include <rdmapp/qp.h>

int main(int argc [[maybe_unused]], char **argv [[maybe_unused]]) {
  coverbs_rpc::detail::get_logger()->info("hello world!");
  return 0;
}
