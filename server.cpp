#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <format>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <print>
#include <pstl/glue_algorithm_defs.h>
#include <sys/socket.h>
#include <system_error>
#include <thread>
#include <unistd.h>
#include <vector>

// Global storage for connected clients and its synchronization mutex
std::vector<int> active_clients;
std::mutex clients_mutex;

void print_system_error(std::string_view context) {
  std::error_code ec = std::make_error_code(static_cast<std::errc>(errno));
  std::println(std::cerr, "[ERROR] {}: {} (Code: {})", context, ec.message(),
               ec.value());
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

  while (true) {
    char buffer[1024] = {0};
    ssize_t byte_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (byte_received > 0) {
      buffer[byte_received] = '\0';
      std::println("[SERVER RECEIVED FROM fd {}] {}", client_fd, buffer);

      // BROADCEST: Send this message to all other connected clients
      std::string broadcast_text =
          std::format("[Client {}] {}", client_fd, buffer);
      broadcast_message(broadcast_text, client_fd);

      // Echo back to the sender just to unblock out custom C++ client recv()
      // loop
      send(client_fd, buffer, byte_received, 0);
    } else { // Client disconnected or error occurred
      if (byte_received == 0) {
        std::println("[INFO] Client on fd {} disconnected", client_fd);
      } else {
        print_system_error("Gailed to received data");
      }
      break;
    }
  }

  // Cleaan up: remove client from global vector and close socket
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
