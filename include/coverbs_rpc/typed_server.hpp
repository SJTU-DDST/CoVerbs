#pragma once

#include "coverbs_rpc/basic_server.hpp"
#include "coverbs_rpc/detail/traits.hpp"

#include <exception>
#include <glaze/glaze.hpp>
#include <memory>

namespace coverbs_rpc {

class typed_server {
public:
  typed_server(std::shared_ptr<rdmapp::qp> qp, RpcConfig config = {},
               std::uint32_t thread_count = 4)
      : server_(qp, config, thread_count) {}

  template <auto Handler>
  auto register_handler() -> void {
    static_assert(!detail::is_member_fn_v<Handler>,
                  "for calling with no instance, Handler must not be a member function");
    register_handler_impl<Handler>(Handler);
  }

  template <auto Handler, typename Class>
  auto register_handler(Class *instance) -> void {
    static_assert(detail::is_member_fn_v<Handler>,
                  "for calling with instance, Handler must be a member function");
    auto invoker = [instance](auto &&...args) {
      return std::invoke(Handler, instance, std::forward<decltype(args)>(args)...);
    };
    register_handler_impl<Handler>(invoker);
  }

  auto run() -> cppcoro::task<void> { co_await server_.run(); }

private:
  template <auto Handler, typename Invoker>
  auto register_handler_impl(Invoker invoker) -> void {
    using Req = detail::rpc_req_t<Handler>;
    using Resp = detail::rpc_resp_t<Handler>;
    constexpr uint32_t fn_id = detail::function_id<Handler>;

    auto h = [inv = std::move(invoker)](std::span<std::byte> req_bytes,
                                        std::span<std::byte> resp_bytes) -> std::size_t {
      Req req{};
      auto err = glz::read_beve(req, req_bytes);
      if (err) [[unlikely]] {
        std::terminate();
        return 0;
      }

      Resp resp;
      if constexpr (detail::is_coro_fn_v<Handler>) {
        // resp = co_await inv(req);
      } else {
        resp = inv(req);
      }

      auto ec = glz::write_beve(resp, resp_bytes);
      if (ec) [[unlikely]] {
        std::terminate();
        return 0;
      }
      return ec.count;
    };

    server_.register_handler(fn_id, std::move(h));
  }

  basic_server server_;
};

} // namespace coverbs_rpc
