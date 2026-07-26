#include "protocol.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <print>
#include <string>
#include <sys/socket.h>
#include <system_error>
#include <thread>
#include <unistd.h>

void print_system_error(std::string_view context) {
  std::error_code ec = std::make_error_code(static_cast<std::errc>(errno));
  std::println(std::cerr, "[ERROR] {}: {} (Code: {})", context, ec.message(),
               ec.value());
}

// Background thread function: strictly handles incoming message from server
void received_message(int client_fd) {
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
        std::println("\r[RECV] {}\n", msg);
        std::println("> ");
      }
    } else if (byte_received == 0) {
      std::println("\r[INFO] Server closed the connection.");
      close(client_fd);
      std::exit(0);
    } else {
      print_system_error("Error receiving data");
      close(client_fd);
      std::exit(1);
    }
  }
}

int main() {
  // Create socket
  int client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_fd == -1) {
    print_system_error("Failed to create socket");
    return 1;
  }

  // Configure server address
  struct sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(55555);
  inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

  // Connect to server
  if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    print_system_error("Failed to connect to server");
    close(client_fd);
    return 1;
  }

  std::println("[INFO] Connected to server! Type 'exit' to quit.");

  // SPAWN BACKGROUND THREAD: Hand over the socket listening task to it
  std::thread recv_thread(received_message, client_fd);
  recv_thread.detach();

  std::string user_input;
  while (true) {
    std::cout << "> ";
    std::getline(std::cin, user_input);

    if (user_input == "exit") {
      std::println("[INFO] Exiting...");
      break;
    }

    if (user_input.empty()) {
      continue;
    }

    // Send data to server using protocol
    auto packet = make_protocol_message(user_input);
    ssize_t bytes_sent = send(client_fd, packet.data(), packet.size(), 0);
    if (bytes_sent < 0) {
      print_system_error("Failed to send message");
      break;
    }
  }

  // Clean up
  close(client_fd);

  return 0;
}
