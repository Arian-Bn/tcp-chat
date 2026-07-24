#pragma once

#include <cstdint>
#include <string>
#include <vector>

constexpr uint32_t HEADER_SIZE = 4;

// Send a message with a lenght prefix
std::vector<char> make_protocol_message(std::string_view message);

// Receive a message from the protocol (only if data is available)
std::string extract_message(const std::vector<char> &data);
