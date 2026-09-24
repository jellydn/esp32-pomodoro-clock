#include "session_store.h"

#include <Preferences.h>
#include <time.h>

namespace services {

namespace {
constexpr std::uint32_t kMagic = 0x504F4D4FU;
constexpr std::uint16_t kVersion = 1;

struct Record {
  std::uint32_t magic;
  std::uint16_t version;
  std::uint8_t state;
  std::uint8_t phase;
  std::uint8_t nextPhase;
  std::uint8_t completedFocusInCycle;
  std::uint64_t remainingMs;
  std::int64_t wallDeadlineEpoch;
};

bool valid(const Record& record) {
  return record.magic == kMagic && record.version == kVersion && record.state <= 3 &&
         record.phase <= 2 && record.nextPhase <= 2 && record.completedFocusInCycle <= 4;
}
}  // namespace

bool SessionStore::restore(pomodoro::Engine& engine, std::uint64_t nowUs,
                           bool wallTimeValid) {
  Preferences preferences;
  preferences.begin("session", true);
  Record record{};
  const std::size_t size = preferences.getBytesLength("record");
  const bool read = size == sizeof(record) &&
                    preferences.getBytes("record", &record, sizeof(record)) == sizeof(record);
  preferences.end();
  if (!read || !valid(record)) {
    return false;
  }

  auto state = static_cast<pomodoro::State>(record.state);
  std::uint64_t remainingMs = record.remainingMs;
  if (state == pomodoro::State::Running) {
    if (wallTimeValid && record.wallDeadlineEpoch > 0) {
      const std::int64_t remainingSeconds = record.wallDeadlineEpoch - time(nullptr);
      remainingMs = remainingSeconds > 0 ? remainingSeconds * 1000U : 0;
    } else {
      state = pomodoro::State::Paused;
    }
  }

  engine.restore({state, static_cast<pomodoro::Phase>(record.phase),
                  static_cast<pomodoro::Phase>(record.nextPhase),
                  record.completedFocusInCycle, remainingMs, 0},
                 nowUs);
  engine.update(nowUs);
  return true;
}

void SessionStore::save(const pomodoro::Snapshot& snapshot, bool wallTimeValid) {
  Record record{
      kMagic,
      kVersion,
      static_cast<std::uint8_t>(snapshot.state),
      static_cast<std::uint8_t>(snapshot.phase),
      static_cast<std::uint8_t>(snapshot.nextPhase),
      snapshot.completedFocusInCycle,
      snapshot.remainingMs,
      0,
  };
  if (snapshot.state == pomodoro::State::Running && wallTimeValid) {
    record.wallDeadlineEpoch = time(nullptr) + (snapshot.remainingMs + 999U) / 1000U;
  }

  Preferences preferences;
  preferences.begin("session", false);
  preferences.putBytes("record", &record, sizeof(record));
  preferences.end();
}

}  // namespace services
