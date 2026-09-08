#include <Arduino.h>
#include <WiFi.h>
#include "appGlobals.h"
#include "natConfig.h"

namespace {

IPAddress protectedIP(192, 168, 4, 1);
IPAddress protectedMask(255, 255, 255, 0);
IPAddress protectedLeaseStart(192, 168, 4, 10);
IPAddress protectedDNS(192, 168, 4, 1);

bool protectedNatAPEnabled = false;

bool protectedAPConfigReady() {
  if (!PROTECTED_AP_HAS_LOCAL_SECRETS) {
    Serial.println("[NAT-AP] Disabled: copy natSecrets.example.h to natSecrets.h and set a password");
    return false;
  }

  const size_t ssidLen = strlen(PROTECTED_AP_SSID);
  const size_t passLen = strlen(PROTECTED_AP_PASS);

  if (ssidLen == 0 || ssidLen > 32) {
    Serial.println("[NAT-AP] ERROR: protected AP SSID must be 1..32 characters");
    return false;
  }

  if (strcmp(PROTECTED_AP_PASS, "CHANGE_ME") == 0 || passLen < 8 || passLen > 63) {
    Serial.println("[NAT-AP] ERROR: set a private 8..63 character password in natSecrets.h");
    return false;
  }

  return true;
}

}

bool startProtectedNatAP() {

  Serial.println();
  Serial.println("[NAT-AP] Starting protected Wi-Fi...");

  if (!protectedAPConfigReady()) {
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[NAT-AP] ERROR: upstream STA is not connected");
    return false;
  }

  Serial.printf(
    "[NAT-AP] Upstream: %s / IP %s / GW %s\n",
    WiFi.SSID().c_str(),
    WiFi.localIP().toString().c_str(),
    WiFi.gatewayIP().toString().c_str()
  );

  // Stop the setup AP if it is already active.
  WiFi.AP.end();
  delay(200);

  WiFi.AP.begin();

  if (!WiFi.AP.config(
        protectedIP,
        protectedIP,
        protectedMask,
        protectedLeaseStart,
        protectedDNS
      )) {
    Serial.println("[NAT-AP] ERROR: AP network config failed");
    return false;
  }

  if (!WiFi.AP.create(PROTECTED_AP_SSID, PROTECTED_AP_PASS)) {
    Serial.println("[NAT-AP] ERROR: AP creation failed");
    return false;
  }

  if (!WiFi.AP.waitStatusBits(ESP_NETIF_STARTED_BIT, 2000)) {
    Serial.println("[NAT-AP] ERROR: AP did not start");
    return false;
  }

  Serial.println("[NAT-AP] Forcing STA as default Internet route...");

  if (!WiFi.STA.setDefault()) {
    Serial.println("[NAT-AP] ERROR: could not set STA as default route");
    return false;
  }

  delay(250);

  if (!WiFi.AP.enableNAPT(true)) {
    Serial.println("[NAT-AP] ERROR: NAPT could not be enabled");
    return false;
  }

  Serial.println("[NAT-AP] STA is default route");

  Serial.println("[NAT-AP] ==================================");
  Serial.printf("[NAT-AP] SSID:    %s\n", PROTECTED_AP_SSID);
  Serial.printf("[NAT-AP] AP IP:   %s\n", protectedIP.toString().c_str());
  Serial.printf("[NAT-AP] DHCP:    %s+\n", protectedLeaseStart.toString().c_str());
  Serial.printf("[NAT-AP] DNS:     %s\n", protectedDNS.toString().c_str());
  Serial.println("[NAT-AP] NAPT:    ENABLED");
  Serial.println("[NAT-AP] ==================================");

  protectedNatAPEnabled = true;

  return true;
}

bool isProtectedNatAPEnabled() {
  return protectedNatAPEnabled;
}

bool recoverProtectedNatAP() {

  if (!protectedNatAPEnabled) {
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[NAT-AP] Recovery postponed: upstream STA not connected");
    return false;
  }

  Serial.println("[NAT-AP] Recovering protected Internet route...");

  if (!WiFi.STA.setDefault()) {
    Serial.println("[NAT-AP] ERROR: recovery could not set STA as default route");
    return false;
  }

  if (!WiFi.AP.enableNAPT(true)) {
    Serial.println("[NAT-AP] ERROR: recovery could not enable NAPT");
    return false;
  }

  Serial.println("[NAT-AP] Recovery complete: STA default + NAPT enabled");

  return true;
}
