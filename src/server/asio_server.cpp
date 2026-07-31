#include "chat_server.hpp"
#include "utils.hpp"
#include <boost/asio.hpp>
#include <exception>
#include <print>

int run_asio_server() {
  try {
    boost::asio::io_context io_context;

    ChatServer servet(io_context, 55555);
    log_to_file("asio", "Server started on port 55555");

    io_context.run();
  } catch (const std::exception &e) {
    std::println(stderr, "[MAIN] Exception caught: {}", e.what());
    return 1;
  }
  return 0;
}
