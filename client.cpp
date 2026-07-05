#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  // Create socket
  int client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_fd == -1) {
    std::cerr << "[ERROR] Failed to create socket" << std::endl;
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
    std::cerr << "[ERROR] Failed to connect to server" << std::endl;
    close(client_fd);
    return 1;
  }

  std::cout << "[INFO] Connected to server!" << std::endl;

  // Send message
  const char *message = "Hello from client!";
  ssize_t bytes_sent = send(client_fd, message, strlen(message), 0);

  if (bytes_sent > 0) {
    std::cout << "[SENT] " << message << std::endl;

    char response_buffer[1024]{};
    ssize_t byte_received =
        recv(client_fd, response_buffer, sizeof(response_buffer), 0);
    if (byte_received > 0) {
      std::cout << "[ECHO FROM SERVER] " << response_buffer << std::endl;
    } else {
      std::cerr << "[ERROR] Failed to get echo from server" << std::endl;
    }

  } else {
    std::cerr << "[ERROR] Failed to send message" << std::endl;
  }

  // Clean up
  close(client_fd);

  return 0;
}
