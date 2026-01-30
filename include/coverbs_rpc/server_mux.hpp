#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <span>

namespace coverbs_rpc {

class basic_mux {
public:
  using Handler =
      std::function<std::size_t(std::span<std::byte> payload, std::span<std::byte> resp)>;

  auto register_handler(uint32_t fn_id, Handler h) -> void;

  auto dispatch(uint32_t fn_id, std::span<std::byte> payload, std::span<std::byte> resp) const
      -> std::size_t;

private:
  std::map<uint32_t, Handler> handlers_;
};

} // namespace coverbs_rpc
