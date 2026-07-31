#pragma once
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class ChatServer {
public:
  ChatServer(boost::asio::io_context &io_context, int port);

private:
  void start_accept();

  boost::asio::io_context &io_context_;
  tcp::acceptor acceptor_;
};
