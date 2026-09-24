#include <unity.h>

#include "core/pomodoro_engine.h"

using pomodoro::Durations;
using pomodoro::Engine;
using pomodoro::Phase;
using pomodoro::State;

namespace {

constexpr std::uint64_t kSecond = 1000000U;

void test_completes_at_exact_deadline_once() {
  Engine engine(Durations{2, 1, 3});
  engine.start(10 * kSecond);

  engine.update(12 * kSecond);
  auto result = engine.snapshot(12 * kSecond);

  TEST_ASSERT_EQUAL(static_cast<int>(State::Alert), static_cast<int>(result.state));
  TEST_ASSERT_EQUAL_UINT8(1, result.completedFocusInCycle);
  TEST_ASSERT_EQUAL(static_cast<int>(Phase::ShortBreak), static_cast<int>(result.nextPhase));

  engine.update(50 * kSecond);
  result = engine.snapshot(50 * kSecond);
  TEST_ASSERT_EQUAL_UINT8(1, result.completedFocusInCycle);
}

void test_pause_and_resume_uses_elapsed_monotonic_time() {
  Engine engine(Durations{10, 2, 4});
  engine.start(100 * kSecond);
  engine.pause(103 * kSecond);

  auto paused = engine.snapshot(1000 * kSecond);
  TEST_ASSERT_EQUAL_UINT64(7000, paused.remainingMs);

  engine.resume(2000 * kSecond);
  engine.update(2007 * kSecond - 1);
  TEST_ASSERT_EQUAL(static_cast<int>(State::Running),
                    static_cast<int>(engine.snapshot(2007 * kSecond - 1).state));
  engine.update(2007 * kSecond);
  TEST_ASSERT_EQUAL(static_cast<int>(State::Alert),
                    static_cast<int>(engine.snapshot(2007 * kSecond).state));
}

void completeCurrentPhase(Engine& engine, std::uint64_t& nowUs) {
  engine.start(nowUs);
  nowUs += engine.snapshot(nowUs).durationMs * 1000U;
  engine.update(nowUs);
  engine.acknowledgeAlert();
}

void test_fourth_focus_selects_long_break_and_long_break_resets_cycle() {
  Engine engine(Durations{1, 1, 2});
  std::uint64_t nowUs = 0;

  for (int completed = 1; completed <= 4; ++completed) {
    completeCurrentPhase(engine, nowUs);
    auto breakPhase = engine.snapshot(nowUs);
    TEST_ASSERT_EQUAL_UINT8(completed, breakPhase.completedFocusInCycle);
    TEST_ASSERT_EQUAL(static_cast<int>(completed == 4 ? Phase::LongBreak : Phase::ShortBreak),
                      static_cast<int>(breakPhase.phase));
    completeCurrentPhase(engine, nowUs);
  }

  auto result = engine.snapshot(nowUs);
  TEST_ASSERT_EQUAL(static_cast<int>(Phase::Focus), static_cast<int>(result.phase));
  TEST_ASSERT_EQUAL_UINT8(0, result.completedFocusInCycle);
}

void test_stop_resets_current_phase_but_preserves_completed_count() {
  Engine engine(Durations{1, 1, 2});
  std::uint64_t nowUs = 0;
  completeCurrentPhase(engine, nowUs);
  completeCurrentPhase(engine, nowUs);

  engine.start(nowUs);
  nowUs += 400000;
  engine.stopReset();
  auto result = engine.snapshot(nowUs);

  TEST_ASSERT_EQUAL(static_cast<int>(State::Idle), static_cast<int>(result.state));
  TEST_ASSERT_EQUAL(static_cast<int>(Phase::Focus), static_cast<int>(result.phase));
  TEST_ASSERT_EQUAL_UINT8(1, result.completedFocusInCycle);
  TEST_ASSERT_EQUAL_UINT64(1000, result.remainingMs);
}

void test_restore_running_uses_a_new_monotonic_anchor() {
  Engine engine(Durations{10, 2, 4});
  engine.restore({State::Running, Phase::Focus, Phase::ShortBreak, 2, 3500, 10000},
                 500 * kSecond);

  engine.update(503 * kSecond);
  auto running = engine.snapshot(503 * kSecond);
  TEST_ASSERT_EQUAL(static_cast<int>(State::Running), static_cast<int>(running.state));
  TEST_ASSERT_EQUAL_UINT64(500, running.remainingMs);

  engine.update(503 * kSecond + 500000);
  auto completed = engine.snapshot(503 * kSecond + 500000);
  TEST_ASSERT_EQUAL(static_cast<int>(State::Alert), static_cast<int>(completed.state));
  TEST_ASSERT_EQUAL_UINT8(3, completed.completedFocusInCycle);
}

}  // namespace

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_completes_at_exact_deadline_once);
  RUN_TEST(test_pause_and_resume_uses_elapsed_monotonic_time);
  RUN_TEST(test_fourth_focus_selects_long_break_and_long_break_resets_cycle);
  RUN_TEST(test_stop_resets_current_phase_but_preserves_completed_count);
  RUN_TEST(test_restore_running_uses_a_new_monotonic_anchor);
  return UNITY_END();
}
