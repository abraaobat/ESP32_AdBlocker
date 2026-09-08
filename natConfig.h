#pragma once

// Public defaults for the optional protected SoftAP.
// Override PROTECTED_AP_SSID in natSecrets.h if desired.
static const char* PROTECTED_AP_DEFAULT_SSID = "ESP32-AdBlock";

#if __has_include("natSecrets.h")
#include "natSecrets.h"
#define PROTECTED_AP_HAS_LOCAL_SECRETS 1
#else
// Keep fresh clones buildable without publishing a real password.
static const char* PROTECTED_AP_PASS = "CHANGE_ME";
#define PROTECTED_AP_HAS_LOCAL_SECRETS 0
#endif

#ifndef PROTECTED_AP_SSID
#define PROTECTED_AP_SSID PROTECTED_AP_DEFAULT_SSID
#endif
