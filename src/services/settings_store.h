#pragma once

#include <Preferences.h>

namespace services {

struct Settings {
  bool use24Hour{true};
  std::uint8_t brightness{100};
  std::uint8_t dimBrightness{15};
  std::uint16_t dimAfterSeconds{60};
};

class SettingsStore {
 public:
  Settings load();
  void save(const Settings& settings);
};

}  // namespace services
