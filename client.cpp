#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
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

  std::string user_input;
  while (true) {
    std::cout << "> ";
    std::getline(std::cin, user_input);

    if (user_input == "exit") {
      std::cout << "[INFO] Exiting..." << std::endl;
      break;
    }

    if (user_input.empty()) {
      continue;
    }

    // Send data to server
    ssize_t bytes_sent =
        send(client_fd, user_input.c_str(), user_input.length(), 0);
    if (bytes_sent < 0) {
      std::cerr << "[ERROR] Failed to send message" << std::endl;
      break;
    }

    // Receive echo back from server
    char buffer[1024]{};
    // Level 1 byte free at the end for the null terminator
    ssize_t byte_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (byte_received > 0) {
      // Ecplicitly null-terminate the receivec raw bytes to safely use
      // std::cout
      buffer[byte_received] = '\0';
      std::cout << "[ECHO FROM SERVER] " << buffer << std::endl;
    } else if (byte_received == 0) {
      std::cout << "[INFO] Server closed the connection" << std::endl;
      break;
    } else {
      std::cerr << "[ERROR] Failed to get echo from server" << std::endl;
      break;
    }
  }

  // Clean up
  close(client_fd);

  return 0;
}
