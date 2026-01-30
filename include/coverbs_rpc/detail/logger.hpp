#pragma once
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace coverbs_rpc ::detail {

inline std::shared_ptr<spdlog::logger> get_logger() {
  static auto logger = []() {
    auto l = spdlog::stdout_color_mt("coverbs_rpc");
    return l;
  }();
  return logger;
}

} // namespace coverbs_rpc::detail
