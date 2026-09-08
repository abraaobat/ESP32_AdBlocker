#pragma once

// Copy this file to natSecrets.h before enabling the protected SoftAP.
// Keep natSecrets.h private; it is ignored by Git.

// Optional: change the protected Wi-Fi name.
#define PROTECTED_AP_SSID "ESP32-AdBlock"

// Required: replace with a private WPA2 password (8..63 characters).
static const char* PROTECTED_AP_PASS = "CHANGE_ME";
