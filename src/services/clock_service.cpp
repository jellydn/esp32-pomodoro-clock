#include "clock_service.h"

#include <time.h>

namespace services {

namespace {
constexpr char kSingaporeTimezone[] = "SGT-8";
constexpr time_t kMinimumValidEpoch = 1700000000;
}  // namespace

ClockService::ClockService(WifiService& wifi) : wifi_(wifi) {}

void ClockService::update() {
  if (wifiConnected() && !ntpConfigured_) {
    configTzTime(kSingaporeTimezone, "pool.ntp.org", "time.cloudflare.com",
                 "time.google.com");
    ntpConfigured_ = true;
    Serial.println("NTP configured for Asia/Singapore");
  }
}

bool ClockService::timeValid() const { return time(nullptr) >= kMinimumValidEpoch; }

bool ClockService::wifiConnected() const { return wifi_.connected(); }

void ClockService::formatTime(char* destination, std::size_t length, bool use24Hour) const {
  const time_t now = time(nullptr);
  struct tm local {};
  localtime_r(&now, &local);
  strftime(destination, length, use24Hour ? "%H:%M:%S" : "%I:%M:%S %p", &local);
}

void ClockService::formatDate(char* destination, std::size_t length) const {
  const time_t now = time(nullptr);
  struct tm local {};
  localtime_r(&now, &local);
  strftime(destination, length, "%a, %d %b %Y", &local);
}

}  // namespace services
