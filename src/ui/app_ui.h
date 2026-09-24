#pragma once

#include <lvgl.h>

#include "core/pomodoro_engine.h"
#include "services/clock_service.h"
#include "services/settings_store.h"

namespace ui {

class AppUi {
 public:
  AppUi(pomodoro::Engine& engine, services::ClockService& clock,
        services::Settings& settings, services::SettingsStore& settingsStore);

  void begin();
  void update(std::uint64_t nowUs);

 private:
  static void onPrimary(lv_event_t* event);
  static void onStop(lv_event_t* event);
  static void onTimeFormat(lv_event_t* event);
  static lv_obj_t* makeButton(lv_obj_t* parent, const char* text, lv_coord_t x,
                              lv_event_cb_t callback, AppUi* self);

  void handlePrimary();
  void handleStop();
  void handleTimeFormat();
  const char* phaseName(pomodoro::Phase phase) const;

  pomodoro::Engine& engine_;
  services::ClockService& clock_;
  services::Settings& settings_;
  services::SettingsStore& settingsStore_;
  std::uint64_t nowUs_{0};
  std::uint32_t lastRenderMs_{0};

  lv_obj_t* clockLabel_{nullptr};
  lv_obj_t* dateLabel_{nullptr};
  lv_obj_t* statusLabel_{nullptr};
  lv_obj_t* phaseLabel_{nullptr};
  lv_obj_t* timerLabel_{nullptr};
  lv_obj_t* progress_{nullptr};
  lv_obj_t* countLabel_{nullptr};
  lv_obj_t* primaryLabel_{nullptr};
  lv_obj_t* formatLabel_{nullptr};
};

}  // namespace ui
