#include <Arduino.h>

#include "deviceProfiles.h"
#include "deviceRegistry.h"
#include "dnsClientAttribution.h"

namespace {

String macToString(const uint8_t mac[6]) {
  char text[18];
  snprintf(text, sizeof(text), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(text);
}

}  // namespace

DnsClientContext dnsClientAttributionResolve(uint32_t sourceIpv4) {
  DnsClientContext ctx;
  ctx.ipv4 = sourceIpv4;
  strncpy(ctx.profile, DEVICE_PROFILE_STANDARD, sizeof(ctx.profile) - 1);
  ctx.profile[sizeof(ctx.profile) - 1] = 0;

  DeviceRegistryEntry entry;
  if (!deviceRegistryFindOnlineByIPv4(sourceIpv4, &entry)) {
    return ctx;
  }

  ctx.attributed = true;
  memcpy(ctx.mac, entry.mac, sizeof(ctx.mac));
  deviceProfileGet(entry.mac, ctx.profile, sizeof(ctx.profile));

  LOG_VRB("DNS client attributed IP=%s MAC=%s profile=%s",
          IPAddress(sourceIpv4).toString().c_str(),
          macToString(ctx.mac).c_str(),
          ctx.profile);
  return ctx;
}
