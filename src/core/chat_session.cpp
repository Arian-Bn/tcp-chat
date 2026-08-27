#include "chat_session.hpp"
#include "protocol.hpp"
#include "utils.hpp"
#include <arpa/inet.h>
#include <boost/asio/write.hpp>
#include <cstring>
#include <print>
#include <utility>

ChatSession::ChatSession(tcp::socket socket, ChatRoom &room)
    : socket_(std::move(socket)), room_(room) {}

void ChatSession::start() {
  room_.join(shared_from_this());
  do_read();
}

void ChatSession::deliver(std::string_view message) {
  bool write_in_progress = !write_msgs_.empty();
  write_msgs_.emplace_back(make_protocol_message(message));
  if (!write_in_progress) {
    do_write();
  }
}

void ChatSession::do_read() {
  auto self = shared_from_this();

  if (reading_header_) {
    read_buffer_.resize(HEADER_SIZE);
    boost::asio::async_read(
        socket_, boost::asio::buffer(read_buffer_),
        [this, self](const boost::system::error_code &error, size_t) {
          if (!error) {
            uint32_t net_length;
            std::memcpy(&net_length, read_buffer_.data(), HEADER_SIZE);
            expected_length_ = ntohl(net_length);

            read_buffer_.resize(HEADER_SIZE + expected_length_);
            reading_header_ = false;
            do_read();
          } else if (error == boost::asio::error::eof) {
            std::println("[SESSION] Client disconnected gracefully.");
            log_to_file("asio", "Client disconnected");
          } else {
            std::println("[SESSION] Read error: {}", error.message());
          }
        });
  } else {
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_buffer_.data() + HEADER_SIZE,
                            expected_length_),
        [this, self](const boost::system::error_code &error, size_t) {
          if (!error) {
            std::string message(read_buffer_.data() + HEADER_SIZE,
                                expected_length_);
            std::println("[SESSION] Received {} bytes: {}", expected_length_,
                         message);

            room_.broadcast(message);

            read_buffer_.clear();
            reading_header_ = true;
            do_read();
          } else if (error == boost::asio::error::eof) {
            std::println("[SESSION] Client disconnected gracefully.");
            log_to_file("asio", "Client disconnected");
          } else {
            std::println("[SESSION] Read error: {}", error.message());
          }
        });
  }
}

void ChatSession::do_write() {
  auto self = shared_from_this();
  boost::asio::async_write(
      socket_, boost::asio::buffer(write_msgs_.front()),
      [this, self](const boost::system::error_code &error, size_t) {
        if (!error) {
          write_msgs_.pop_front();
          if (!write_msgs_.empty()) {
            do_write();
          }
        } else {
          std::println(stderr, "[SESSION] Write error: {}", error.message());
          room_.leave(shared_from_this());
        }
      });
}
