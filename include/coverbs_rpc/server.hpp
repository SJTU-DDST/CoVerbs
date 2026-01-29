#pragma once

#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include <rdmapp/mr.h>
#include <rdmapp/qp.h>

#include "coverbs_rpc/common.hpp"

namespace coverbs_rpc {

class Server {
public:
  using Handler = auto (*)(std::span<std::byte> payload,
                           std::span<std::byte> resp) -> std::size_t;

  Server(std::shared_ptr<rdmapp::qp> qp, Handler h, RpcConfig config = {},
         std::uint32_t thread_count = 4);

  auto run() -> cppcoro::task<void>;

private:
  auto server_worker(std::size_t idx) -> cppcoro::task<void>;

  Handler h_;
  RpcConfig const config_;
  std::size_t const send_buffer_size_;
  std::size_t const recv_buffer_size_;
  std::shared_ptr<rdmapp::qp> qp_;
  cppcoro::static_thread_pool tp_;

  std::vector<std::byte> recv_buffer_pool_;
  rdmapp::local_mr recv_mr_;
  std::vector<std::byte> send_buffer_pool_;
  rdmapp::local_mr send_mr_;
};

} // namespace coverbs_rpc
