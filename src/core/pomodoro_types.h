#pragma once

#include <cstdint>

namespace pomodoro {

enum class Phase : std::uint8_t {
  Focus,
  ShortBreak,
  LongBreak,
};

enum class State : std::uint8_t {
  Idle,
  Running,
  Paused,
  Alert,
};

struct Durations {
  std::uint32_t focusSeconds{25U * 60U};
  std::uint32_t shortBreakSeconds{5U * 60U};
  std::uint32_t longBreakSeconds{15U * 60U};
};

struct Snapshot {
  State state;
  Phase phase;
  Phase nextPhase;
  std::uint8_t completedFocusInCycle;
  std::uint64_t remainingMs;
  std::uint64_t durationMs;
};

}  // namespace pomodoro
