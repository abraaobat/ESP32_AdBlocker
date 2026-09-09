#include <Arduino.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "appGlobals.h"
#include "categoryIndex.h"

namespace {

struct CategoryIndexEntry {
  const char* domain = nullptr;
  DomainCategoryMask mask = DOMAIN_CATEGORY_NONE;
};

CategoryIndexEntry* g_entries = nullptr;
char* g_storage = nullptr;
CategoryIndexStats g_stats;

int compareEntries(const void* a, const void* b) {
  const auto* ea = static_cast<const CategoryIndexEntry*>(a);
  const auto* eb = static_cast<const CategoryIndexEntry*>(b);
  return strcmp(ea->domain, eb->domain);
}

void normalizeDomain(char* text) {
  if (!text) return;

  while (*text && isspace((unsigned char)*text)) {
    memmove(text, text + 1, strlen(text));
  }

  size_t len = strlen(text);
  while (len && isspace((unsigned char)text[len - 1])) text[--len] = 0;
  if (len && text[len - 1] == '.') text[--len] = 0;

  if (!strncmp(text, "www.", 4)) {
    memmove(text, text + 4, strlen(text + 4) + 1);
  }

  for (char* p = text; *p; ++p) *p = (char)tolower((unsigned char)*p);
}

DomainCategoryMask tokenMask(const char* token) {
  if (!strcmp(token, "ads")) return DOMAIN_CATEGORY_ADS;
  if (!strcmp(token, "trackers")) return DOMAIN_CATEGORY_TRACKERS;
  if (!strcmp(token, "analytics")) return DOMAIN_CATEGORY_ANALYTICS;
  if (!strcmp(token, "telemetry")) return DOMAIN_CATEGORY_TELEMETRY;
  if (!strcmp(token, "malware")) return DOMAIN_CATEGORY_MALWARE;
  if (!strcmp(token, "phishing")) return DOMAIN_CATEGORY_PHISHING;
  if (!strcmp(token, "unknown")) return DOMAIN_CATEGORY_UNKNOWN;
  return DOMAIN_CATEGORY_NONE;
}

bool parseSourceLine(char* line, char*& domainOut, DomainCategoryMask& maskOut) {
  domainOut = nullptr;
  maskOut = DOMAIN_CATEGORY_NONE;
  if (!line) return false;

  while (*line && isspace((unsigned char)*line)) ++line;
  if (!*line || *line == '#') return false;

  char* sep = strchr(line, '|');
  if (!sep) return false;
  *sep = 0;
  char* categories = sep + 1;

  normalizeDomain(line);
  if (!*line) return false;

  maskOut = categoryIndexParseMask(categories);
  if (maskOut == DOMAIN_CATEGORY_NONE) return false;

  domainOut = line;
  return true;
}

}  // namespace

DomainCategoryMask categoryIndexParseMask(const char* categoryList) {
  if (!categoryList || !*categoryList) return DOMAIN_CATEGORY_NONE;

  char copy[128];
  strncpy(copy, categoryList, sizeof(copy) - 1);
  copy[sizeof(copy) - 1] = 0;

  DomainCategoryMask mask = DOMAIN_CATEGORY_NONE;
  char* save = nullptr;
  for (char* token = strtok_r(copy, ",", &save); token; token = strtok_r(nullptr, ",", &save)) {
    while (*token && isspace((unsigned char)*token)) ++token;
    size_t len = strlen(token);
    while (len && isspace((unsigned char)token[len - 1])) token[--len] = 0;
    for (char* p = token; *p; ++p) *p = (char)tolower((unsigned char)*p);
    mask |= tokenMask(token);
  }
  return mask;
}

void categoryIndexClear() {
  if (g_entries) free(g_entries);
  if (g_storage) free(g_storage);
  g_entries = nullptr;
  g_storage = nullptr;
  g_stats = CategoryIndexStats{};
}

