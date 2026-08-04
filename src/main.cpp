#include <print>
#include <string_view>

int run_threaded_server();
int run_epoll_server();
int run_asio_server();

int main(int argc, char *argv[]) {
  // Default value
  std::string_view mode = "epoll";

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
  } else {
    std::println("[MAIN] Unknown mode: {}. Use: threads, epoll, asio", mode);
    return 1;
  }

  return 0;
}
