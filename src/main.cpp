#include <Arduino.h>
#include <esp_timer.h>
#include <lvgl.h>

#include "board/display_driver.h"
#include "board/touch_driver.h"
#include "core/pomodoro_engine.h"
#include "services/clock_service.h"
#include "services/session_store.h"
#include "services/settings_store.h"
#include "ui/app_ui.h"

namespace {

board::DisplayDriver display;
board::TouchDriver touch;
pomodoro::Engine timer;
services::ClockService clockService;
services::SettingsStore settingsStore;
services::SessionStore sessionStore;
services::Settings settings;
ui::AppUi appUi(timer, clockService, settings, settingsStore);

std::uint32_t lastActivityMs = 0;
bool dimmed = false;
bool savedWithWallTime = false;
pomodoro::Snapshot lastSavedSession = timer.snapshot(0);

void printHardwareReport() {
  Serial.printf("Chip: %s, revision %u, cores %u\n", ESP.getChipModel(), ESP.getChipRevision(),
                ESP.getChipCores());
  Serial.printf("Flash: %u bytes, PSRAM: %u bytes, free heap: %u bytes\n",
                ESP.getFlashChipSize(), ESP.getPsramSize(), ESP.getFreeHeap());
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nJC4827W543 clock + Pomodoro");
  printHardwareReport();

  settings = settingsStore.load();
  lv_init();

  if (!display.begin()) {
    Serial.println("ERROR: NV3041A display initialization failed");
  }
  display.showBootScreen();
  delay(800);
  display.registerLvgl();
  display.setBrightness(settings.brightness);

  if (touch.begin()) {
    Serial.printf("Touch found at 0x%02X, product ID: %s\n", touch.address(), touch.productId());
    touch.registerLvgl();
  } else {
    Serial.println("WARNING: no GT911 response at 0x5D or 0x14");
  }

  clockService.begin();
  sessionStore.restore(timer, esp_timer_get_time(), clockService.timeValid());
  lastSavedSession = timer.snapshot(esp_timer_get_time());
  savedWithWallTime = lastSavedSession.state != pomodoro::State::Running ||
                      clockService.timeValid();
  appUi.begin();
  lastActivityMs = millis();
}

void loop() {
  const std::uint64_t nowUs = esp_timer_get_time();
  clockService.update();
  timer.update(nowUs);
  const pomodoro::Snapshot session = timer.snapshot(nowUs);
  const bool wallTimeValid = clockService.timeValid();
  const bool stateChanged = session.state != lastSavedSession.state ||
                            session.phase != lastSavedSession.phase ||
                            session.nextPhase != lastSavedSession.nextPhase ||
                            session.completedFocusInCycle !=
                                lastSavedSession.completedFocusInCycle;
  const bool runningNeedsDeadline = session.state == pomodoro::State::Running &&
                                    wallTimeValid && !savedWithWallTime;
  if (stateChanged || runningNeedsDeadline) {
    sessionStore.save(session, wallTimeValid);
    lastSavedSession = session;
    savedWithWallTime = session.state != pomodoro::State::Running || wallTimeValid;
  }
  appUi.update(nowUs);
  lv_timer_handler();

  if (touch.touchedSinceLastRead()) {
    lastActivityMs = millis();
    if (dimmed) {
      display.setBrightness(settings.brightness);
      dimmed = false;
    }
  } else if (!dimmed && millis() - lastActivityMs >= settings.dimAfterSeconds * 1000UL) {
    display.setBrightness(settings.dimBrightness);
    dimmed = true;
  }

  delay(5);
}
