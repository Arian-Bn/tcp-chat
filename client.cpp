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
  server_addr.sin_port = htons(55554);
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
  } else {
    std::cerr << "[ERROR] Failed to send message" << std::endl;
  }

  // Clean up
  close(client_fd);

  return 0;
}
