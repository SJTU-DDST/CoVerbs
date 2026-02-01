#include "coverbs_rpc/log.hpp"
#include "coverbs_rpc/detail/logger.hpp"

namespace coverbs_rpc {

void set_log_level(log_level level) { detail::get_logger()->set_level(level); }

} // namespace coverbs_rpc
