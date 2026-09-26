#pragma once

#include <lvgl.h>

#include "core/pomodoro_engine.h"
#include "services/clock_service.h"
#include "services/settings_store.h"
#include "services/wifi_service.h"

namespace ui {

class AppUi {
 public:
  AppUi(pomodoro::Engine& engine, services::ClockService& clock,
        services::WifiService& wifi, services::Settings& settings,
        services::SettingsStore& settingsStore);

  void begin();
  void update(std::uint64_t nowUs);

 private:
  static void onPrimary(lv_event_t* event);
  static void onStop(lv_event_t* event);
  static void onTimeFormat(lv_event_t* event);
  static void onOpenWifi(lv_event_t* event);
  static void onCloseWifi(lv_event_t* event);
  static void onScan(lv_event_t* event);
  static void onForget(lv_event_t* event);
  static void onNetwork(lv_event_t* event);
  static void onCredentialBack(lv_event_t* event);
  static void onConnect(lv_event_t* event);
  static void onPasswordField(lv_event_t* event);
  static void onKeyboard(lv_event_t* event);
  static lv_obj_t* makeButton(lv_obj_t* parent, const char* text, lv_coord_t x,
                              lv_event_cb_t callback, AppUi* self);
  static lv_obj_t* makeSmallButton(lv_obj_t* parent, const char* text, lv_coord_t x,
                                   lv_coord_t y, lv_coord_t width,
                                   lv_event_cb_t callback, void* userData);

  void handlePrimary();
  void handleStop();
  void handleTimeFormat();
  void showWifi();
  void showTimer();
  void showCredentials(const char* ssid, bool secure);
  void showKeyboard();
  void hideKeyboard();
  void connectSelectedNetwork();
  void rebuildNetworkList();
  void updateWifiView();
  const char* phaseName(pomodoro::Phase phase) const;

  struct NetworkChoice {
    AppUi* app{nullptr};
    std::uint32_t scanGeneration{0};
    char ssid[33]{};
    bool secure{false};
    bool supported{false};
  };

  pomodoro::Engine& engine_;
  services::ClockService& clock_;
  services::WifiService& wifi_;
  services::Settings& settings_;
  services::SettingsStore& settingsStore_;
  std::uint64_t nowUs_{0};
  std::uint32_t lastRenderMs_{0};
  std::uint32_t renderedScanGeneration_{0};
  char selectedSsid_[33]{};

  lv_obj_t* timerView_{nullptr};
  lv_obj_t* wifiView_{nullptr};
  lv_obj_t* credentialView_{nullptr};
  lv_obj_t* clockLabel_{nullptr};
  lv_obj_t* dateLabel_{nullptr};
  lv_obj_t* statusLabel_{nullptr};
  lv_obj_t* phaseLabel_{nullptr};
  lv_obj_t* timerLabel_{nullptr};
  lv_obj_t* progress_{nullptr};
  lv_obj_t* countLabel_{nullptr};
  lv_obj_t* primaryLabel_{nullptr};
  lv_obj_t* formatLabel_{nullptr};
  lv_obj_t* wifiStatusLabel_{nullptr};
  lv_obj_t* networkList_{nullptr};
  lv_obj_t* scanButton_{nullptr};
  lv_obj_t* forgetButton_{nullptr};
  lv_obj_t* credentialTitle_{nullptr};
  lv_obj_t* credentialStatus_{nullptr};
  lv_obj_t* passwordField_{nullptr};
  lv_obj_t* keyboard_{nullptr};
  NetworkChoice networkChoices_[services::WifiService::kMaxNetworks]{};
};

}  // namespace ui
