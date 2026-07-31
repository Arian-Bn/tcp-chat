#include "protocol.hpp"
#include <arpa/inet.h>
#include <cstring>

// Pack: length prefix (4 bytes) + message data
std::vector<char> make_protocol_message(std::string_view message) {
  uint32_t length = static_cast<uint32_t>(message.size());
  std::vector<char> result(HEADER_SIZE + length);

  // Write length in network bytes order (big-endian)
  uint32_t net_lenght = htonl(length);
  std::memcpy(result.data(), &net_lenght, HEADER_SIZE);

  // Copy the actual message
  std::memcpy(result.data() + HEADER_SIZE, message.data(), length);

  return result;
}

// Unpack: extract the first complete message from raw byte buffer
std::string extract_message(const std::vector<char> &date) {
  if (date.size() < HEADER_SIZE)
    return {};

  uint32_t net_length;
  std::memcpy(&net_length, date.data(), HEADER_SIZE);

  uint32_t length = ntohl(net_length);

  if (date.size() < HEADER_SIZE + length)
    return {};

  return std::string(date.data() + HEADER_SIZE, length);
}
