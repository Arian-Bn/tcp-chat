#include <arpa/inet.h>
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

  // Configure server address
  struct sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(55554);
  inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

  // Bind socket to address
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    std::cerr << "[ERROR] Failed to bind socket" << std::endl;
    close(server_fd);
    return 1;
  }

  // Start listening
  if (listen(server_fd, 5) < 0) {
    std::cerr << "[ERROR] Failed to listen on socket" << std::endl;
    close(server_fd);
    return 1;
  }

  std::cout << "[INFO] Server is listening on port 55554..." << std::endl;

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

  // Receive message from client
  char buffer[1024] = {0};
  ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

  if (bytes_received > 0) {
    std::cout << "[RECV] " << buffer << std::endl;
  } else if (bytes_received == 0) {
    std::cout << "[INFO] Client disconnected" << std::endl;
  } else {
    std::cerr << "[ERROR] Failed to receive data" << std::endl;
  }

  // Clean up
  close(client_fd);
  close(server_fd);

  return 0;
}
