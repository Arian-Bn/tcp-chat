#pragma once
#include "chat_room.hpp"
#include <boost/asio.hpp>
#include <deque>

using boost::asio::ip::tcp;

class ChatSession : public ChatParticipant,
                    public std::enable_shared_from_this<ChatSession> {
public:
  explicit ChatSession(tcp::socket socket, ChatRoom &room);
  void start();
  void deliver(std::string_view message) override;

private:
  void do_read();
  void do_write();

  tcp::socket socket_;
  ChatRoom &room_;
  std::array<char, 1024> buffer_;
  std::deque<std::string> write_msgs_;
};
