#pragma once

#include <Arduino.h>

#include "appGlobals.h"
#include "dnsClientAttribution.h"

enum class DnsProfilePolicyReason : uint8_t {
  StandardBlocklist = 0,
  IoTStrictBlocklist,
  UnrestrictedDirectResolve,
  FallbackStandard
};

struct DnsProfilePolicyDecision {
  DnsResult result = DNS_SERVFAIL;
  DnsProfilePolicyReason reason = DnsProfilePolicyReason::FallbackStandard;
};

DnsProfilePolicyDecision resolveDnsForClientProfile(
  const DnsClientContext& client,
  const char* domainName,
  IPAddress& retIP
);

const char* dnsProfilePolicyReasonName(DnsProfilePolicyReason reason);
