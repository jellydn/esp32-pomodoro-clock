#pragma once

#include <Arduino.h>

#include "wifi_service.h"

namespace services {

class ClockService {
 public:
  explicit ClockService(WifiService& wifi);

  void update();
  bool timeValid() const;
  bool wifiConnected() const;
  void formatTime(char* destination, std::size_t length, bool use24Hour) const;
  void formatDate(char* destination, std::size_t length) const;

 private:
  WifiService& wifi_;
  bool ntpConfigured_{false};
};

}  // namespace services
