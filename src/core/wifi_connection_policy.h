#pragma once

#include <cstddef>
#include <cstdint>

namespace wifi {

enum class DisconnectFailure : std::uint8_t {
  Ignore,
  NetworkUnavailable,
  Authentication,
  UnsupportedSecurity,
  Connection,
};

DisconnectFailure classifyDisconnectReason(std::uint8_t reason);
bool isValidPersonalPassword(const char* password, std::size_t length);

}  // namespace wifi
