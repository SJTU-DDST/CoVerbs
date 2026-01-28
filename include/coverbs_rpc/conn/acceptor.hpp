#pragma once

#include <cppcoro/net/socket.hpp>
#include <cppcoro/task.hpp>
#include <cstdint>

#include <list>
#include <memory>
#include <rdmapp/device.h>
#include <rdmapp/pd.h>
#include <rdmapp/qp.h>

#include <rdmapp/cq.h>
#include <rdmapp/cq_poller.h>

#include "coverbs_rpc/conn/transmission.hpp"

namespace cppcoro {
class io_service;
}

namespace coverbs_rpc {

struct qp_acceptor {
  using pd = rdmapp::pd;
  using cq = rdmapp::cq;
  using srq = rdmapp::srq;
  using qp_t = rdmapp::basic_qp;
  using cq_poller_t = rdmapp::native_cq_poller;

  qp_acceptor(cppcoro::io_service &io_service, uint16_t port,
              std::shared_ptr<pd> pd, std::shared_ptr<srq> srq = nullptr);

  auto accept() -> cppcoro::task<std::shared_ptr<qp_t>>;

  auto accept_multiple(qp_handshake &handshake)
      -> cppcoro::task<std::vector<std::shared_ptr<qp_t>>>;

  auto close() noexcept -> void;

  ~qp_acceptor() = default;

private:
  auto accept_qp(cppcoro::net::socket &socket,
                 std::shared_ptr<rdmapp::cq> send_cq,
                 std::shared_ptr<rdmapp::cq> recv_cq)
      -> cppcoro::task<std::shared_ptr<qp_t>>;

  auto alloc_cq() -> std::shared_ptr<rdmapp::cq>;

  cppcoro::net::socket acceptor_socket_;
  std::shared_ptr<pd> pd_;
  std::shared_ptr<srq> srq_;
  std::list<cq_poller_t> pollers_;
  uint16_t const port_;
  cppcoro::io_service &io_service_;
};

} // namespace coverbs_rpc
