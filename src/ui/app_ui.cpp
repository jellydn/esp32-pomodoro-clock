#include "app_ui.h"

#include <Arduino.h>

namespace ui {

namespace {
constexpr lv_color_t kBackground = LV_COLOR_MAKE(16, 20, 28);
constexpr lv_color_t kAccent = LV_COLOR_MAKE(255, 111, 97);
constexpr lv_color_t kMuted = LV_COLOR_MAKE(154, 164, 178);
}  // namespace

AppUi::AppUi(pomodoro::Engine& engine, services::ClockService& clock,
             services::Settings& settings, services::SettingsStore& settingsStore)
    : engine_(engine), clock_(clock), settings_(settings), settingsStore_(settingsStore) {}

void AppUi::begin() {
  lv_obj_t* screen = lv_scr_act();
  lv_obj_set_style_bg_color(screen, kBackground, 0);
  lv_obj_set_style_text_color(screen, lv_color_white(), 0);

  clockLabel_ = lv_label_create(screen);
  lv_obj_set_style_text_font(clockLabel_, &lv_font_montserrat_28, 0);
  lv_obj_align(clockLabel_, LV_ALIGN_TOP_MID, 0, 8);

  dateLabel_ = lv_label_create(screen);
  lv_obj_set_style_text_color(dateLabel_, kMuted, 0);
  lv_obj_align(dateLabel_, LV_ALIGN_TOP_MID, 0, 42);

  statusLabel_ = lv_label_create(screen);
  lv_obj_set_style_text_color(statusLabel_, kMuted, 0);
  lv_obj_align(statusLabel_, LV_ALIGN_TOP_RIGHT, -10, 12);

  phaseLabel_ = lv_label_create(screen);
  lv_obj_set_style_text_color(phaseLabel_, kAccent, 0);
  lv_obj_set_style_text_font(phaseLabel_, &lv_font_montserrat_20, 0);
  lv_obj_align(phaseLabel_, LV_ALIGN_TOP_MID, 0, 70);

  timerLabel_ = lv_label_create(screen);
  lv_obj_set_style_text_font(timerLabel_, &lv_font_montserrat_48, 0);
  lv_obj_align(timerLabel_, LV_ALIGN_TOP_MID, 0, 96);

  progress_ = lv_bar_create(screen);
  lv_obj_set_size(progress_, 360, 12);
  lv_obj_align(progress_, LV_ALIGN_TOP_MID, 0, 155);
  lv_bar_set_range(progress_, 0, 1000);
  lv_obj_set_style_bg_color(progress_, kAccent, LV_PART_INDICATOR);

  countLabel_ = lv_label_create(screen);
  lv_obj_set_style_text_color(countLabel_, kMuted, 0);
  lv_obj_align(countLabel_, LV_ALIGN_TOP_MID, 0, 177);

  lv_obj_t* primary = makeButton(screen, "Start", 44, onPrimary, this);
  primaryLabel_ = lv_obj_get_child(primary, 0);
  makeButton(screen, "Reset", 184, onStop, this);
  lv_obj_t* format = makeButton(screen, settings_.use24Hour ? "24h" : "12h", 324,
                                onTimeFormat, this);
  formatLabel_ = lv_obj_get_child(format, 0);
}

void AppUi::update(std::uint64_t nowUs) {
  nowUs_ = nowUs;
  if (millis() - lastRenderMs_ < 100) {
    return;
  }
  lastRenderMs_ = millis();

  char timeText[16] = "--:--:--";
  char dateText[24] = "Waiting for NTP";
  if (clock_.timeValid()) {
    clock_.formatTime(timeText, sizeof(timeText), settings_.use24Hour);
    clock_.formatDate(dateText, sizeof(dateText));
  }
  lv_label_set_text(clockLabel_, timeText);
  lv_label_set_text(dateLabel_, dateText);
  lv_label_set_text(statusLabel_, clock_.wifiConnected() ? LV_SYMBOL_WIFI : "offline");

  const pomodoro::Snapshot snapshot = engine_.snapshot(nowUs);
  lv_label_set_text(phaseLabel_, snapshot.state == pomodoro::State::Alert
                                     ? "Session complete"
                                     : phaseName(snapshot.phase));

  const std::uint64_t seconds = (snapshot.remainingMs + 999U) / 1000U;
  char timerText[12];
  snprintf(timerText, sizeof(timerText), "%02llu:%02llu",
           static_cast<unsigned long long>(seconds / 60U),
           static_cast<unsigned long long>(seconds % 60U));
  lv_label_set_text(timerLabel_, timerText);

  const std::uint32_t progress = snapshot.durationMs == 0
                                     ? 0
                                     : static_cast<std::uint32_t>(
                                           (snapshot.durationMs - snapshot.remainingMs) * 1000U /
                                           snapshot.durationMs);
  lv_bar_set_value(progress_, progress, LV_ANIM_OFF);

  char countText[40];
  snprintf(countText, sizeof(countText), "Focus sessions: %u / 4",
           snapshot.completedFocusInCycle);
  lv_label_set_text(countLabel_, countText);

  const char* primaryText = "Start";
  if (snapshot.state == pomodoro::State::Running) {
    primaryText = "Pause";
  } else if (snapshot.state == pomodoro::State::Paused) {
    primaryText = "Resume";
  } else if (snapshot.state == pomodoro::State::Alert) {
    primaryText = "Next";
  }
  lv_label_set_text(primaryLabel_, primaryText);
}

void AppUi::onPrimary(lv_event_t* event) {
  static_cast<AppUi*>(lv_event_get_user_data(event))->handlePrimary();
}

void AppUi::onStop(lv_event_t* event) {
  static_cast<AppUi*>(lv_event_get_user_data(event))->handleStop();
}

void AppUi::onTimeFormat(lv_event_t* event) {
  static_cast<AppUi*>(lv_event_get_user_data(event))->handleTimeFormat();
}

lv_obj_t* AppUi::makeButton(lv_obj_t* parent, const char* text, lv_coord_t x,
                            lv_event_cb_t callback, AppUi* self) {
  lv_obj_t* button = lv_btn_create(parent);
  lv_obj_set_size(button, 112, 48);
  lv_obj_set_pos(button, x, 214);
  lv_obj_set_style_bg_color(button, kAccent, 0);
  lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, self);
  lv_obj_t* label = lv_label_create(button);
  lv_label_set_text(label, text);
  lv_obj_center(label);
  return button;
}

void AppUi::handlePrimary() {
  const pomodoro::State state = engine_.snapshot(nowUs_).state;
  if (state == pomodoro::State::Running) {
    engine_.pause(nowUs_);
  } else if (state == pomodoro::State::Paused) {
    engine_.resume(nowUs_);
  } else {
    if (state == pomodoro::State::Alert) {
      engine_.acknowledgeAlert();
    }
    engine_.start(nowUs_);
  }
}

void AppUi::handleStop() { engine_.stopReset(); }

void AppUi::handleTimeFormat() {
  settings_.use24Hour = !settings_.use24Hour;
  settingsStore_.save(settings_);
  lv_label_set_text(formatLabel_, settings_.use24Hour ? "24h" : "12h");
}

const char* AppUi::phaseName(pomodoro::Phase phase) const {
  if (phase == pomodoro::Phase::ShortBreak) {
    return "Short break";
  }
  if (phase == pomodoro::Phase::LongBreak) {
    return "Long break";
  }
  return "Focus";
}

}  // namespace ui
