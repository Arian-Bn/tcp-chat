#pragma once
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// We inherit from std::enable_shared_from_this so the session
// Can keep itself alive during asynchronous background operarion.

class ChatSession : public std::enable_shared_from_this<ChatSession> {
public:
  explicit ChatSession(tcp::socket socket);

  void start();

private:
  void do_read();

  tcp::socket socket_;
  std::array<char, 1024> buffer_;
};
