#include <arpa/inet.h>
#include <iostream>
#include <print>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

void handle_client(int client_fd) {
  char buffer[1024];
  while (true) {
    ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0)
      break;
    buffer[bytes] = '\0';
    std::println("[THREAD] Received: {}", buffer);
    // echo
    send(client_fd, buffer, bytes, 0);
  }
  close(client_fd);
}

int run_threaded_server() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    std::cerr << "[ERROR] socket failed\n";
    return 1;
  }

  int reuse = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(55555);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
    std::cerr << "[ERROR] bind\n";
    return 1;
  }

  if (listen(server_fd, 5) < 0) {
    std::cerr << "[ERROR] listen\n";
    return 1;
  }

  std::println("[THREAD] Server listening on port 55555");

  while (true) {
    sockaddr_in client_addr{};
    socklen_t len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr *)&client_addr, &len);
    if (client_fd < 0)
      continue;

    std::println("[THREAD] New client connected");
    std::thread(handle_client, client_fd).detach();
  }

  close(server_fd);
  return 0;
}
