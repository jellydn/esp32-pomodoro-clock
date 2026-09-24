#pragma once

#include "pomodoro_types.h"

namespace pomodoro {

class Engine {
 public:
  explicit Engine(Durations durations = {});

  void start(std::uint64_t nowUs);
  void pause(std::uint64_t nowUs);
  void resume(std::uint64_t nowUs);
  void stopReset();
  void acknowledgeAlert();
  void update(std::uint64_t nowUs);
  void restore(const Snapshot& saved, std::uint64_t nowUs);

  Snapshot snapshot(std::uint64_t nowUs) const;

 private:
  std::uint64_t durationFor(Phase phase) const;
  std::uint64_t remainingAt(std::uint64_t nowUs) const;
  void completePhase();

  Durations durations_;
  State state_{State::Idle};
  Phase phase_{Phase::Focus};
  Phase nextPhase_{Phase::Focus};
  std::uint8_t completedFocusInCycle_{0};
  std::uint64_t startedAtUs_{0};
  std::uint64_t remainingAtStartUs_{0};
};

}  // namespace pomodoro
