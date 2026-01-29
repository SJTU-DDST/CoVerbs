#pragma once

#include <atomic>
#include <cppcoro/task.hpp>
#include <memory>
#include <span>
#include <vector>

#include <rdmapp/mr.h>
#include <rdmapp/qp.h>

#include "coverbs_rpc/common.hpp"

namespace coverbs_rpc {

class Client {
public:
  Client(std::shared_ptr<rdmapp::qp> qp, RpcConfig config = {});
  ~Client();

  auto call(std::span<const std::byte> req_data,
            std::span<std::byte> resp_buffer) -> cppcoro::task<std::size_t>;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

class ClientMux {
public:
  ClientMux(std::vector<std::shared_ptr<rdmapp::qp>> qps,
            RpcConfig config = {});
  ~ClientMux();

  auto call(std::span<const std::byte> req_data,
            std::span<std::byte> resp_buffer) -> cppcoro::task<std::size_t>;

private:
  std::vector<std::unique_ptr<Client>> clients_;
  std::atomic<unsigned int> selector_{0};
};

} // namespace coverbs_rpc
