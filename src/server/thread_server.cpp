#include "utils.hpp"
#include <arpa/inet.h>
#include <format>
#include <memory>
#include <print>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

// Custom stateless deleter to enforce RAII for POSIX file descriptors
struct SocketDeleter {
  void operator()(int *fd_ptr) const {
    // Extract the raw file descriptor using pointer conversion arithmetic
    int fd = reinterpret_cast<intptr_t>(fd_ptr);
    if (fd >= 0) {
      close(fd);
      std::println("[RAII] Socket {} successfully closed by the OS!", fd);
    }
  }
};

// Type alias for zero-overhead resource management via Empty Base Optimization
// (EBO)
using SafeSocket = std::unique_ptr<int, SocketDeleter>;

void handle_client(int raw_client_fd) {
  // Transfer raw descriptor ownership to unique_ptr to guarantee cleanup even
  // if an exception occurs
  SafeSocket client_fd(
      reinterpret_cast<int *>(static_cast<intptr_t>(raw_client_fd)));
  char buffer[1024];
  while (true) {
    int fd = static_cast<int>(reinterpret_cast<intptr_t>(client_fd.get()));
    ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

    // Explicitly distinguish between EOF (clean disconnect) and socket network
    // errors
    if (bytes <= 0) {
      if (bytes < 0) {
        std::perror("[ERROR] recv failed");
      } else {
        std::println("[THREAD] Client disconnected cleanly.");
      }
      break;
    }

    buffer[bytes] = '\0';
    std::println("[THREAD] Received: {}", buffer);

    // Prevent process crashes via MSG_NOSIGNAL if the remote client terminates
    // abruptly
    send(fd, buffer, bytes, MSG_NOSIGNAL);
  }
  log_to_file("threads", "Client disconnected");
}

int run_threaded_server() {
  int raw_server = socket(AF_INET, SOCK_STREAM, 0);
  if (raw_server < 0) {
    std::perror("[ERROR] socket failed");
    return 1;
  }

  // Manage server lifecycle using the same exception-safe RAII logic
  SafeSocket server_fd(
      reinterpret_cast<int *>(static_cast<intptr_t>(raw_server)));
  int s_fd = static_cast<int>(reinterpret_cast<intptr_t>(server_fd.get()));

  // Allow immediate socket re-binding to bypass the TCP TIME_WAIT state
  int reuse = 1;
  if (setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::perror("[ERROR] setsockopt SO_REUSEADDR failed");
    return 1;
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(55555);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(s_fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
    std::perror("[ERROR] bind");
    return 1;
  }

  if (listen(s_fd, 5) < 0) {
    std::perror("[ERROR] listen");
    return 1;
  }

  std::println("[THREAD] Server listening on port 55555");

  while (true) {
    sockaddr_in client_addr{};
    socklen_t len = sizeof(client_addr);

    // Accept blocks until a new TCP connection request arrives in the queue
    int client_raw = accept(s_fd, (sockaddr *)&client_addr, &len);
    if (client_raw < 0) {
      std::perror("[ERROR] accept failed");
      continue;
    }

    std::println("[THREAD] New client connected");
    log_to_file("threads", std::format("Client connected"));

    // Delegate the connection processing to an isolated, detached worker thread
    std::thread(handle_client, client_raw).detach();
  }

  return 0;
}