bool categoryIndexLoad(const char* path) {
  categoryIndexClear();
  const uint32_t started = millis();

  File file = STORAGE.open(path, FILE_READ);
  if (!file) {
    LOG_WRN("Category index source not found: %s", path);
    return false;
  }

  const size_t sourceBytes = file.size();
  if (!sourceBytes) {
    file.close();
    LOG_WRN("Category index source is empty: %s", path);
    return false;
  }

  size_t candidateLines = 0;
  while (file.available() && candidateLines < CATEGORY_INDEX_MAX_ENTRIES) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() && line[0] != '#' && line.indexOf('|') > 0) ++candidateLines;
  }

  if (!candidateLines) {
    file.close();
    LOG_WRN("Category index has no valid entries: %s", path);
    return false;
  }

  g_entries = static_cast<CategoryIndexEntry*>(ps_malloc(candidateLines * sizeof(CategoryIndexEntry)));
  g_storage = static_cast<char*>(ps_malloc(sourceBytes + 1));
  if (!g_entries || !g_storage) {
    file.close();
    LOG_ERR("Category index PSRAM allocation failed entries=%u source=%u",
            (unsigned)candidateLines, (unsigned)sourceBytes);
    categoryIndexClear();
    return false;
  }

  memset(g_entries, 0, candidateLines * sizeof(CategoryIndexEntry));
  memset(g_storage, 0, sourceBytes + 1);

  if (!file.seek(0)) {
    file.close();
    categoryIndexClear();
    LOG_ERR("Category index source seek failed");
    return false;
  }

  size_t count = 0;
  size_t storageUsed = 0;
  while (file.available() && count < candidateLines) {
    String sourceLine = file.readStringUntil('\n');
    if (sourceLine.length() >= 512) continue;

    char line[512];
    sourceLine.toCharArray(line, sizeof(line));

    char* domain = nullptr;
    DomainCategoryMask mask = DOMAIN_CATEGORY_NONE;
    if (!parseSourceLine(line, domain, mask)) continue;

    const size_t len = strlen(domain);
    if (!len || storageUsed + len + 1 > sourceBytes + 1) continue;

    char* stored = g_storage + storageUsed;
    memcpy(stored, domain, len + 1);
    storageUsed += len + 1;

    g_entries[count].domain = stored;
    g_entries[count].mask = mask;
    ++count;
  }
  file.close();

  if (!count) {
    categoryIndexClear();
    LOG_WRN("Category index parsing produced zero entries");
    return false;
  }

  qsort(g_entries, count, sizeof(CategoryIndexEntry), compareEntries);

  // Merge duplicate domains in place by OR-ing category masks.
  size_t unique = 0;
  for (size_t i = 0; i < count; ++i) {
    if (unique && !strcmp(g_entries[unique - 1].domain, g_entries[i].domain)) {
      g_entries[unique - 1].mask |= g_entries[i].mask;
    } else {
      if (unique != i) g_entries[unique] = g_entries[i];
      ++unique;
    }
  }

  g_stats.loaded = true;
  g_stats.entries = count;
  g_stats.uniqueDomains = unique;
  g_stats.sourceBytes = sourceBytes;
  g_stats.storageBytes = storageUsed;
  g_stats.entryBytes = candidateLines * sizeof(CategoryIndexEntry);
  g_stats.loadMs = millis() - started;

  LOG_INF("Category index loaded entries=%u unique=%u source=%uB storage=%uB entryMem=%uB load=%ums",
          (unsigned)count,
          (unsigned)unique,
          (unsigned)sourceBytes,
          (unsigned)storageUsed,
          (unsigned)g_stats.entryBytes,
          (unsigned)g_stats.loadMs);
  return true;
}

bool categoryIndexLoaded() {
  return g_stats.loaded;
}

CategoryIndexStats categoryIndexStats() {
  return g_stats;
}

DomainCategoryMask categoryIndexLookup(const char* domainName) {
  if (!g_stats.loaded || !g_entries || !domainName || !*domainName) {
    return DOMAIN_CATEGORY_NONE;
  }

  char query[256];
  strncpy(query, domainName, sizeof(query) - 1);
  query[sizeof(query) - 1] = 0;
  normalizeDomain(query);
  if (!*query) return DOMAIN_CATEGORY_NONE;

  size_t first = 0;
  size_t last = g_stats.uniqueDomains;
  while (first < last) {
    const size_t mid = first + (last - first) / 2;
    const int cmp = strcmp(g_entries[mid].domain, query);
    if (cmp < 0) first = mid + 1;
    else last = mid;
  }

  if (first < g_stats.uniqueDomains && !strcmp(g_entries[first].domain, query)) {
    return g_entries[first].mask;
  }
  return DOMAIN_CATEGORY_NONE;
}
