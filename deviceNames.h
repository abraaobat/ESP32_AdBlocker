#pragma once

#include <Arduino.h>

constexpr size_t DEVICE_NAME_MAX_LEN = 32;

bool deviceNameGet(const uint8_t mac[6], char* outName, size_t outSize);
bool deviceNameSet(const uint8_t mac[6], const char* name);
bool deviceNameSetFromControlValue(const char* value);
