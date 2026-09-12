#pragma once

#include <Arduino.h>

#include "domainCategories.h"

constexpr const char* CATEGORY_INDEX_PATH = "/data/domain-categories.txt";
constexpr size_t CATEGORY_INDEX_MAX_ENTRIES = 8192;

struct CategoryIndexStats {
  bool loaded = false;
  size_t entries = 0;
  size_t uniqueDomains = 0;
  size_t sourceBytes = 0;
  size_t storageBytes = 0;
  size_t entryBytes = 0;
  uint32_t loadMs = 0;
};

// Load a sorted, bounded category index from LittleFS/STORAGE.
// File format: domain|category[,category...]
// Example: ads.example|ads,trackers
bool categoryIndexLoad(const char* path = CATEGORY_INDEX_PATH);

void categoryIndexClear();
bool categoryIndexLoaded();
CategoryIndexStats categoryIndexStats();

// Exact-domain lookup only in F4.2. Parent/suffix semantics are deferred until
// the category source format and expected matching rules are validated.
DomainCategoryMask categoryIndexLookup(const char* domainName);

// Human-readable parser used by source ingestion and diagnostics.
DomainCategoryMask categoryIndexParseMask(const char* categoryList);
