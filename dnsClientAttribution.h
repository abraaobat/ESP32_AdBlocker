#pragma once

#include <Arduino.h>
#include "deviceProfiles.h"

struct DnsClientContext {
  bool attributed = false;
  uint32_t ipv4 = 0;
  uint8_t mac[6] = {0};
  char profile[DEVICE_PROFILE_ID_MAX_LEN + 1] = {0};
};

DnsClientContext dnsClientAttributionResolve(uint32_t sourceIpv4);
