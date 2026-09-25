#include "app_ui.h"

#include <Arduino.h>

namespace ui {

namespace {
constexpr lv_color_t kBackground = LV_COLOR_MAKE(16, 20, 28);
constexpr lv_color_t kAccent = LV_COLOR_MAKE(255, 111, 97);
constexpr lv_color_t kMuted = LV_COLOR_MAKE(154, 164, 178);
}  // namespace

AppUi::AppUi(pomodoro::Engine& engine, services::ClockService& clock,
             services::WifiService& wifi, services::Settings& settings,
             services::SettingsStore& settingsStore)
    : engine_(engine),
      clock_(clock),
      wifi_(wifi),
      settings_(settings),
      settingsStore_(settingsStore) {}

void AppUi::begin() {
  lv_obj_t* screen = lv_scr_act();
  lv_obj_set_style_bg_color(screen, kBackground, 0);
  lv_obj_set_style_text_color(screen, lv_color_white(), 0);
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

  timerView_ = lv_obj_create(screen);
  lv_obj_set_size(timerView_, 480, 272);
  lv_obj_set_pos(timerView_, 0, 0);
  lv_obj_set_style_bg_opa(timerView_, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(timerView_, 0, 0);
  lv_obj_set_style_pad_all(timerView_, 0, 0);
  lv_obj_clear_flag(timerView_, LV_OBJ_FLAG_SCROLLABLE);

  clockLabel_ = lv_label_create(timerView_);
  lv_obj_set_style_text_font(clockLabel_, &lv_font_montserrat_28, 0);
  lv_obj_align(clockLabel_, LV_ALIGN_TOP_MID, 0, 8);

  dateLabel_ = lv_label_create(timerView_);
  lv_obj_set_style_text_color(dateLabel_, kMuted, 0);
  lv_obj_align(dateLabel_, LV_ALIGN_TOP_MID, 0, 42);

  statusLabel_ = lv_label_create(timerView_);
  lv_obj_set_style_text_color(statusLabel_, kMuted, 0);
  lv_obj_align(statusLabel_, LV_ALIGN_TOP_RIGHT, -10, 12);

  phaseLabel_ = lv_label_create(timerView_);
  lv_obj_set_style_text_color(phaseLabel_, kAccent, 0);
  lv_obj_set_style_text_font(phaseLabel_, &lv_font_montserrat_20, 0);
  lv_obj_align(phaseLabel_, LV_ALIGN_TOP_MID, 0, 70);

  timerLabel_ = lv_label_create(timerView_);
  lv_obj_set_style_text_font(timerLabel_, &lv_font_montserrat_48, 0);
  lv_obj_align(timerLabel_, LV_ALIGN_TOP_MID, 0, 96);

  progress_ = lv_bar_create(timerView_);
  lv_obj_set_size(progress_, 360, 12);
  lv_obj_align(progress_, LV_ALIGN_TOP_MID, 0, 155);
  lv_bar_set_range(progress_, 0, 1000);
  lv_obj_set_style_bg_color(progress_, kAccent, LV_PART_INDICATOR);

  countLabel_ = lv_label_create(timerView_);
  lv_obj_set_style_text_color(countLabel_, kMuted, 0);
  lv_obj_align(countLabel_, LV_ALIGN_TOP_MID, 0, 177);

  lv_obj_t* primary = makeButton(timerView_, "Start", 44, onPrimary, this);
  primaryLabel_ = lv_obj_get_child(primary, 0);
  makeButton(timerView_, "Reset", 184, onStop, this);
  lv_obj_t* format = makeButton(timerView_, settings_.use24Hour ? "24h" : "12h", 324,
                                onTimeFormat, this);
  formatLabel_ = lv_obj_get_child(format, 0);
  makeSmallButton(timerView_, LV_SYMBOL_WIFI, 8, 8, 48, onOpenWifi, this);

  wifiView_ = lv_obj_create(screen);
  lv_obj_set_size(wifiView_, 480, 272);
  lv_obj_set_pos(wifiView_, 0, 0);
  lv_obj_set_style_bg_color(wifiView_, kBackground, 0);
  lv_obj_set_style_border_width(wifiView_, 0, 0);
  lv_obj_set_style_pad_all(wifiView_, 0, 0);
  lv_obj_clear_flag(wifiView_, LV_OBJ_FLAG_SCROLLABLE);

  makeSmallButton(wifiView_, LV_SYMBOL_LEFT, 8, 6, 48, onCloseWifi, this);
  lv_obj_t* wifiTitle = lv_label_create(wifiView_);
  lv_label_set_text(wifiTitle, "Wi-Fi settings");
  lv_obj_set_style_text_font(wifiTitle, &lv_font_montserrat_20, 0);
  lv_obj_set_pos(wifiTitle, 70, 14);
  scanButton_ = makeSmallButton(wifiView_, "Scan", 394, 6, 78, onScan, this);

  wifiStatusLabel_ = lv_label_create(wifiView_);
  lv_obj_set_style_text_color(wifiStatusLabel_, kMuted, 0);
  lv_obj_set_pos(wifiStatusLabel_, 12, 54);
  lv_obj_set_size(wifiStatusLabel_, 350, 20);
  lv_label_set_long_mode(wifiStatusLabel_, LV_LABEL_LONG_DOT);

  forgetButton_ = makeSmallButton(wifiView_, "Forget", 394, 46, 78, onForget, this);
  networkList_ = lv_list_create(wifiView_);
  lv_obj_set_size(networkList_, 464, 176);
  lv_obj_set_pos(networkList_, 8, 92);
  lv_obj_set_style_bg_color(networkList_, LV_COLOR_MAKE(27, 33, 44), 0);
  lv_obj_set_style_border_width(networkList_, 0, 0);

  credentialView_ = lv_obj_create(screen);
  lv_obj_set_size(credentialView_, 480, 272);
  lv_obj_set_pos(credentialView_, 0, 0);
  lv_obj_set_style_bg_color(credentialView_, kBackground, 0);
  lv_obj_set_style_border_width(credentialView_, 0, 0);
  lv_obj_set_style_pad_all(credentialView_, 0, 0);
  lv_obj_clear_flag(credentialView_, LV_OBJ_FLAG_SCROLLABLE);

  makeSmallButton(credentialView_, LV_SYMBOL_LEFT, 8, 6, 48, onCredentialBack, this);
  credentialTitle_ = lv_label_create(credentialView_);
  lv_obj_set_style_text_font(credentialTitle_, &lv_font_montserrat_20, 0);
  lv_obj_set_pos(credentialTitle_, 70, 14);
  lv_obj_set_size(credentialTitle_, 305, 24);
  lv_label_set_long_mode(credentialTitle_, LV_LABEL_LONG_DOT);
  makeSmallButton(credentialView_, "Connect", 386, 6, 86, onConnect, this);

  credentialStatus_ = lv_label_create(credentialView_);
  lv_obj_set_style_text_color(credentialStatus_, kMuted, 0);
  lv_obj_set_pos(credentialStatus_, 12, 55);
  lv_obj_set_size(credentialStatus_, 456, 18);
  lv_label_set_long_mode(credentialStatus_, LV_LABEL_LONG_DOT);
  passwordField_ = lv_textarea_create(credentialView_);
  lv_obj_set_size(passwordField_, 456, 46);
  lv_obj_set_pos(passwordField_, 12, 76);
  lv_textarea_set_one_line(passwordField_, true);
  lv_textarea_set_password_mode(passwordField_, true);
  lv_textarea_set_max_length(passwordField_, 63);
  lv_textarea_set_placeholder_text(passwordField_, "Wi-Fi password");
  lv_obj_add_event_cb(passwordField_, onPasswordField, LV_EVENT_CLICKED, this);

  keyboard_ = lv_keyboard_create(credentialView_);
  lv_obj_set_size(keyboard_, 480, 142);
  lv_obj_set_pos(keyboard_, 0, 130);
  lv_obj_add_event_cb(keyboard_, onKeyboard, LV_EVENT_READY, this);
  lv_obj_add_event_cb(keyboard_, onKeyboard, LV_EVENT_CANCEL, this);
  lv_obj_add_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);

  lv_obj_add_flag(wifiView_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(credentialView_, LV_OBJ_FLAG_HIDDEN);
}

void AppUi::update(std::uint64_t nowUs) {
  nowUs_ = nowUs;
  if (millis() - lastRenderMs_ < 100) {
    return;
  }
  lastRenderMs_ = millis();

  if (!lv_obj_has_flag(wifiView_, LV_OBJ_FLAG_HIDDEN)) {
    updateWifiView();
  }

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

void AppUi::onOpenWifi(lv_event_t* event) {
  static_cast<AppUi*>(lv_event_get_user_data(event))->showWifi();
}

void AppUi::onCloseWifi(lv_event_t* event) {
  static_cast<AppUi*>(lv_event_get_user_data(event))->showTimer();
}

void AppUi::onScan(lv_event_t* event) {
  static_cast<AppUi*>(lv_event_get_user_data(event))->wifi_.scan();
}

void AppUi::onForget(lv_event_t* event) {
  AppUi* app = static_cast<AppUi*>(lv_event_get_user_data(event));
  app->wifi_.forget();
  app->wifi_.scan();
}

void AppUi::onNetwork(lv_event_t* event) {
  auto* choice = static_cast<NetworkChoice*>(lv_event_get_user_data(event));
  const services::WifiState state = choice->app->wifi_.state();
  if (choice->scanGeneration != choice->app->wifi_.scanGeneration() ||
      state == services::WifiState::Scanning || state == services::WifiState::Connecting) {
    return;
  }
  choice->app->showCredentials(choice->ssid, choice->secure);
}

void AppUi::onCredentialBack(lv_event_t* event) {
  AppUi* app = static_cast<AppUi*>(lv_event_get_user_data(event));
  lv_textarea_set_text(app->passwordField_, "");
  app->hideKeyboard();
  lv_obj_add_flag(app->credentialView_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(app->wifiView_, LV_OBJ_FLAG_HIDDEN);
}

void AppUi::onConnect(lv_event_t* event) {
  static_cast<AppUi*>(lv_event_get_user_data(event))->connectSelectedNetwork();
}

void AppUi::onPasswordField(lv_event_t* event) {
  static_cast<AppUi*>(lv_event_get_user_data(event))->showKeyboard();
}

void AppUi::onKeyboard(lv_event_t* event) {
  AppUi* app = static_cast<AppUi*>(lv_event_get_user_data(event));
  if (lv_event_get_code(event) == LV_EVENT_READY) {
    app->connectSelectedNetwork();
  } else {
    app->hideKeyboard();
  }
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

lv_obj_t* AppUi::makeSmallButton(lv_obj_t* parent, const char* text, lv_coord_t x,
                                 lv_coord_t y, lv_coord_t width,
                                 lv_event_cb_t callback, void* userData) {
  lv_obj_t* button = lv_btn_create(parent);
  lv_obj_set_size(button, width, 40);
  lv_obj_set_pos(button, x, y);
  lv_obj_set_style_bg_color(button, kAccent, 0);
  lv_obj_set_style_pad_all(button, 4, 0);
  lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, userData);
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

void AppUi::showWifi() {
  hideKeyboard();
  lv_obj_add_flag(timerView_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(credentialView_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(wifiView_, LV_OBJ_FLAG_HIDDEN);
  updateWifiView();
  if (wifi_.scanGeneration() == 0 && wifi_.state() != services::WifiState::Connecting) {
    wifi_.scan();
  }
}

void AppUi::showTimer() {
  hideKeyboard();
  lv_obj_add_flag(wifiView_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(credentialView_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(timerView_, LV_OBJ_FLAG_HIDDEN);
}

void AppUi::showCredentials(const char* ssid, bool secure) {
  snprintf(selectedSsid_, sizeof(selectedSsid_), "%s", ssid);
  if (!secure) {
    wifi_.connect(selectedSsid_, "");
    updateWifiView();
    return;
  }

  lv_label_set_text(credentialTitle_, selectedSsid_);
  lv_label_set_text(credentialStatus_, "Enter the network password");
  lv_textarea_set_text(passwordField_, "");
  lv_obj_add_flag(wifiView_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(credentialView_, LV_OBJ_FLAG_HIDDEN);
  showKeyboard();
}

void AppUi::showKeyboard() {
  lv_keyboard_set_textarea(keyboard_, passwordField_);
  lv_obj_clear_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(keyboard_);
  lv_obj_add_state(passwordField_, LV_STATE_FOCUSED);
}

void AppUi::hideKeyboard() {
  lv_keyboard_set_textarea(keyboard_, nullptr);
  lv_obj_add_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_state(passwordField_, LV_STATE_FOCUSED);
}

void AppUi::connectSelectedNetwork() {
  const char* password = lv_textarea_get_text(passwordField_);
  if (strlen(password) < 8) {
    lv_label_set_text(credentialStatus_, "Password must have at least 8 characters");
    return;
  }
  if (!wifi_.connect(selectedSsid_, password)) {
    lv_label_set_text(credentialStatus_, "Wi-Fi is busy. Try again shortly.");
    return;
  }
  lv_textarea_set_text(passwordField_, "");
  hideKeyboard();
  lv_obj_add_flag(credentialView_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(wifiView_, LV_OBJ_FLAG_HIDDEN);
  updateWifiView();
}

void AppUi::rebuildNetworkList() {
  lv_obj_clean(networkList_);
  const std::size_t count = wifi_.networkCount();
  if (count == 0) {
    lv_obj_t* label = lv_label_create(networkList_);
    lv_label_set_text(label, "No networks found. Tap Scan to retry.");
    lv_obj_set_style_text_color(label, kMuted, 0);
    return;
  }

  for (std::size_t index = 0; index < count; ++index) {
    const services::WifiNetwork& network = wifi_.network(index);
    char label[64];
    snprintf(label, sizeof(label), "%s  %ld dBm  %s", network.ssid,
             static_cast<long>(network.rssi), network.secure ? "secured" : "open");
    networkChoices_[index].app = this;
    networkChoices_[index].scanGeneration = wifi_.scanGeneration();
    snprintf(networkChoices_[index].ssid, sizeof(networkChoices_[index].ssid), "%s",
             network.ssid);
    networkChoices_[index].secure = network.secure;
    lv_obj_t* button = lv_list_add_btn(networkList_, LV_SYMBOL_WIFI, label);
    lv_obj_add_event_cb(button, onNetwork, LV_EVENT_CLICKED, &networkChoices_[index]);
  }
}

void AppUi::updateWifiView() {
  const services::WifiState state = wifi_.state();
  char status[96];
  if (state == services::WifiState::Scanning) {
    snprintf(status, sizeof(status), "Scanning for nearby networks...");
  } else if (state == services::WifiState::Connecting) {
    snprintf(status, sizeof(status), "Connecting to %s...", wifi_.connectedSsid());
  } else if (state == services::WifiState::Connected) {
    snprintf(status, sizeof(status), "Connected to %s", wifi_.connectedSsid());
  } else if (wifi_.error()[0] != '\0') {
    snprintf(status, sizeof(status), "%s", wifi_.error());
  } else if (state == services::WifiState::Disconnected) {
    snprintf(status, sizeof(status), "Offline. Reconnecting automatically.");
  } else {
    snprintf(status, sizeof(status), "No saved network. Select one below.");
  }
  lv_label_set_text(wifiStatusLabel_, status);

  const bool busy = state == services::WifiState::Scanning ||
                    state == services::WifiState::Connecting;
  if (busy) {
    lv_obj_add_state(scanButton_, LV_STATE_DISABLED);
  } else {
    lv_obj_clear_state(scanButton_, LV_STATE_DISABLED);
  }
  const std::uint32_t networkRows = lv_obj_get_child_cnt(networkList_);
  for (std::uint32_t index = 0; index < networkRows; ++index) {
    lv_obj_t* row = lv_obj_get_child(networkList_, index);
    if (busy) {
      lv_obj_add_state(row, LV_STATE_DISABLED);
    } else {
      lv_obj_clear_state(row, LV_STATE_DISABLED);
    }
  }
  if (wifi_.hasCredentials() || wifi_.connected()) {
    lv_obj_clear_flag(forgetButton_, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(forgetButton_, LV_OBJ_FLAG_HIDDEN);
  }

  if (renderedScanGeneration_ != wifi_.scanGeneration()) {
    renderedScanGeneration_ = wifi_.scanGeneration();
    rebuildNetworkList();
  }
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
