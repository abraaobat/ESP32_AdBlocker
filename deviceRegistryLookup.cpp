#include "deviceRegistry.h"

bool deviceRegistryFindOnlineByIPv4(uint32_t ipv4, DeviceRegistryEntry* outEntry) {
  if (ipv4 == 0 || outEntry == nullptr) return false;

  constexpr size_t MAX_ENTRIES = 16;
  DeviceRegistryEntry entries[MAX_ENTRIES];
  const size_t copied = deviceRegistryCopy(entries, MAX_ENTRIES);

  for (size_t i = 0; i < copied; ++i) {
    if (entries[i].online && entries[i].ipv4 == ipv4) {
      *outEntry = entries[i];
      return true;
    }
  }

  return false;
}
