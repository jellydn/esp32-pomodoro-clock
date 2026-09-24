#include "clock_service.h"

#include <WiFi.h>
#include <time.h>

#if __has_include("wifi_config.h")
#include "wifi_config.h"
#else
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#endif

namespace services {

namespace {
constexpr char kSingaporeTimezone[] = "SGT-8";
constexpr std::uint32_t kReconnectIntervalMs = 30000;
constexpr time_t kMinimumValidEpoch = 1700000000;
}  // namespace

void ClockService::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
      Serial.printf("Wi-Fi connected: %s, RSSI %d dBm\n", WiFi.localIP().toString().c_str(),
                    WiFi.RSSI());
    } else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
      Serial.printf("Wi-Fi disconnected, reason %u\n", info.wifi_sta_disconnected.reason);
    }
  });
  startWifi();
}

void ClockService::update() {
  if (wifiConnected() && !ntpConfigured_) {
    configTzTime(kSingaporeTimezone, "pool.ntp.org", "time.cloudflare.com",
                 "time.google.com");
    ntpConfigured_ = true;
    Serial.println("NTP configured for Asia/Singapore");
  }

  if (!wifiConnected() && millis() - lastConnectAttemptMs_ >= kReconnectIntervalMs) {
    startWifi();
  }
}

bool ClockService::timeValid() const { return time(nullptr) >= kMinimumValidEpoch; }

bool ClockService::wifiConnected() const { return WiFi.status() == WL_CONNECTED; }

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

void ClockService::startWifi() {
  lastConnectAttemptMs_ = millis();
  if (strlen(WIFI_SSID) == 0) {
    Serial.println("Wi-Fi not configured; copy include/wifi_config.example.h");
    return;
  }
  Serial.printf("Connecting to Wi-Fi SSID: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

}  // namespace services
