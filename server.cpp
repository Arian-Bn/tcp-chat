#include "protocol.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <format>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <print>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>
#include <unordered_map>
#include <vector>

// Global storage for connected clients and its synchronization mutex
std::vector<int> active_clients;
std::unordered_map<int, std::vector<char>> client_buffers;

// Log a timestamped message to chat.log for server-size monitoring
void log_to_file(std::string_view message) {
  auto now = std::chrono::system_clock::now();
  auto seconsd = std::chrono::floor<std::chrono::seconds>(now);

  auto local_time =
      std::chrono::zoned_time(std::chrono::current_zone(), seconsd);

  std::string time_str = std::format("{:%Y-%m-%d %H:%M:%S}", local_time);

  std::ofstream log_file("chat.log", std::ios::app);
  log_file << "[" << time_str << "] " << message << std::endl;
}

void print_system_error(std::string_view context) {
  std::error_code ec = std::make_error_code(static_cast<std::errc>(errno));
  std::println(std::cerr, "[ERROR] {}: {} (Code: {})", context, ec.message(),
               ec.value());
}

void set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    print_system_error("fcntl F_GETFL");
    return;
  }

  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    print_system_error("fcntl F_SETFL O_NONBLOCK");
  }
}

int main() {
  // Create server socket
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (server_fd == -1) {
    print_system_error("Failed to create socket");
    return 1;
  }

  // Set up server address structure
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    print_system_error("Setsockopt failed");
    close(server_fd);
    return 1;
  }

  // Configure server address
  struct sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(55555);
  inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

  // Bind socket to address
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    print_system_error("Failed to bind socket");
    close(server_fd);
    return 1;
  }

  // Start listening
  if (listen(server_fd, 5) < 0) {
    print_system_error("Failed to listen on socket");
    close(server_fd);
    return 1;
  }

  // Create epoll descriptor
  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    print_system_error("epoll_create1 failed");
    close(server_fd);
    return 1;
  }

  // Add server socket to epoll
  struct epoll_event ev{};
  ev.events = EPOLLIN;
  ev.data.fd = server_fd;

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
    print_system_error("epoll_ctl: add server_fd");
    close(server_fd);
    close(epoll_fd);
    return 1;
  }

  std::println("[INFO] Server is listening on port 55555 (epoll version)");

  // Main epoll loop
  const int MAX_EVENTS = 64;
  struct epoll_event events[MAX_EVENTS];

  while (true) {
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (nfds == -1) {
      print_system_error("epoll_wait failed");
      break;
    }

    for (int i = 0; i < nfds; i++) {
      int fd = events[i].data.fd;

      if (fd == server_fd) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd =
            accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue;
          }
          print_system_error("Faild to accept connection");
          continue;
        }

        set_nonblocking(client_fd);

        struct epoll_event ev_client{};
        ev_client.events = EPOLLIN | EPOLLET;
        ev_client.data.fd = client_fd;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev_client) == -1) {
          print_system_error("epoll_ctl: add client_fd");
          close(client_fd);
          continue;
        }

        active_clients.push_back(client_fd);
        std::println("[INFO] New client connected: fd={}", client_fd);
      } else {
        // Reading data from client
        bool connection_closed = false;

        while (true) {
          char chunk[1024];
          ssize_t bytes_read = recv(fd, chunk, sizeof(chunk) - 1, 0);

          if (bytes_read > 0) {
            client_buffers[fd].insert(client_buffers[fd].end(), chunk,
                                      chunk + bytes_read);
            while (true) {
              std::string msg = extract_message(client_buffers[fd]);
              if (msg.empty())
                break;

              // Remove processed bytes
              client_buffers[fd].erase(client_buffers[fd].begin(),
                                       client_buffers[fd].begin() +
                                           HEADER_SIZE + msg.size());
              std::println("[Client {}] {}", fd, msg);

              for (int order_fd : active_clients) {
                if (order_fd != fd) {
                  std::string broadcast_msg =
                      std::format("[Client {}] {}", fd, msg);
                  auto packet = make_protocol_message(broadcast_msg);
                  send(order_fd, packet.data(), packet.size(), 0);
                  std::println("[BCAST] [Client {} -> Client {}]: {}", fd,
                               order_fd, msg);
                }
              }
              // Echo back to sender
              auto echo_packet = make_protocol_message(msg);
              send(fd, echo_packet.data(), echo_packet.size(), 0);
              std::println("[ECHO] [Server -> Client {}]: {}", fd, msg);
            }
          } else if (bytes_read == 0) {
            std::println("[INFO] Client {} disconnected", fd);
            connection_closed = true;
            break;

          } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
              break;
            }
            print_system_error("recv failed");
            connection_closed = true;
            break;
          }
        }

        if (connection_closed) {
          epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
          close(fd);
          client_buffers.erase(fd);
          active_clients.erase(
              std::remove(active_clients.begin(), active_clients.end(), fd),
              active_clients.end());
        }
      }
    }
  }

  close(server_fd);

  return 0;
}
