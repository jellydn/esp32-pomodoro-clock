#pragma once

#include <Arduino.h>

namespace services {

class ClockService {
 public:
  void begin();
  void update();
  bool timeValid() const;
  bool wifiConnected() const;
  void formatTime(char* destination, std::size_t length, bool use24Hour) const;
  void formatDate(char* destination, std::size_t length) const;

 private:
  void startWifi();

  std::uint32_t lastConnectAttemptMs_{0};
  bool ntpConfigured_{false};
};

}  // namespace services
