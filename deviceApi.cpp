#include <Arduino.h>
#include <WiFi.h>

#include "appGlobals.h"
#include "deviceApi.h"
#include "deviceNames.h"
#include "deviceProfiles.h"
#include "deviceRegistry.h"

bool checkAuth(httpd_req_t* req);

namespace {

String macToString(const uint8_t mac[6]) {
  char text[18];
  snprintf(
    text,
    sizeof(text),
    "%02X:%02X:%02X:%02X:%02X:%02X",
    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
  );
  return String(text);
}

String ipToString(uint32_t ipv4) {
  return ipv4 == 0 ? String("") : IPAddress(ipv4).toString();
}

String jsonEscape(const char* value) {
  String out;
  if (value == nullptr) return out;
  while (*value) {
    const char c = *value++;
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '"': out += "\\\""; break;
      case '\b': out += "\\b"; break;
      case '\f': out += "\\f"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (static_cast<uint8_t>(c) < 0x20) {
          char escaped[7];
          snprintf(escaped, sizeof(escaped), "\\u%04x", static_cast<uint8_t>(c));
          out += escaped;
        } else {
          out += c;
        }
    }
  }
  return out;
}

}  // namespace

esp_err_t deviceApiHandler(httpd_req_t* req) {
  if (!checkAuth(req)) {
    return ESP_OK;
  }

  constexpr size_t MAX_ENTRIES = 16;
  DeviceRegistryEntry entries[MAX_ENTRIES];
  const size_t copied = deviceRegistryCopy(entries, MAX_ENTRIES);

  String payload;
  payload.reserve(360 + copied * 240);
  payload += "{\"online\":";
  payload += String(deviceRegistryOnlineCount());
  payload += ",\"known\":";
  payload += String(deviceRegistryCount());
  payload += ",\"profiles\":[\"standard\",\"iot-strict\",\"unrestricted\"]";
  payload += ",\"devices\":[";

  for (size_t i = 0; i < copied; ++i) {
    if (i > 0) {
      payload += ',';
    }

    const DeviceRegistryEntry& entry = entries[i];
    char friendlyName[DEVICE_NAME_MAX_LEN + 1] = {0};
    char profileId[DEVICE_PROFILE_ID_MAX_LEN + 1] = {0};
    deviceNameGet(entry.mac, friendlyName, sizeof(friendlyName));
    deviceProfileGet(entry.mac, profileId, sizeof(profileId));

    payload += "{\"mac\":\"";
    payload += macToString(entry.mac);
    payload += "\",\"name\":\"";
    payload += jsonEscape(friendlyName);
    payload += "\",\"profile\":\"";
    payload += jsonEscape(profileId);
    payload += "\",\"ipv4\":\"";
    payload += ipToString(entry.ipv4);
    payload += "\",\"online\":";
    payload += entry.online ? "true" : "false";
    payload += ",\"firstSeenMs\":";
    payload += String(entry.firstSeenMs);
    payload += ",\"lastSeenMs\":";
    payload += String(entry.lastSeenMs);
    payload += ",\"connectCount\":";
    payload += String(entry.connectCount);
    payload += '}';
  }

  payload += "]}";

  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, payload.c_str(), payload.length());
}
