#include "ChatSession.hpp"
#include <print>
#include <utility>

ChatSession::ChatSession(tcp::socket socket) : socket_(std::move(socket)) {}

void ChatSession::start() { do_read(); }

void ChatSession::do_read() {

  auto self = shared_from_this();
  socket_.async_read_some(
      boost::asio::buffer(buffer_),
      [this, self](const boost::system::error_code &error,
                   std::size_t bytes_transferred) {
        if (!error) {
          std::string_view message(buffer_.data(), bytes_transferred);

          std::println("[SESSION] Received {} bytes: {}", bytes_transferred,
                       message);
          do_read();
        } else if (error == boost::asio::error::eof) {
          std::println("[SESSION] Client disconnected gracefully.");
        } else {
          std::println("[SESSION] Read error: {}", error.message());
        }
      });
}
