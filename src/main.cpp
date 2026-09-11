#include <print>
#include <signal.h>
#include <string_view>

int run_threaded_server();
int run_epoll_server();
int run_asio_server();

int main(int argc, char *argv[]) {
  // Prevent process termination when writing to a broken TCP socket
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    std::perror("[ERROR] Failed to ignore SIGPIPE");
    return 1;
  }

  // Set default execution mode using zero-allocation string_view
  std::string_view mode = "epoll";

  // Parse command line arguments for the server execution mode
  for (int i = 1; i < argc; i++) {
    if (std::string_view(argv[i]) == "--mode" && i + 1 < argc) {
      mode = argv[i + 1];
      break;
    }
  }

  std::println("[MAIN] Selected mode: {}", mode);

  if (mode == "threads") {
    std::println("[MAIN] Starting multithreaded server...");
    return run_threaded_server();
  } else if (mode == "epoll") {
    std::println("[MAIN] Starting epoll server...");
    return run_epoll_server();
  } else if (mode == "asio") {
    std::println("[MAIN] Starting Boost.Asio server...");
    return run_asio_server();
  }

  std::println("[MAIN] Unknown mode: {}. Use: threads, epoll, asio", mode);
  return 1;
}
