#include "wifi_service.h"

#include <Preferences.h>
#include <WiFi.h>

#if __has_include("wifi_config.h")
#include "wifi_config.h"
#else
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#endif

namespace services {

namespace {
constexpr std::uint32_t kConnectTimeoutMs = 15000;
constexpr std::uint32_t kReconnectIntervalMs = 30000;

void copyText(char* destination, std::size_t size, const char* source) {
  snprintf(destination, size, "%s", source == nullptr ? "" : source);
}
}  // namespace

void WifiService::begin() {
  resetWifiStorage();
  WiFi.setAutoReconnect(false);
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
      ++gotIpGeneration_;
    } else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
      disconnectReason_ = info.wifi_sta_disconnected.reason;
      ++disconnectGeneration_;
    }
  });
  loadCredentials();
  if (hasCredentials()) {
    startConnection(savedSsid_, savedPassword_, false);
  }
}

void WifiService::update() {
  if (connectionStage_ == ConnectionStage::WaitingForCleanup) {
    if (disconnectGeneration_ != attemptDisconnectGeneration_ ||
        millis() - attemptStartedMs_ >= 1000) {
      connectionStage_ = ConnectionStage::Idle;
    } else {
      return;
    }
  }

  if (state_ == WifiState::Scanning) {
    const std::int16_t result = WiFi.scanComplete();
    if (result >= 0) {
      finishScan(result);
    } else if (result == WIFI_SCAN_FAILED) {
      WiFi.scanDelete();
      setError("Scan failed. Try again.");
      state_ = connected() ? WifiState::Connected
                           : (hasCredentials() ? WifiState::Disconnected
                                               : WifiState::Failed);
    }
    return;
  }

  if (state_ == WifiState::Connecting) {
    if (connectionStage_ == ConnectionStage::WaitingForDisconnect) {
      if (disconnectGeneration_ == attemptDisconnectGeneration_) {
        if (millis() - attemptStartedMs_ >= kConnectTimeoutMs) {
          failConnection("Could not disconnect from current network.", true);
        }
        return;
      }
      beginPendingConnection();
      return;
    }

    if (gotIpGeneration_ != attemptGotIpGeneration_ && connected() &&
        WiFi.SSID().equals(pendingSsid_)) {
      state_ = WifiState::Connected;
      connectionStage_ = ConnectionStage::Idle;
      error_[0] = '\0';
      if (saveOnSuccess_) {
        savePendingCredentials();
      }
      Serial.printf("Wi-Fi connected: %s, RSSI %d dBm\n",
                    WiFi.localIP().toString().c_str(), WiFi.RSSI());
      return;
    }

    if (disconnectGeneration_ != attemptDisconnectGeneration_) {
      const std::uint8_t reason = disconnectReason_;
      if (reason == WIFI_REASON_NO_AP_FOUND) {
        failConnection("Network is no longer available.", true);
      } else if (reason == WIFI_REASON_AUTH_FAIL || reason == WIFI_REASON_AUTH_EXPIRE ||
                 reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT ||
                 reason == WIFI_REASON_HANDSHAKE_TIMEOUT) {
        failConnection("Could not authenticate.", true);
      } else {
        failConnection("Could not connect.", true);
      }
      return;
    }

    if (millis() - attemptStartedMs_ >= kConnectTimeoutMs) {
      failConnection("Connection timed out.", true);
    }
    return;
  }

  if (state_ == WifiState::Connected && !connected()) {
    state_ = hasCredentials() ? WifiState::Disconnected : WifiState::NotConfigured;
    lastReconnectMs_ = millis();
  }

  if (connectionStage_ == ConnectionStage::Idle && state_ == WifiState::Disconnected &&
      hasCredentials() &&
      millis() - lastReconnectMs_ >= kReconnectIntervalMs) {
    startConnection(savedSsid_, savedPassword_, false);
  }
}

void WifiService::scan() {
  if (connectionStage_ != ConnectionStage::Idle || state_ == WifiState::Scanning ||
      state_ == WifiState::Connecting) {
    return;
  }
  error_[0] = '\0';
  WiFi.scanDelete();
  const std::int16_t result = WiFi.scanNetworks(true, false, false, 300);
  if (result == WIFI_SCAN_FAILED) {
    setError("Could not start scan.");
    state_ = connected() ? WifiState::Connected
                         : (hasCredentials() ? WifiState::Disconnected
                                             : WifiState::Failed);
    return;
  }
  state_ = WifiState::Scanning;
}

bool WifiService::connect(const char* ssid, const char* password) {
  if (connectionStage_ != ConnectionStage::Idle || state_ == WifiState::Scanning ||
      state_ == WifiState::Connecting) {
    return false;
  }
  if (ssid == nullptr || ssid[0] == '\0') {
    return false;
  }
  startConnection(ssid, password, true);
  return true;
}

void WifiService::forget() {
  Preferences preferences;
  preferences.begin("wifi", false);
  preferences.remove("ssid");
  preferences.remove("password");
  preferences.putBool("disabled", true);
  preferences.end();

  memset(savedSsid_, 0, sizeof(savedSsid_));
  memset(savedPassword_, 0, sizeof(savedPassword_));
  memset(pendingSsid_, 0, sizeof(pendingSsid_));
  memset(pendingPassword_, 0, sizeof(pendingPassword_));
  connectionStage_ = ConnectionStage::Idle;
  saveOnSuccess_ = false;
  error_[0] = '\0';
  resetWifiStorage();
  state_ = WifiState::NotConfigured;
}

