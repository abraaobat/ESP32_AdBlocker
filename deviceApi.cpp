#include <Arduino.h>
#include <WiFi.h>

#include "appGlobals.h"
#include "deviceApi.h"
#include "deviceRegistry.h"

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

}  // namespace

esp_err_t deviceApiHandler(httpd_req_t* req) {
  if (!checkAuth(req)) {
    return ESP_OK;
  }

  constexpr size_t MAX_ENTRIES = 16;
  DeviceRegistryEntry entries[MAX_ENTRIES];
  const size_t copied = deviceRegistryCopy(entries, MAX_ENTRIES);

  String payload;
  payload.reserve(256 + copied * 160);
  payload += "{\"online\":";
  payload += String(deviceRegistryOnlineCount());
  payload += ",\"known\":";
  payload += String(deviceRegistryCount());
  payload += ",\"devices\":[";

  for (size_t i = 0; i < copied; ++i) {
    if (i > 0) {
      payload += ',';
    }

    const DeviceRegistryEntry& entry = entries[i];
    payload += "{\"mac\":\"";
    payload += macToString(entry.mac);
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
