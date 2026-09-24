#include "pomodoro_engine.h"

namespace pomodoro {

Engine::Engine(Durations durations) : durations_(durations) {
  remainingAtStartUs_ = durationFor(phase_);
}

void Engine::start(std::uint64_t nowUs) {
  if (state_ != State::Idle) {
    return;
  }
  remainingAtStartUs_ = durationFor(phase_);
  startedAtUs_ = nowUs;
  state_ = State::Running;
}

void Engine::pause(std::uint64_t nowUs) {
  if (state_ != State::Running) {
    return;
  }
  remainingAtStartUs_ = remainingAt(nowUs);
  state_ = State::Paused;
}

void Engine::resume(std::uint64_t nowUs) {
  if (state_ != State::Paused) {
    return;
  }
  startedAtUs_ = nowUs;
  state_ = State::Running;
}

void Engine::stopReset() {
  state_ = State::Idle;
  remainingAtStartUs_ = durationFor(phase_);
}

void Engine::acknowledgeAlert() {
  if (state_ != State::Alert) {
    return;
  }
  phase_ = nextPhase_;
  state_ = State::Idle;
  remainingAtStartUs_ = durationFor(phase_);
}

void Engine::update(std::uint64_t nowUs) {
  if (state_ == State::Running && remainingAt(nowUs) == 0) {
    completePhase();
  }
}

void Engine::restore(const Snapshot& saved, std::uint64_t nowUs) {
  state_ = saved.state;
  phase_ = saved.phase;
  nextPhase_ = saved.nextPhase;
  completedFocusInCycle_ = saved.completedFocusInCycle;
  remainingAtStartUs_ = saved.remainingMs * 1000U;
  startedAtUs_ = nowUs;
}

Snapshot Engine::snapshot(std::uint64_t nowUs) const {
  return Snapshot{
      state_,
      phase_,
      nextPhase_,
      completedFocusInCycle_,
      remainingAt(nowUs) / 1000U,
      durationFor(phase_) / 1000U,
  };
}

std::uint64_t Engine::durationFor(Phase phase) const {
  std::uint32_t seconds = durations_.focusSeconds;
  if (phase == Phase::ShortBreak) {
    seconds = durations_.shortBreakSeconds;
  } else if (phase == Phase::LongBreak) {
    seconds = durations_.longBreakSeconds;
  }
  return static_cast<std::uint64_t>(seconds) * 1000000U;
}

std::uint64_t Engine::remainingAt(std::uint64_t nowUs) const {
  if (state_ != State::Running) {
    return remainingAtStartUs_;
  }
  const std::uint64_t elapsed = nowUs - startedAtUs_;
  return elapsed >= remainingAtStartUs_ ? 0 : remainingAtStartUs_ - elapsed;
}

void Engine::completePhase() {
  state_ = State::Alert;
  remainingAtStartUs_ = 0;

  if (phase_ == Phase::Focus) {
    completedFocusInCycle_++;
    nextPhase_ = completedFocusInCycle_ == 4 ? Phase::LongBreak : Phase::ShortBreak;
    return;
  }

  if (phase_ == Phase::LongBreak) {
    completedFocusInCycle_ = 0;
  }
  nextPhase_ = Phase::Focus;
}

}  // namespace pomodoro
