#include <cstring>

#include "deviceProfiles.h"
#include "profileCategoryPolicy.h"

namespace {

constexpr DomainCategoryMask STANDARD_BLOCK_MASK =
  DOMAIN_CATEGORY_ADS |
  DOMAIN_CATEGORY_TRACKERS |
  DOMAIN_CATEGORY_MALWARE |
  DOMAIN_CATEGORY_PHISHING;

constexpr DomainCategoryMask IOT_STRICT_BLOCK_MASK =
  STANDARD_BLOCK_MASK |
  DOMAIN_CATEGORY_ANALYTICS |
  DOMAIN_CATEGORY_TELEMETRY;

constexpr DomainCategoryMask UNRESTRICTED_BLOCK_MASK =
  DOMAIN_CATEGORY_MALWARE |
  DOMAIN_CATEGORY_PHISHING;

}  // namespace

DomainCategoryMask profileCategoryBlockMask(const char* profileId) {
  if (!profileId) return STANDARD_BLOCK_MASK;

  if (!strcmp(profileId, DEVICE_PROFILE_IOT_STRICT)) {
    return IOT_STRICT_BLOCK_MASK;
  }

  if (!strcmp(profileId, DEVICE_PROFILE_UNRESTRICTED)) {
    return UNRESTRICTED_BLOCK_MASK;
  }

  return STANDARD_BLOCK_MASK;
}

bool profileBlocksCategories(const char* profileId, DomainCategoryMask categories) {
  if (categories == DOMAIN_CATEGORY_NONE || categories == DOMAIN_CATEGORY_UNKNOWN) {
    return false;
  }

  return (profileCategoryBlockMask(profileId) & categories) != 0;
}
