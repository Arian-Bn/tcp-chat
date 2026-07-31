#pragma once

#include <cstdint>
#include <string>
#include <vector>

constexpr uint32_t HEADER_SIZE = 4;

// Pack: length prefix (4 bytes) + message data
std::vector<char> make_protocol_message(std::string_view message);

// Unpack: extract the first complete message from raw byte buffer
std::string extract_message(const std::vector<char> &data);
