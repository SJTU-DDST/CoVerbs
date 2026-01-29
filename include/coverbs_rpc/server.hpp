#pragma once

#include "coverbs_rpc/common.hpp"

#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <rdmapp/mr.h>
#include <rdmapp/qp.h>
#include <span>
#include <vector>

namespace coverbs_rpc {

class Server {
public:
  using Handler =
      std::function<std::size_t(std::span<std::byte> payload, std::span<std::byte> resp)>;

  Server(std::shared_ptr<rdmapp::qp> qp, RpcConfig config = {}, std::uint32_t thread_count = 4);

  auto register_handler(uint32_t fn_id, Handler h) -> void;

  auto run() -> cppcoro::task<void>;

private:
  auto server_worker(std::size_t idx) -> cppcoro::task<void>;

  std::map<uint32_t, Handler> handlers_;
  std::mutex handlers_mutex_;
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
