#include "config_store.h"

#include <Arduino.h>
#include <Preferences.h>
#include <esp_system.h>
#include <mbedtls/sha256.h>
#include <stdio.h>
#include <string.h>

#include "system_config.h"

// Mot de passe initial du tableau de bord (à changer à la première connexion).
#ifndef DASHBOARD_DEFAULT_PASSWORD
#define DASHBOARD_DEFAULT_PASSWORD RESCUE_PASS
#endif

namespace
{

  Preferences prefs;

  char g_salt[33] = {0};
  char g_hash[65] = {0};
  char g_deviceId[DEVICE_ID_MAX_LEN + 1] = DEVICE_ID;
  bool g_autoTime = true;
  bool g_ready = false;

  bool validDeviceId(const char *deviceId)
  {
    if (deviceId == nullptr)
      return false;

    size_t len = strlen(deviceId);
    if (len == 0 || len > DEVICE_ID_MAX_LEN)
      return false;

    for (size_t i = 0; i < len; i++)
    {
      char c = deviceId[i];
      if (!((c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_'))
        return false;
    }

    return true;
  }

  void sha256Hex(const char *input, char *outHex)
  {
    uint8_t hash[32];

    mbedtls_sha256(reinterpret_cast<const unsigned char *>(input), strlen(input), hash, 0);

    static const char hexDigits[] = "0123456789abcdef";

    for (int i = 0; i < 32; i++)
    {
      outHex[i * 2] = hexDigits[(hash[i] >> 4) & 0xF];
      outHex[i * 2 + 1] = hexDigits[hash[i] & 0xF];
    }

    outHex[64] = '\0';
  }

  // Comparaison à temps constant (évite de deviner le hash octet par octet
  // en mesurant le temps de réponse).
  bool constantTimeEquals(const char *a, const char *b, size_t len)
  {
    uint8_t diff = 0;

    for (size_t i = 0; i < len; i++)
      diff |= static_cast<uint8_t>(a[i]) ^ static_cast<uint8_t>(b[i]);

    return diff == 0;
  }

  void hashPassword(const char *salt, const char *password, char *outHex)
  {
    char buffer[PASSWORD_MAX_LEN + 40];

    snprintf(buffer, sizeof(buffer), "%s:%s", salt, password);

    sha256Hex(buffer, outHex);
  }

  void randomHex(char *out, size_t bytes)
  {
    static const char hexDigits[] = "0123456789abcdef";

    for (size_t i = 0; i < bytes; i++)
    {
      uint8_t b = static_cast<uint8_t>(esp_random() & 0xFF);

      out[i * 2] = hexDigits[(b >> 4) & 0xF];
      out[i * 2 + 1] = hexDigits[b & 0xF];
    }

    out[bytes * 2] = '\0';
  }

  bool hasStoredPassword()
  {
    return strlen(g_salt) == 32 && strlen(g_hash) == 64;
  }

} // namespace

void config_init(void)
{
  prefs.begin("pointeuse", false);

  g_salt[0] = '\0';
  g_hash[0] = '\0';

  prefs.getString("pwsalt", g_salt, sizeof(g_salt));
  prefs.getString("pwhash", g_hash, sizeof(g_hash));

  g_autoTime = prefs.getBool("autotime", true);

  String storedDeviceId = prefs.getString("deviceid", DEVICE_ID);
  if (validDeviceId(storedDeviceId.c_str()))
    snprintf(g_deviceId, sizeof(g_deviceId), "%s", storedDeviceId.c_str());
  else
    snprintf(g_deviceId, sizeof(g_deviceId), "%s", DEVICE_ID);

  g_ready = true;
}

bool config_check_password(const char *plain)
{
  if (!g_ready)
    config_init();

  if (plain == nullptr)
    return false;

  char candidate[65];

  if (hasStoredPassword())
  {
    hashPassword(g_salt, plain, candidate);

    return constantTimeEquals(candidate, g_hash, 64);
  }

  // Aucun mot de passe choisi : mot de passe par défaut.
  char expected[65];

  sha256Hex(plain, candidate);
  sha256Hex(DASHBOARD_DEFAULT_PASSWORD, expected);

  return constantTimeEquals(candidate, expected, 64);
}

bool config_set_password(const char *plain)
{
  if (!g_ready)
    config_init();

  if (plain == nullptr)
    return false;

  size_t len = strlen(plain);

  if (len < PASSWORD_MIN_LEN || len > PASSWORD_MAX_LEN)
    return false;

  char salt[33];
  char hash[65];

  randomHex(salt, 16);
  hashPassword(salt, plain, hash);

  if (prefs.putString("pwsalt", salt) == 0 || prefs.putString("pwhash", hash) == 0)
    return false;

  memcpy(g_salt, salt, sizeof(g_salt));
  memcpy(g_hash, hash, sizeof(g_hash));

  return true;
}

bool config_password_is_default(void)
{
  if (!g_ready)
    config_init();

  return !hasStoredPassword();
}

bool config_auto_time_sync(void)
{
  if (!g_ready)
    config_init();

  return g_autoTime;
}

void config_set_auto_time_sync(bool enabled)
{
  if (!g_ready)
    config_init();

  g_autoTime = enabled;

  prefs.putBool("autotime", enabled);
}

const char *config_device_id(void)
{
  if (!g_ready)
    config_init();

  return g_deviceId;
}

bool config_device_id_valid(const char *device_id)
{
  return validDeviceId(device_id);
}

bool config_set_device_id(const char *device_id)
{
  if (!g_ready)
    config_init();

  if (!config_device_id_valid(device_id))
    return false;

  if (prefs.putString("deviceid", device_id) == 0)
    return false;

  snprintf(g_deviceId, sizeof(g_deviceId), "%s", device_id);
  return true;
}