WifiState WifiService::state() const { return state_; }

bool WifiService::connected() const { return WiFi.status() == WL_CONNECTED; }

bool WifiService::hasCredentials() const { return savedSsid_[0] != '\0'; }

const char* WifiService::connectedSsid() const {
  return state_ == WifiState::Connecting ? pendingSsid_ : savedSsid_;
}

const char* WifiService::error() const { return error_; }

std::size_t WifiService::networkCount() const { return networkCount_; }

const WifiNetwork& WifiService::network(std::size_t index) const { return networks_[index]; }

std::uint32_t WifiService::scanGeneration() const { return scanGeneration_; }

void WifiService::startConnection(const char* ssid, const char* password,
                                  bool saveOnSuccess) {
  if (ssid == nullptr || ssid[0] == '\0') {
    setError("Select a network first.");
    state_ = WifiState::Failed;
    return;
  }

  copyText(pendingSsid_, sizeof(pendingSsid_), ssid);
  copyText(pendingPassword_, sizeof(pendingPassword_), password);
  saveOnSuccess_ = saveOnSuccess;
  error_[0] = '\0';
  state_ = WifiState::Connecting;
  if (connected()) {
    connectionStage_ = ConnectionStage::WaitingForDisconnect;
    attemptDisconnectGeneration_ = disconnectGeneration_;
    attemptStartedMs_ = millis();
    WiFi.disconnect(false, false);
    return;
  }
  beginPendingConnection();
}

void WifiService::beginPendingConnection() {
  connectionStage_ = ConnectionStage::WaitingForIp;
  attemptDisconnectGeneration_ = disconnectGeneration_;
  attemptGotIpGeneration_ = gotIpGeneration_;
  WiFi.begin(pendingSsid_, pendingPassword_);
  attemptStartedMs_ = millis();
  Serial.printf("Connecting to Wi-Fi SSID: %s\n", pendingSsid_);
}

void WifiService::failConnection(const char* message, bool disconnectFirst) {
  setError(message);
  if (disconnectFirst) {
    connectionStage_ = ConnectionStage::WaitingForCleanup;
    attemptDisconnectGeneration_ = disconnectGeneration_;
    attemptStartedMs_ = millis();
    WiFi.disconnect(false, false);
  } else {
    connectionStage_ = ConnectionStage::Idle;
  }
  state_ = hasCredentials() ? WifiState::Disconnected : WifiState::Failed;
  lastReconnectMs_ = millis();
  memset(pendingPassword_, 0, sizeof(pendingPassword_));
  saveOnSuccess_ = false;
}

void WifiService::resetWifiStorage() {
  WiFi.mode(WIFI_OFF);
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  attemptDisconnectGeneration_ = disconnectGeneration_;
  attemptGotIpGeneration_ = gotIpGeneration_;
}

void WifiService::loadCredentials() {
  Preferences preferences;
  preferences.begin("wifi", true);
  const bool disabled = preferences.getBool("disabled", false);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  preferences.end();

  if (!disabled && ssid.isEmpty() && strlen(WIFI_SSID) > 0) {
    ssid = WIFI_SSID;
    password = WIFI_PASSWORD;
  }
  copyText(savedSsid_, sizeof(savedSsid_), ssid.c_str());
  copyText(savedPassword_, sizeof(savedPassword_), password.c_str());
  state_ = hasCredentials() ? WifiState::Disconnected : WifiState::NotConfigured;
}

void WifiService::savePendingCredentials() {
  copyText(savedSsid_, sizeof(savedSsid_), pendingSsid_);
  copyText(savedPassword_, sizeof(savedPassword_), pendingPassword_);
  Preferences preferences;
  preferences.begin("wifi", false);
  preferences.putString("ssid", savedSsid_);
  preferences.putString("password", savedPassword_);
  preferences.putBool("disabled", false);
  preferences.end();
  memset(pendingPassword_, 0, sizeof(pendingPassword_));
  saveOnSuccess_ = false;
}

void WifiService::finishScan(std::int16_t count) {
  networkCount_ = 0;
  for (std::int16_t index = 0;
       index < count && networkCount_ < kMaxNetworks; ++index) {
    const String ssid = WiFi.SSID(index);
    if (ssid.isEmpty()) {
      continue;
    }

    bool duplicate = false;
    for (std::size_t existing = 0; existing < networkCount_; ++existing) {
      if (ssid.equals(networks_[existing].ssid)) {
        duplicate = true;
        break;
      }
    }
    if (duplicate) {
      continue;
    }

    WifiNetwork& network = networks_[networkCount_++];
    copyText(network.ssid, sizeof(network.ssid), ssid.c_str());
    network.rssi = WiFi.RSSI(index);
    network.secure = WiFi.encryptionType(index) != WIFI_AUTH_OPEN;
  }
  WiFi.scanDelete();
  ++scanGeneration_;
  state_ = connected() ? WifiState::Connected
                       : (hasCredentials() ? WifiState::Disconnected
                                           : WifiState::NotConfigured);
}

void WifiService::setError(const char* message) {
  copyText(error_, sizeof(error_), message);
}

}  // namespace services
