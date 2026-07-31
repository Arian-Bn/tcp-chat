#pragma once

#include <string_view>

// Log a timestamped message to chat.log for server-size monitoring
void log_to_file(std::string_view mode, std::string_view message);
