#pragma once

#include <cstdint>

namespace coverbs_rpc {

enum class log_level : uint8_t {
  trace = 0,
  debug = 1,
  info = 2,
  warn = 3,
  err = 4,
  critical = 5,
  off = 6,
};

/**
 * @brief Set the log level for coverbs_rpc.
 *
 * @param level The log level to set.
 */
void set_log_level(log_level level);

} // namespace coverbs_rpc
