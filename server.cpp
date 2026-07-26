#include "protocol.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <print>
#include <sys/socket.h>
#include <system_error>
#include <thread>
#include <unistd.h>
#include <vector>

// Global storage for connected clients and its synchronization mutex
std::vector<int> active_clients;
std::mutex clients_mutex;

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

// Function to broadcast message to EVERYONE except the sender
void broadcast_message(std::string_view message, int sender_fd) {
  std::lock_guard<std::mutex> lock(clients_mutex);

  for (int client_fd : active_clients) {
    if (client_fd != sender_fd) {
      send(client_fd, message.data(), message.length(), 0);
    }
  }
}

// Thread worker function for each connected client
void handle_client(int client_fd) {
  std::println("[INFO] Thread started for client fd: {}", client_fd);
  log_to_file(std::format("Client connected: fd={}", client_fd));

  std::vector<char> buffer;

  while (true) {
    char chunk[1024];
    ssize_t byte_received = recv(client_fd, chunk, sizeof(chunk), 0);

    if (byte_received > 0) {
      buffer.insert(buffer.end(), chunk, chunk + byte_received);

      while (true) {
        std::string msg = extract_message(buffer);
        if (msg.empty())
          break;

        buffer.erase(buffer.begin(), buffer.begin() + HEADER_SIZE + msg.size());

        std::println("[SERVER RECEIVED FROM fd {}] {}", client_fd, msg);

        // BROADCEST: Send this message to all other connected clients
        std::string broadcast_text =
            std::format("[Client {}] {}", client_fd, msg);
        broadcast_message(broadcast_text, client_fd);

        // Echo back to the sender just to unblock out custom C++ client recv()
        auto packet = make_protocol_message(msg);
        send(client_fd, packet.data(), packet.size(), 0);
      }
    } else { // Client disconnected or error occurred
      if (byte_received == 0) {
        std::println("[INFO] Client on fd {} disconnected", client_fd);
        log_to_file(std::format("Client disconnection: fd={}", client_fd));
      } else {
        print_system_error("Failed to received data");
      }
      break;
    }
  }

  // Clean up: remove client from global vector and close socket
  {
    std::lock_guard<std::mutex> lock(clients_mutex);
    active_clients.erase(
        std::remove(active_clients.begin(), active_clients.end(), client_fd),
        active_clients.end());
  }
  close(client_fd);
  std::println("[INFO] Connection on fd {} closed. Thread exiting.", client_fd);
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

  std::println("[INFO] Server is listening on port 55555...");

  while (true) {
    // Accept incoming connection
    struct sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
      print_system_error("Failed to accept connection");
      continue;
    }
    set_nonblocking(client_fd);

    std::println("[INFO] Client connected!");

    // Add new client to global list safely using mutex
    {
      std::lock_guard<std::mutex> lock(clients_mutex);
      active_clients.push_back(client_fd);
    }

    std::thread client_thread(handle_client, client_fd);
    client_thread.detach();
  }
  close(server_fd);

  return 0;
}
