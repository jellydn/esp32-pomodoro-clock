#include "wifi_service.h"

#include <Preferences.h>
#include <WiFi.h>

#include "core/wifi_connection_policy.h"

#if __has_include("wifi_config.h")
#include "wifi_config.h"
#else
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#endif

namespace services {

namespace {
constexpr std::uint32_t kConnectTimeoutMs = 30000;
constexpr std::uint32_t kReconnectIntervalMs = 30000;

void copyText(char* destination, std::size_t size, const char* source) {
  snprintf(destination, size, "%s", source == nullptr ? "" : source);
}

bool containsOnlyPrintableAscii(const char* text) {
  for (const char* character = text; *character != '\0'; ++character) {
    if (*character < 32 || *character > 126) {
      return false;
    }
  }
  return true;
}
}  // namespace

void WifiService::begin() {
  resetWifiStorage();
  WiFi.setAutoReconnect(false);
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
      ++gotIpGeneration_;
      Serial.println("Wi-Fi stage=got-ip");
    } else if (event == ARDUINO_EVENT_WIFI_STA_CONNECTED) {
      associated_ = true;
      Serial.println("Wi-Fi stage=associated; waiting for DHCP");
    } else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
      disconnectReason_ = info.wifi_sta_disconnected.reason;
      associated_ = false;
      ++disconnectGeneration_;
      Serial.printf("Wi-Fi stage=disconnected reason=%u (%s)\n", disconnectReason_,
                    WiFi.disconnectReasonName(
                        static_cast<wifi_err_reason_t>(disconnectReason_)));
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
      if (wifi::classifyDisconnectReason(reason) == wifi::DisconnectFailure::Ignore) {
        attemptDisconnectGeneration_ = disconnectGeneration_;
        Serial.println("Wi-Fi stage=connecting; ignored voluntary driver disconnect");
      } else {
        failForDisconnectReason(reason);
      }
      return;
    }

    if (millis() - attemptStartedMs_ >= kConnectTimeoutMs) {
      failConnection(associated_ ? "IP address request timed out."
                                 : "Wi-Fi association timed out.",
                     true);
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

const char* WifiService::connectionStatus() const {
  if (connectionStage_ == ConnectionStage::WaitingForDisconnect) {
    return "Disconnecting from current network";
  }
  return associated_ ? "Associated; getting IP address" : "Associating";
}

const char* WifiService::securityName(std::uint8_t authMode) {
  switch (static_cast<wifi_auth_mode_t>(authMode)) {
    case WIFI_AUTH_OPEN:
      return "open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA/WPA2";
    case WIFI_AUTH_ENTERPRISE:
      return "enterprise";
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WPA2/WPA3";
    default:
      return "unknown";
  }
}

bool WifiService::supportsSecurity(std::uint8_t authMode) {
  const wifi_auth_mode_t mode = static_cast<wifi_auth_mode_t>(authMode);
  return mode == WIFI_AUTH_OPEN || mode == WIFI_AUTH_WPA2_PSK ||
         mode == WIFI_AUTH_WPA_WPA2_PSK || mode == WIFI_AUTH_WPA3_PSK ||
         mode == WIFI_AUTH_WPA2_WPA3_PSK;
}

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
  associated_ = false;
  attemptDisconnectGeneration_ = disconnectGeneration_;
  attemptGotIpGeneration_ = gotIpGeneration_;
  WiFi.begin(pendingSsid_, pendingPassword_);
  attemptStartedMs_ = millis();
  const std::size_t passwordLength = strlen(pendingPassword_);
  const bool edgeSpace = passwordLength > 0 &&
                         (pendingPassword_[0] == ' ' ||
                          pendingPassword_[passwordLength - 1] == ' ');
  Serial.printf(
      "Wi-Fi stage=associating ssid=\"%s\" ssidLength=%u passphraseLength=%u "
      "printableAscii=%s edgeSpace=%s\n",
      pendingSsid_, static_cast<unsigned>(strlen(pendingSsid_)),
      static_cast<unsigned>(passwordLength),
      containsOnlyPrintableAscii(pendingPassword_) ? "yes" : "no",
      edgeSpace ? "yes" : "no");
}

void WifiService::failForDisconnectReason(std::uint8_t reason) {
  const char* summary = "Connection failed";
  switch (wifi::classifyDisconnectReason(reason)) {
    case wifi::DisconnectFailure::NetworkUnavailable:
      summary = "Network unavailable";
      break;
    case wifi::DisconnectFailure::Authentication:
      summary = "Authentication failed";
      break;
    case wifi::DisconnectFailure::UnsupportedSecurity:
      summary = "Unsupported network security";
      break;
    case wifi::DisconnectFailure::Ignore:
    case wifi::DisconnectFailure::Connection:
      break;
  }
  char message[sizeof(error_)];
  snprintf(message, sizeof(message), "%s (%u %s).", summary, reason,
           WiFi.disconnectReasonName(static_cast<wifi_err_reason_t>(reason)));
  failConnection(message, true);
}

void WifiService::failConnection(const char* message, bool disconnectFirst) {
  setError(message);
  Serial.printf("Wi-Fi stage=failed message=\"%s\"\n", message);
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
    network.authMode = static_cast<std::uint8_t>(WiFi.encryptionType(index));
    network.secure = network.authMode != WIFI_AUTH_OPEN;
    Serial.printf("Wi-Fi scan ssid=\"%s\" rssi=%ld auth=%u (%s) supported=%s\n",
                  network.ssid, static_cast<long>(network.rssi), network.authMode,
                  securityName(network.authMode),
                  supportsSecurity(network.authMode) ? "yes" : "no");
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
