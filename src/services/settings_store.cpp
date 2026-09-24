#include "settings_store.h"

namespace services {

Settings SettingsStore::load() {
  Preferences preferences;
  preferences.begin("settings", true);
  Settings settings;
  settings.use24Hour = preferences.getBool("24hour", settings.use24Hour);
  settings.brightness = preferences.getUChar("bright", settings.brightness);
  settings.dimBrightness = preferences.getUChar("dim", settings.dimBrightness);
  settings.dimAfterSeconds = preferences.getUShort("dimAfter", settings.dimAfterSeconds);
  preferences.end();
  return settings;
}

void SettingsStore::save(const Settings& settings) {
  Preferences preferences;
  preferences.begin("settings", false);
  preferences.putBool("24hour", settings.use24Hour);
  preferences.putUChar("bright", settings.brightness);
  preferences.putUChar("dim", settings.dimBrightness);
  preferences.putUShort("dimAfter", settings.dimAfterSeconds);
  preferences.end();
}

}  // namespace services
