#include "coverbs_rpc/conn/acceptor.hpp"
#include "coverbs_rpc/conn/transmission.hpp"
#include "coverbs_rpc/logger.hpp"

#include <cppcoro/io_service.hpp>
#include <cppcoro/net/ipv4_address.hpp>
#include <cppcoro/net/ipv4_endpoint.hpp>

namespace coverbs_rpc {

qp_acceptor::qp_acceptor(cppcoro::io_service &io_service, uint16_t port,
                         std::shared_ptr<pd> pd, std::shared_ptr<srq> srq)
    : acceptor_socket_(cppcoro::net::socket::create_tcpv4(io_service)), pd_(pd),
      srq_(srq), port_(port), io_service_(io_service) {
  try {
    acceptor_socket_.bind(
        cppcoro::net::ipv4_endpoint(cppcoro::net::ipv4_address(), port));
    acceptor_socket_.listen();
    get_logger()->info("qp_acceptor: bind and listen: port={}", port_);
  } catch (std::exception &e) {
    get_logger()->error("qp_acceptor: failed to bind and listen: error={}",
                        e.what());
    std::terminate();
  }
}

auto qp_acceptor::accept_qp(cppcoro::net::socket &socket,
                            std::shared_ptr<rdmapp::cq> send_cq,
                            std::shared_ptr<rdmapp::cq> recv_cq)
    -> cppcoro::task<std::shared_ptr<qp_t>> {
  auto remote_qp = co_await recv_qp(socket);
  auto local_qp = std::make_shared<qp_t>(
      remote_qp.header.lid, remote_qp.header.qp_num, remote_qp.header.sq_psn,
      remote_qp.header.gid, pd_, recv_cq, send_cq);
  local_qp->user_data() = std::move(remote_qp.user_data);
  co_await send_qp(*local_qp, socket);
  co_return local_qp;
}

auto qp_acceptor::accept() -> cppcoro::task<std::shared_ptr<qp_t>> {
  cppcoro::net::socket socket = cppcoro::net::socket::create_tcpv4(io_service_);
  co_await acceptor_socket_.accept(socket);
  co_return co_await accept_qp(socket, alloc_cq(), alloc_cq());
}

auto qp_acceptor::accept_multiple(qp_handshake &handshake)
    -> cppcoro::task<std::vector<std::shared_ptr<qp_t>>> {
  cppcoro::net::socket socket = cppcoro::net::socket::create_tcpv4(io_service_);
  co_await acceptor_socket_.accept(socket);
  get_logger()->info("qp_acceptor: accept client handshake: remote={}",
                     socket.remote_endpoint().to_string());

  handshake = co_await recv_handshake(socket);
  get_logger()->info("qp_acceptor: handshake nr_qp={} sid={}", handshake.nr_qp,
                     handshake.sid);

  std::vector<std::shared_ptr<qp_t>> result;
  result.reserve(handshake.nr_qp);
  for (unsigned int i = 0; i < handshake.nr_qp; i++) {
    auto cq = alloc_cq();
    result.emplace_back(co_await accept_qp(socket, cq, cq));
  }
  get_logger()->info("qp_acceptor: accept nr_qp={} sid={}", handshake.nr_qp,
                     handshake.sid);
  co_return result;
}

auto qp_acceptor::alloc_cq() -> std::shared_ptr<rdmapp::cq> {
  auto cq = std::make_shared<rdmapp::cq>(pd_->device_ptr(), 2048);
  pollers_.emplace_back(cq);
  return cq;
}

auto qp_acceptor::close() noexcept -> void {
  try {
    acceptor_socket_.close();
    get_logger()->warn("qp_acceptor: closed");
  } catch (std::exception &e) {
    get_logger()->error("qp_acceptor: error close, msg={}", e.what());
  }
}

} // namespace coverbs_rpc
