#include <Arduino.h>

#include "appGlobals.h"
#include "deviceProfiles.h"
#include "dnsProfilePolicy.h"

DnsProfilePolicyDecision resolveDnsForClientProfile(
  const DnsClientContext& client,
  const char* domainName,
  IPAddress& retIP
) {
  DnsProfilePolicyDecision decision;

  if (!client.attributed) {
    decision.reason = DnsProfilePolicyReason::FallbackStandard;
    decision.result = checkBlocklist(domainName, retIP);
    return decision;
  }

  if (!strcmp(client.profile, DEVICE_PROFILE_UNRESTRICTED)) {
    decision.reason = DnsProfilePolicyReason::UnrestrictedDirectResolve;
    decision.result = resolveDomainStatus(domainName, retIP);
    return decision;
  }

  if (!strcmp(client.profile, DEVICE_PROFILE_IOT_STRICT)) {
    decision.reason = DnsProfilePolicyReason::IoTStrictBlocklist;
    decision.result = checkBlocklist(domainName, retIP);
    return decision;
  }

  decision.reason = DnsProfilePolicyReason::StandardBlocklist;
  decision.result = checkBlocklist(domainName, retIP);
  return decision;
}

const char* dnsProfilePolicyReasonName(DnsProfilePolicyReason reason) {
  switch (reason) {
    case DnsProfilePolicyReason::StandardBlocklist:
      return "standard-blocklist";
    case DnsProfilePolicyReason::IoTStrictBlocklist:
      return "iot-strict-blocklist";
    case DnsProfilePolicyReason::UnrestrictedDirectResolve:
      return "unrestricted-direct-resolve";
    case DnsProfilePolicyReason::FallbackStandard:
    default:
      return "fallback-standard";
  }
}
