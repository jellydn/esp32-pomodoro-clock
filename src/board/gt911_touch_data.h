#pragma once

#include <cstdint>

namespace board {

constexpr std::uint16_t kGt911FirstPointRegister = 0x814F;

struct Gt911PointData {
  std::uint8_t trackId;
  std::uint8_t xLow;
  std::uint8_t xHigh;
  std::uint8_t yLow;
  std::uint8_t yHigh;
  std::uint8_t sizeLow;
  std::uint8_t sizeHigh;
  std::uint8_t reserved;
};

static_assert(sizeof(Gt911PointData) == 8, "GT911 point records are eight bytes");

inline std::uint16_t gt911X(const Gt911PointData& point) {
  return static_cast<std::uint16_t>(point.xLow | (point.xHigh << 8U));
}

inline std::uint16_t gt911Y(const Gt911PointData& point) {
  return static_cast<std::uint16_t>(point.yLow | (point.yHigh << 8U));
}

}  // namespace board
