#include "ChatSession.hpp"
#include <boost/asio/write.hpp>
#include <print>
#include <utility>

ChatSession::ChatSession(tcp::socket socket, ChatRoom &room)
    : socket_(std::move(socket)), room_(room) {}

void ChatSession::start() {
  room_.join(shared_from_this());
  do_read();
}

// Implemention of the ChatParticipant interface method
void ChatSession::deliver(std::string_view message) {
  bool write_in_progress = !write_msgs_.empty();

  write_msgs_.emplace_back(message);

  if (!write_in_progress) {
    do_write();
  }
}

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
          room_.broadcast(message);

          do_read();
        } else if (error == boost::asio::error::eof) {
          std::println("[SESSION] Client disconnected gracefully.");
        } else {
          std::println("[SESSION] Read error: {}", error.message());
        }
      });
}

// Write messages from the queue back to the client socket asynchronously
void ChatSession::do_write() {
  auto self = shared_from_this();

  boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front()),
                           [this, self](const boost::system::error_code &error,
                                        std::size_t /*bytes_transferred*/) {
                             if (!error) {
                               write_msgs_.pop_front();
                               if (!write_msgs_.empty()) {
                                 do_write();
                               }
                             } else {
                               std::println(stderr, "[SESSION] Write error: {}",
                                            error.message());
                               room_.leave(shared_from_this());
                             }
                           });
}
