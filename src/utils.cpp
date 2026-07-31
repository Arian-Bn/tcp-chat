#include "utils.hpp"
#include <chrono>
#include <format>
#include <fstream>

void log_to_file(std::string_view mode, std::string_view message) {
  auto now = std::chrono::system_clock::now();
  auto seconds = std::chrono::floor<std::chrono::seconds>(now);
  auto local_time =
      std::chrono::zoned_time(std::chrono::current_zone(), seconds);

  std::string time_str = std::format("{:%Y-%m-%d %H:%M:%S}", local_time);

  std::ofstream log_file("chat.log", std::ios::app);
  log_file << "[" << time_str << "] [" << mode << "] " << message << std::endl;
}
