#include <arpa/inet.h>
#include <cerrno>
#include <iostream>
#include <netinet/in.h>
#include <print>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

void print_system_error(std::string_view context) {
  std::error_code ec = std::make_error_code(static_cast<std::errc>(errno));
  std::println(std::cerr, "[ERROR] {}: {} (Code: {})", context, ec.message(),
               ec.value());
}

int main() {
  // Create socket
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

    while (true) {
      // Receive message from client
      char buffer[1024] = {0};
      ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

      if (bytes_received > 0) {
        buffer[bytes_received] = '\0';

        // Print to the server console
        std::println("[SERVER RECEIVED] {}", buffer);

        // Send echo back to the socket
        send(client_fd, buffer, bytes_received, 0);
      } else if (bytes_received == 0) {
        std::println("[INFO] Client disconnected");
        break;
      } else {
        print_system_error("Failed to receive data");
        break;
      }
    }

    // Clean up
    close(client_fd);
    std::println("[INFO] Connection closed. Ready for next client.");
  }

  close(server_fd);

  return 0;
}
