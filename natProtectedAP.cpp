#include <Arduino.h>
#include <WiFi.h>
#include "appGlobals.h"
#include "natSecrets.h"

namespace {

const char* PROTECTED_AP_SSID = "AMB-AdBlock";

// TROQUE esta senha antes do upload.
// Mínimo 8 caracteres.


IPAddress protectedIP(192, 168, 4, 1);
IPAddress protectedMask(255, 255, 255, 0);
IPAddress protectedLeaseStart(192, 168, 4, 10);
IPAddress protectedDNS(192, 168, 4, 1);

}

bool startProtectedNatAP() {

  Serial.println();
  Serial.println("[NAT-AP] Starting protected Wi-Fi...");

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

  return true;
}
