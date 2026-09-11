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

// Custom stateless deleter to enforce automatic exception-safe resource cleanup
struct SocketDeleter {
  void operator()(int *fd_ptr) const {
    int fd = reinterpret_cast<intptr_t>(fd_ptr);
    if (fd >= 0) {
      close(fd);
      std::println("[RAII] Client socket {} clean shutdown by OS.", fd);
    }
  }
};

using SafeSocket = std::unique_ptr<int, SocketDeleter>;

// Modern C++ wrapper to extract and print OS system error strings via
// std::error_code
void print_system_error(std::string_view context) {
  std::error_code ec = std::make_error_code(static_cast<std::errc>(errno));
  std::println(std::cerr, "[ERROR] {}: {} (Code: {})", context, ec.message(),
               ec.value());
}

// Background thread function: strictly handles incoming message from server
void received_message(int client_fd) {
  std::vector<char> buffer;
  char chunk[1024];

  while (true) {
    ssize_t byte_received = recv(client_fd, chunk, sizeof(chunk), 0);

    if (byte_received > 0) {
      // Append new raw wire fragments to the back of our accumulation buffer
      buffer.insert(buffer.end(), chunk, chunk + byte_received);

      // Process and slash multiple complete messages packed within a single TCP
      // packet burst
      while (true) {
        std::string msg = extract_message(buffer);
        if (msg.empty())
          break;

        buffer.erase(buffer.begin(), buffer.begin() + HEADER_SIZE + msg.size());
        std::println("\r[CHAT]: {}\n", msg);
      }
    } else if (byte_received == 0) {
      std::println("\r[INFO] Server closed the connection.");
      // Destructor in main thread will handle the raw descriptor cleanup safely
      std::exit(0);
    } else {
      print_system_error("Error receiving data");
      std::exit(1);
    }
  }
}

int main() {
  int raw_client = socket(AF_INET, SOCK_STREAM, 0);
  if (raw_client < 0) {
    print_system_error("Failed to create socket");
    return 1;
  }

  // Manage client lifecycle via EBO-optimized smart pointer
  SafeSocket client_holder(
      reinterpret_cast<int *>(static_cast<intptr_t>(raw_client)));
  int client_fd =
      static_cast<int>(reinterpret_cast<intptr_t>(client_holder.get()));

  // Configure server address
  struct sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(55555);
  if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
    print_system_error("Invalid loopback target address");
    return 1;
  }

  // Connect to server
  if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    print_system_error("Failed to connect to server");
    return 1;
  }

  std::println("[INFO] Connected to server! Type 'exit' to quit.");

  // Spin off an isolated reading thread while main thread manages interactive
  // user console inputs
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

    // Serialize text into our length-prefixed stream framing protocol layout
    auto packet = make_protocol_message(user_input);
    ssize_t bytes_sent =
        send(client_fd, packet.data(), packet.size(), MSG_NOSIGNAL);
    if (bytes_sent < 0) {
      print_system_error("Failed to dispatch protocol package");
      break;
    }
  }

  return 0;
}
