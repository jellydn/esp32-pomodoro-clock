#include <unity.h>

#include <cstring>

#include "core/wifi_connection_policy.h"

using wifi::DisconnectFailure;

namespace {

void test_ignores_voluntary_disconnect_from_wifi_begin() {
  TEST_ASSERT_EQUAL(static_cast<int>(DisconnectFailure::Ignore),
                    static_cast<int>(wifi::classifyDisconnectReason(8)));
}

void test_maps_actionable_disconnect_reasons() {
  TEST_ASSERT_EQUAL(static_cast<int>(DisconnectFailure::NetworkUnavailable),
                    static_cast<int>(wifi::classifyDisconnectReason(201)));
  TEST_ASSERT_EQUAL(static_cast<int>(DisconnectFailure::Authentication),
                    static_cast<int>(wifi::classifyDisconnectReason(202)));
  TEST_ASSERT_EQUAL(static_cast<int>(DisconnectFailure::UnsupportedSecurity),
                    static_cast<int>(wifi::classifyDisconnectReason(20)));
  TEST_ASSERT_EQUAL(static_cast<int>(DisconnectFailure::Connection),
                    static_cast<int>(wifi::classifyDisconnectReason(203)));
}

void test_accepts_passphrases_and_full_length_hex_keys() {
  const char* passphrase = "correct horse battery staple";
  const char* hexKey = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";

  TEST_ASSERT_TRUE(wifi::isValidPersonalPassword(passphrase, strlen(passphrase)));
  TEST_ASSERT_TRUE(wifi::isValidPersonalPassword(hexKey, strlen(hexKey)));
}

void test_rejects_truncated_or_invalid_passwords() {
  const char* tooShort = "1234567";
  const char* nonHexKey = "z123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
  const char* newline = "password\n";

  TEST_ASSERT_FALSE(wifi::isValidPersonalPassword(tooShort, strlen(tooShort)));
  TEST_ASSERT_FALSE(wifi::isValidPersonalPassword(nonHexKey, strlen(nonHexKey)));
  TEST_ASSERT_FALSE(wifi::isValidPersonalPassword(newline, strlen(newline)));
  TEST_ASSERT_TRUE(wifi::isValidPersonalPassword(" password ", 10));
}

}  // namespace

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_ignores_voluntary_disconnect_from_wifi_begin);
  RUN_TEST(test_maps_actionable_disconnect_reasons);
  RUN_TEST(test_accepts_passphrases_and_full_length_hex_keys);
  RUN_TEST(test_rejects_truncated_or_invalid_passwords);
  return UNITY_END();
}
