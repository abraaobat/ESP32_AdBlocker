#pragma once

#include <Arduino.h>

constexpr size_t DEVICE_PROFILE_ID_MAX_LEN = 16;
constexpr const char* DEVICE_PROFILE_STANDARD = "standard";
constexpr const char* DEVICE_PROFILE_IOT_STRICT = "iot-strict";
constexpr const char* DEVICE_PROFILE_UNRESTRICTED = "unrestricted";

bool deviceProfileGet(const uint8_t mac[6], char* outProfile, size_t outSize);
bool deviceProfileSet(const uint8_t mac[6], const char* profileId);
bool deviceProfileSetFromControlValue(const char* value);
bool deviceProfileIsKnown(const char* profileId);
