#include "coverbs_rpc/conn/acceptor.hpp"
#include "coverbs_rpc/conn/connector.hpp"
#include "coverbs_rpc/logger.hpp"
#include "coverbs_rpc/typed_client.hpp"
#include "coverbs_rpc/typed_server.hpp"

#include <cppcoro/async_scope.hpp>
#include <cppcoro/io_service.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>
#include <memory>
#include <rdmapp/rdmapp.h>
#include <thread>

struct EchoReq {
  std::string msg;
};

struct EchoResp {
  std::string msg;
};

auto echo(const EchoReq &req) -> EchoResp { return EchoResp{.msg = "Echo: " + req.msg}; }

cppcoro::task<void> qp_wrapper(std::shared_ptr<rdmapp::qp> qp, auto &&config) {
  coverbs_rpc::typed_server server(qp, config);
  server.register_handler<echo>();
  co_await server.run();
}

cppcoro::task<void> run_server(cppcoro::io_service &io_service, uint16_t port,
                               std::shared_ptr<rdmapp::pd> pd, coverbs_rpc::RpcConfig config) {
  coverbs_rpc::qp_acceptor acceptor(io_service, port, pd);
  cppcoro::async_scope scope;
  while (true) {
    auto qp = co_await acceptor.accept();
    scope.spawn(qp_wrapper(qp, config));
  }
}

cppcoro::task<void> run_client(cppcoro::io_service &io_service, std::string hostname, uint16_t port,
                               std::shared_ptr<rdmapp::pd> pd, coverbs_rpc::RpcConfig config) {
  coverbs_rpc::qp_connector connector(io_service, pd);
  auto qp = co_await connector.connect(hostname, port);
  coverbs_rpc::typed_client client(qp, config);

  EchoReq req{.msg = "Hello Typed RPC!"};
  auto resp = co_await client.call<echo>(req);
  coverbs_rpc::get_logger()->info("Received: {}", resp.msg);

  if (resp.msg == "Echo: Hello Typed RPC!") {
    coverbs_rpc::get_logger()->info("Test Passed!");
  } else {
    coverbs_rpc::get_logger()->error("Test Failed!");
  }
}

auto main(int argc, char *argv[]) -> int {
  auto device = std::make_shared<rdmapp::device>(0, 1);
  auto pd = std::make_shared<rdmapp::pd>(device);
  coverbs_rpc::RpcConfig config{.max_req_payload = 1024, .max_resp_payload = 1024};

  cppcoro::io_service io_service;
  auto looper = std::jthread([&io_service]() { io_service.process_events(); });

  if (argc == 2) {
    cppcoro::sync_wait(run_server(io_service, std::stoi(argv[1]), pd, config));
  } else if (argc == 3) {
    cppcoro::sync_wait(run_client(io_service, argv[1], std::stoi(argv[2]), pd, config));
  } else {
    coverbs_rpc::get_logger()->info(
        "Usage: {} [port] for server and {} [server_ip] [port] for client", argv[0], argv[0]);
  }

  io_service.stop();
  return 0;
}
