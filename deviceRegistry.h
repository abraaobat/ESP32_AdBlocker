#pragma once

#include <Arduino.h>

struct DeviceRegistryEntry {
  bool online = false;
  uint8_t mac[6] = {0};
  uint32_t ipv4 = 0;
  uint32_t firstSeenMs = 0;
  uint32_t lastSeenMs = 0;
  uint32_t connectCount = 0;
};

void deviceRegistryBegin();
void deviceRegistryRefresh(const char* reason = nullptr);
void deviceRegistryDump();
size_t deviceRegistryCount();
size_t deviceRegistryOnlineCount();
size_t deviceRegistryCopy(DeviceRegistryEntry* outEntries, size_t capacity);
