#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  // Create socket
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    std::cerr << "[ERROR] Failed to create socket" << std::endl;
    return 1;
  }

  // Set up server address structure
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    std::cerr << "[ERROR] Setsockopt!" << std::endl;
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
    std::cerr << "[ERROR] Failed to bind socket: " << strerror(errno)
              << std::endl;
    close(server_fd);
    return 1;
  }

  // Start listening
  if (listen(server_fd, 5) < 0) {
    std::cerr << "[ERROR] Failed to listen on socket" << std::endl;
    close(server_fd);
    return 1;
  }

  std::cout << "[INFO] Server is listening on port 55555..." << std::endl;

  while (true) {
    // Accept incoming connection
    struct sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
      std::cerr << "[ERROR] Failed to accept connection" << std::endl;
      close(server_fd);
      return 1;
    }

    std::cout << "[INFO] Client connected!" << std::endl;

    while (true) {
      // Receive message from client
      char buffer[1024] = {0};
      ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

      if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::cout << "[RECV] " << buffer << std::endl;

        // Send echo back to the socket
        send(client_fd, buffer, bytes_received, 0);
      } else if (bytes_received == 0) {
        std::cout << "[INFO] Client disconnected" << std::endl;
        break;
      } else {
        std::cerr << "[ERROR] Failed to receive data" << std::endl;
        break;
      }
    }

    // Clean up
    close(client_fd);
    std::cout << "[INFO] Connection closed. Ready for next client. \n";
  }

  close(server_fd);

  return 0;
}
