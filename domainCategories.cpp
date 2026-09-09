#include "domainCategories.h"

bool domainCategoryMaskHas(DomainCategoryMask mask, DomainCategory category) {
  return (mask & static_cast<DomainCategoryMask>(category)) != 0;
}

const char* domainCategoryName(DomainCategory category) {
  switch (category) {
    case DOMAIN_CATEGORY_ADS:
      return "ads";
    case DOMAIN_CATEGORY_TRACKERS:
      return "trackers";
    case DOMAIN_CATEGORY_ANALYTICS:
      return "analytics";
    case DOMAIN_CATEGORY_TELEMETRY:
      return "telemetry";
    case DOMAIN_CATEGORY_MALWARE:
      return "malware";
    case DOMAIN_CATEGORY_PHISHING:
      return "phishing";
    case DOMAIN_CATEGORY_UNKNOWN:
      return "unknown";
    case DOMAIN_CATEGORY_NONE:
    default:
      return "none";
  }
}
