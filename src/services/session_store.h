#pragma once

#include <cstdint>

#include "core/pomodoro_engine.h"

namespace services {

class SessionStore {
 public:
  bool restore(pomodoro::Engine& engine, std::uint64_t nowUs, bool wallTimeValid);
  void save(const pomodoro::Snapshot& snapshot, bool wallTimeValid);
};

}  // namespace services
