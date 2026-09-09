#pragma once

#include <Arduino.h>

using DomainCategoryMask = uint16_t;

enum DomainCategory : DomainCategoryMask {
  DOMAIN_CATEGORY_NONE       = 0,
  DOMAIN_CATEGORY_ADS        = 1u << 0,
  DOMAIN_CATEGORY_TRACKERS   = 1u << 1,
  DOMAIN_CATEGORY_ANALYTICS  = 1u << 2,
  DOMAIN_CATEGORY_TELEMETRY  = 1u << 3,
  DOMAIN_CATEGORY_MALWARE    = 1u << 4,
  DOMAIN_CATEGORY_PHISHING   = 1u << 5,
  DOMAIN_CATEGORY_UNKNOWN    = 1u << 6,
};

constexpr DomainCategoryMask DOMAIN_CATEGORY_SAFETY =
  DOMAIN_CATEGORY_MALWARE | DOMAIN_CATEGORY_PHISHING;

constexpr DomainCategoryMask DOMAIN_CATEGORY_PRIVACY =
  DOMAIN_CATEGORY_ADS | DOMAIN_CATEGORY_TRACKERS |
  DOMAIN_CATEGORY_ANALYTICS | DOMAIN_CATEGORY_TELEMETRY;

bool domainCategoryMaskHas(DomainCategoryMask mask, DomainCategory category);
const char* domainCategoryName(DomainCategory category);
