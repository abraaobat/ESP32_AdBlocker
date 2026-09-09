#include <Arduino.h>

#include "appGlobals.h"
#include "deviceProfiles.h"

namespace {

constexpr size_t MAX_DEVICE_PROFILES = 16;
constexpr const char* DEVICE_PROFILES_PATH = DATA_DIR "/device-profiles.txt";

struct DeviceProfileRecord {
  bool used = false;
  uint8_t mac[6] = {0};
  char profile[DEVICE_PROFILE_ID_MAX_LEN + 1] = {0};
};

DeviceProfileRecord records[MAX_DEVICE_PROFILES];
bool loaded = false;

bool sameMac(const uint8_t a[6], const uint8_t b[6]) {
  return memcmp(a, b, 6) == 0;
}

String macToString(const uint8_t mac[6]) {
  char text[18];
  snprintf(text, sizeof(text), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(text);
}

bool parseMac(const char* text, uint8_t mac[6]) {
  if (text == nullptr || strlen(text) != 17) return false;
  unsigned int b[6];
  if (sscanf(text, "%2x:%2x:%2x:%2x:%2x:%2x",
             &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) != 6) return false;
  for (size_t i = 0; i < 6; ++i) mac[i] = static_cast<uint8_t>(b[i]);
  return true;
}

int findRecord(const uint8_t mac[6]) {
  for (size_t i = 0; i < MAX_DEVICE_PROFILES; ++i) {
    if (records[i].used && sameMac(records[i].mac, mac)) return static_cast<int>(i);
  }
  return -1;
}

int allocateRecord(const uint8_t mac[6]) {
  for (size_t i = 0; i < MAX_DEVICE_PROFILES; ++i) {
    if (!records[i].used) {
      records[i] = DeviceProfileRecord{};
      records[i].used = true;
      memcpy(records[i].mac, mac, 6);
      return static_cast<int>(i);
    }
  }
  return -1;
}

void ensureLoaded() {
  if (loaded) return;
  loaded = true;

  if (!STORAGE.exists(DEVICE_PROFILES_PATH)) return;
  File file = STORAGE.open(DEVICE_PROFILES_PATH, FILE_READ);
  if (!file) return;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (!line.length()) continue;

    const int sep = line.indexOf('\t');
    if (sep <= 0) continue;

    String macText = line.substring(0, sep);
    String profile = line.substring(sep + 1);
    profile.trim();
    if (!deviceProfileIsKnown(profile.c_str())) continue;

    uint8_t mac[6];
    if (!parseMac(macText.c_str(), mac)) continue;

    int index = findRecord(mac);
    if (index < 0) index = allocateRecord(mac);
    if (index < 0) break;

    strncpy(records[index].profile, profile.c_str(), DEVICE_PROFILE_ID_MAX_LEN);
    records[index].profile[DEVICE_PROFILE_ID_MAX_LEN] = 0;
  }
  file.close();
}

bool saveAll() {
  File file = STORAGE.open(DEVICE_PROFILES_PATH, FILE_WRITE);
  if (!file) {
    LOG_WRN("Failed to write %s", DEVICE_PROFILES_PATH);
    return false;
  }

  for (const auto& record : records) {
    if (!record.used || !record.profile[0]) continue;
    file.print(macToString(record.mac));
    file.print('\t');
    file.println(record.profile);
  }
  file.close();
  return true;
}

}  // namespace

bool deviceProfileIsKnown(const char* profileId) {
  if (profileId == nullptr) return false;
  return !strcmp(profileId, DEVICE_PROFILE_STANDARD) ||
         !strcmp(profileId, DEVICE_PROFILE_IOT_STRICT) ||
         !strcmp(profileId, DEVICE_PROFILE_UNRESTRICTED);
}

bool deviceProfileGet(const uint8_t mac[6], char* outProfile, size_t outSize) {
  if (mac == nullptr || outProfile == nullptr || outSize == 0) return false;
  ensureLoaded();

  const char* resolved = DEVICE_PROFILE_STANDARD;
  const int index = findRecord(mac);
  if (index >= 0 && records[index].profile[0] && deviceProfileIsKnown(records[index].profile)) {
    resolved = records[index].profile;
  }

  strncpy(outProfile, resolved, outSize - 1);
  outProfile[outSize - 1] = 0;
  return index >= 0;
}

bool deviceProfileSet(const uint8_t mac[6], const char* profileId) {
  if (mac == nullptr || !deviceProfileIsKnown(profileId)) return false;
  ensureLoaded();

  int index = findRecord(mac);
  if (index < 0) index = allocateRecord(mac);
  if (index < 0) return false;

  strncpy(records[index].profile, profileId, DEVICE_PROFILE_ID_MAX_LEN);
  records[index].profile[DEVICE_PROFILE_ID_MAX_LEN] = 0;

  if (!saveAll()) return false;
  LOG_INF("Device profile updated MAC=%s profile=%s",
          macToString(mac).c_str(), records[index].profile);
  return true;
}

bool deviceProfileSetFromControlValue(const char* value) {
  if (value == nullptr) return false;
  String input(value);
  const int sep = input.indexOf('|');
  if (sep != 17) return false;

  String macText = input.substring(0, sep);
  String profile = input.substring(sep + 1);
  profile.trim();

  uint8_t mac[6];
  if (!parseMac(macText.c_str(), mac)) return false;
  return deviceProfileSet(mac, profile.c_str());
}
