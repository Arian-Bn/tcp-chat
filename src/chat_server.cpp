#include "chat_server.hpp"
#include "chat_session.hpp"
#include "utils.hpp"
#include <memory>
#include <print>

ChatServer::ChatServer(boost::asio::io_context &io_context, int port)
    : io_context_(io_context),
      acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)) {
  std::println("[SERVER] Started on port {}. Waiting for clients...", port);
  start_accept();
}

void ChatServer::start_accept() {
  auto socket = std::make_shared<tcp::socket>(io_context_);

  acceptor_.async_accept(*socket, [this, socket](
                                      const boost::system::error_code &error) {
    if (!error) {
      std::println("[SERVER] Successful connection from: {}",
                   socket->remote_endpoint().address().to_string());
      log_to_file("asio",
                  std::format("Client connected from {}",
                              socket->remote_endpoint().address().to_string()));
      static ChatRoom temporary_room;
      std::make_shared<ChatSession>(std::move(*socket), temporary_room)
          ->start();
    } else {
      std::println(stderr, "[SERVER] Accept error: {}", error.message());
    }

    start_accept();
  });
}
