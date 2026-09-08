#include <Arduino.h>
#include <Network.h>
#include <WiFi.h>
#include <esp_err.h>
#include <esp_netif.h>
#include <esp_wifi.h>

#include "deviceRegistry.h"

namespace {

constexpr size_t MAX_DEVICE_RECORDS = 16;

struct DeviceRecord {
  bool used = false;
  bool online = false;
  uint8_t mac[6] = {0};
  uint32_t ipv4 = 0;
  uint32_t firstSeenMs = 0;
  uint32_t lastSeenMs = 0;
  uint32_t connectCount = 0;
};

DeviceRecord records[MAX_DEVICE_RECORDS];
bool registryStarted = false;

bool sameMac(const uint8_t a[6], const uint8_t b[6]) {
  return memcmp(a, b, 6) == 0;
}

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
  if (ipv4 == 0) {
    return String("pending");
  }
  return IPAddress(ipv4).toString();
}

int findRecord(const uint8_t mac[6]) {
  for (size_t i = 0; i < MAX_DEVICE_RECORDS; ++i) {
    if (records[i].used && sameMac(records[i].mac, mac)) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

int allocateRecord(const uint8_t mac[6], uint32_t nowMs) {
  for (size_t i = 0; i < MAX_DEVICE_RECORDS; ++i) {
    if (!records[i].used) {
      records[i] = DeviceRecord{};
      records[i].used = true;
      memcpy(records[i].mac, mac, 6);
      records[i].firstSeenMs = nowMs;
      records[i].lastSeenMs = nowMs;
      return static_cast<int>(i);
    }
  }

  int oldestOffline = -1;
  uint32_t oldestSeen = UINT32_MAX;
  for (size_t i = 0; i < MAX_DEVICE_RECORDS; ++i) {
    if (records[i].used && !records[i].online && records[i].lastSeenMs <= oldestSeen) {
      oldestSeen = records[i].lastSeenMs;
      oldestOffline = static_cast<int>(i);
    }
  }

  if (oldestOffline >= 0) {
    records[oldestOffline] = DeviceRecord{};
    records[oldestOffline].used = true;
    memcpy(records[oldestOffline].mac, mac, 6);
    records[oldestOffline].firstSeenMs = nowMs;
    records[oldestOffline].lastSeenMs = nowMs;
  }

  return oldestOffline;
}

size_t knownCount() {
  size_t count = 0;
  for (const auto& record : records) {
    if (record.used) {
      ++count;
    }
  }
  return count;
}

size_t onlineCount() {
  size_t count = 0;
  for (const auto& record : records) {
    if (record.used && record.online) {
      ++count;
    }
  }
  return count;
}

void onRegistryNetworkEvent(arduino_event_id_t event, arduino_event_info_t info) {
  (void)info;

  switch (event) {
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      deviceRegistryRefresh("ap-connect");
      deviceRegistryDump();
      break;

    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      deviceRegistryRefresh("dhcp-lease");
      deviceRegistryDump();
      break;

    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      deviceRegistryRefresh("ap-disconnect");
      deviceRegistryDump();
      break;

    default:
      break;
  }
}

}  // namespace

void deviceRegistryBegin() {
  if (registryStarted) {
    return;
  }

  registryStarted = true;
  Network.onEvent(onRegistryNetworkEvent);

  Serial.printf("[DEVICE] Registry started (capacity=%u)\n", static_cast<unsigned>(MAX_DEVICE_RECORDS));
  deviceRegistryRefresh("startup");
}

void deviceRegistryRefresh(const char* reason) {
  if (!registryStarted) {
    return;
  }

  wifi_sta_list_t stations = {};
  esp_err_t err = esp_wifi_ap_get_sta_list(&stations);
  if (err != ESP_OK) {
    Serial.printf("[DEVICE] AP station list failed: 0x%X (%s)\n", err, esp_err_to_name(err));
    return;
  }

  const size_t currentCount = min(
    static_cast<size_t>(stations.num),
    static_cast<size_t>(ESP_WIFI_MAX_CONN_NUM)
  );

  esp_netif_pair_mac_ip_t leasePairs[ESP_WIFI_MAX_CONN_NUM] = {};
  for (size_t i = 0; i < currentCount; ++i) {
    memcpy(leasePairs[i].mac, stations.sta[i].mac, 6);
  }

  bool leaseLookupOk = true;
  if (currentCount > 0) {
    esp_netif_t* apNetif = WiFi.AP.netif();
    if (apNetif == nullptr) {
      leaseLookupOk = false;
      Serial.println("[DEVICE] DHCP lookup skipped: AP netif unavailable");
    } else {
      err = esp_netif_dhcps_get_clients_by_mac(
        apNetif,
        static_cast<int>(currentCount),
        leasePairs
      );
      if (err != ESP_OK) {
        leaseLookupOk = false;
        Serial.printf("[DEVICE] DHCP MAC->IPv4 lookup pending/failed: 0x%X (%s)\n", err, esp_err_to_name(err));
      }
    }
  }

  bool seenNow[MAX_DEVICE_RECORDS] = {false};
  const uint32_t nowMs = millis();

  for (size_t i = 0; i < currentCount; ++i) {
    const uint8_t* mac = stations.sta[i].mac;
    int index = findRecord(mac);
    if (index < 0) {
      index = allocateRecord(mac, nowMs);
      if (index < 0) {
        Serial.printf("[DEVICE] Registry full; cannot track %s\n", macToString(mac).c_str());
        continue;
      }
      Serial.printf("[DEVICE] discovered MAC=%s\n", macToString(mac).c_str());
    }

    DeviceRecord& record = records[index];
    seenNow[index] = true;
    record.lastSeenMs = nowMs;

    if (!record.online) {
      record.online = true;
      ++record.connectCount;
      Serial.printf(
        "[DEVICE] connected MAC=%s count=%lu\n",
        macToString(record.mac).c_str(),
        static_cast<unsigned long>(record.connectCount)
      );
    }

    const uint32_t newIpv4 = leaseLookupOk ? leasePairs[i].ip.addr : 0;
    if (newIpv4 != 0 && newIpv4 != record.ipv4) {
      const String oldIp = ipToString(record.ipv4);
      record.ipv4 = newIpv4;
      Serial.printf(
        "[DEVICE] lease MAC=%s IPv4=%s (was %s)\n",
        macToString(record.mac).c_str(),
        ipToString(record.ipv4).c_str(),
        oldIp.c_str()
      );
    }
  }

  for (size_t i = 0; i < MAX_DEVICE_RECORDS; ++i) {
    DeviceRecord& record = records[i];
    if (record.used && record.online && !seenNow[i]) {
      record.online = false;
      record.lastSeenMs = nowMs;
      Serial.printf(
        "[DEVICE] disconnected MAC=%s lastIPv4=%s\n",
        macToString(record.mac).c_str(),
        ipToString(record.ipv4).c_str()
      );
    }
  }

  if (reason != nullptr) {
    Serial.printf(
      "[DEVICE] refresh=%s online=%u known=%u\n",
      reason,
      static_cast<unsigned>(onlineCount()),
      static_cast<unsigned>(knownCount())
    );
  }
}

void deviceRegistryDump() {
  if (!registryStarted) {
    return;
  }

  Serial.printf(
    "[DEVICE] registry online=%u known=%u\n",
    static_cast<unsigned>(onlineCount()),
    static_cast<unsigned>(knownCount())
  );

  for (size_t i = 0; i < MAX_DEVICE_RECORDS; ++i) {
    const DeviceRecord& record = records[i];
    if (!record.used) {
      continue;
    }

    Serial.printf(
      "[DEVICE] #%u %s MAC=%s IPv4=%s first=%lu last=%lu connects=%lu\n",
      static_cast<unsigned>(i + 1),
      record.online ? "ONLINE" : "OFFLINE",
      macToString(record.mac).c_str(),
      ipToString(record.ipv4).c_str(),
      static_cast<unsigned long>(record.firstSeenMs),
      static_cast<unsigned long>(record.lastSeenMs),
      static_cast<unsigned long>(record.connectCount)
    );
  }
}

size_t deviceRegistryCount() {
  return knownCount();
}

size_t deviceRegistryOnlineCount() {
  return onlineCount();
}

size_t deviceRegistryCopy(DeviceRegistryEntry* outEntries, size_t capacity) {
  if (outEntries == nullptr || capacity == 0) {
    return 0;
  }

  size_t copied = 0;
  for (const auto& record : records) {
    if (!record.used || copied >= capacity) {
      continue;
    }

    DeviceRegistryEntry& entry = outEntries[copied++];
    entry = DeviceRegistryEntry{};
    entry.online = record.online;
    memcpy(entry.mac, record.mac, sizeof(entry.mac));
    entry.ipv4 = record.ipv4;
    entry.firstSeenMs = record.firstSeenMs;
    entry.lastSeenMs = record.lastSeenMs;
    entry.connectCount = record.connectCount;
  }

  return copied;
}
