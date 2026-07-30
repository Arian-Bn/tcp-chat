#include "ChatServer.hpp"
#include <boost/asio.hpp>
#include <exception>
#include <print>

int main() {
  try {
    boost::asio::io_context io_context;

    ChatServer servet(io_context, 55555);

    io_context.run();
  } catch (const std::exception &e) {
    std::println(stderr, "[MAIN] Exception caught: {}", e.what());
    return 1;
  }
  return 0;
}
