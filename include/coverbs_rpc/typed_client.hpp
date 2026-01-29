#pragma once

#include "coverbs_rpc/basic_client.hpp"
#include "coverbs_rpc/detail/traits.hpp"

#include <glaze/glaze.hpp>
#include <memory>
#include <vector>

namespace coverbs_rpc {

class typed_client {
public:
  typed_client(std::shared_ptr<rdmapp::qp> qp, RpcConfig config = {})
      : client_(qp, config)
      , config_(config) {
    send_buffer_.resize(config.max_req_payload);
    recv_buffer_.resize(config.max_resp_payload);
  }

  template <auto Handler>
  auto call(auto &&req) -> cppcoro::task<detail::rpc_resp_t<Handler>> {
    using Resp = detail::rpc_resp_t<Handler>;
    using Req = detail::rpc_req_t<Handler>;
    static_assert(std::same_as<Req, std::decay_t<decltype(req)>>);
    constexpr uint32_t fn_id = detail::function_id<Handler>;

    auto ec = glz::write_beve(req, send_buffer_);
    if (ec) [[unlikely]] {
      throw std::runtime_error("failed to serialize request");
    }
    std::size_t req_size = ec.count;

    std::size_t resp_size =
        co_await client_.call(fn_id, std::span{send_buffer_.data(), req_size}, recv_buffer_);

    Resp resp{};
    auto err = glz::read_beve(resp, std::span{recv_buffer_.data(), resp_size});
    if (err) [[unlikely]] {
      throw std::runtime_error("failed to deserialize response");
    }

    co_return resp;
  }

private:
  basic_client client_;
  RpcConfig config_;
  std::vector<std::byte> send_buffer_;
  std::vector<std::byte> recv_buffer_;
};

} // namespace coverbs_rpc
