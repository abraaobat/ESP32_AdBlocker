#include <Arduino.h>

#include "appGlobals.h"
#include "deviceNames.h"

namespace {

constexpr size_t MAX_DEVICE_NAMES = 16;
constexpr const char* DEVICE_NAMES_PATH = DATA_DIR "/device-names.txt";

struct DeviceNameRecord {
  bool used = false;
  uint8_t mac[6] = {0};
  char name[DEVICE_NAME_MAX_LEN + 1] = {0};
};

DeviceNameRecord records[MAX_DEVICE_NAMES];
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
  for (size_t i = 0; i < MAX_DEVICE_NAMES; ++i) {
    if (records[i].used && sameMac(records[i].mac, mac)) return static_cast<int>(i);
  }
  return -1;
}

int allocateRecord(const uint8_t mac[6]) {
  for (size_t i = 0; i < MAX_DEVICE_NAMES; ++i) {
    if (!records[i].used) {
      records[i] = DeviceNameRecord{};
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

  if (!STORAGE.exists(DEVICE_NAMES_PATH)) return;
  File file = STORAGE.open(DEVICE_NAMES_PATH, FILE_READ);
  if (!file) return;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (!line.length()) continue;

    const int sep = line.indexOf('\t');
    if (sep <= 0) continue;

    String macText = line.substring(0, sep);
    String name = line.substring(sep + 1);
    name.trim();
    if (!name.length()) continue;

    uint8_t mac[6];
    if (!parseMac(macText.c_str(), mac)) continue;

    int index = findRecord(mac);
    if (index < 0) index = allocateRecord(mac);
    if (index < 0) break;

    name.replace("\t", " ");
    name.replace("\r", " ");
    name.replace("\n", " ");
    name = name.substring(0, DEVICE_NAME_MAX_LEN);
    strncpy(records[index].name, name.c_str(), DEVICE_NAME_MAX_LEN);
    records[index].name[DEVICE_NAME_MAX_LEN] = 0;
  }
  file.close();
}

bool saveAll() {
  File file = STORAGE.open(DEVICE_NAMES_PATH, FILE_WRITE);
  if (!file) {
    LOG_WRN("Failed to write %s", DEVICE_NAMES_PATH);
    return false;
  }

  for (const auto& record : records) {
    if (!record.used || !record.name[0]) continue;
    file.print(macToString(record.mac));
    file.print('\t');
    file.println(record.name);
  }
  file.close();
  return true;
}

}  // namespace

bool deviceNameGet(const uint8_t mac[6], char* outName, size_t outSize) {
  if (mac == nullptr || outName == nullptr || outSize == 0) return false;
  ensureLoaded();
  outName[0] = 0;
  const int index = findRecord(mac);
  if (index < 0 || !records[index].name[0]) return false;
  strncpy(outName, records[index].name, outSize - 1);
  outName[outSize - 1] = 0;
  return true;
}

bool deviceNameSet(const uint8_t mac[6], const char* name) {
  if (mac == nullptr || name == nullptr) return false;
  ensureLoaded();

  String cleaned(name);
  cleaned.trim();
  cleaned.replace("\t", " ");
  cleaned.replace("\r", " ");
  cleaned.replace("\n", " ");
  if (cleaned.length() > DEVICE_NAME_MAX_LEN) cleaned = cleaned.substring(0, DEVICE_NAME_MAX_LEN);

  int index = findRecord(mac);
  if (index < 0 && cleaned.length()) index = allocateRecord(mac);
  if (index < 0) return !cleaned.length();

  if (!cleaned.length()) {
    records[index] = DeviceNameRecord{};
  } else {
    strncpy(records[index].name, cleaned.c_str(), DEVICE_NAME_MAX_LEN);
    records[index].name[DEVICE_NAME_MAX_LEN] = 0;
  }

  if (!saveAll()) return false;
  LOG_INF("Device friendly name updated MAC=%s name=%s",
          macToString(mac).c_str(), cleaned.length() ? cleaned.c_str() : "<cleared>");
  return true;
}

bool deviceNameSetFromControlValue(const char* value) {
  if (value == nullptr) return false;
  String input(value);
  const int sep = input.indexOf('|');
  if (sep != 17) return false;

  String macText = input.substring(0, sep);
  String name = input.substring(sep + 1);
  uint8_t mac[6];
  if (!parseMac(macText.c_str(), mac)) return false;
  return deviceNameSet(mac, name.c_str());
}
