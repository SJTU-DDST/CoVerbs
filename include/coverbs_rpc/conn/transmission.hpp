#pragma once

#include <cppcoro/net/socket.hpp>
#include <cppcoro/task.hpp>
#include <rdmapp/qp.h>

namespace coverbs_rpc {

struct qp_handshake {
  uint32_t nr_qp;
  uint64_t sid;
};

auto send_handshake(qp_handshake const &handshake, cppcoro::net::socket &socket)
    -> cppcoro::task<void>;

auto recv_handshake(cppcoro::net::socket &socket) -> cppcoro::task<qp_handshake>;

auto send_qp(rdmapp::qp const &qp, cppcoro::net::socket &socket) -> cppcoro::task<void>;

auto recv_qp(cppcoro::net::socket &socket) -> cppcoro::task<rdmapp::deserialized_qp>;

} // namespace coverbs_rpc
