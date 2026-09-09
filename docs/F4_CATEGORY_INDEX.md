# F4.2 — Category Index / Source Ingestion

This milestone introduces a bounded, PSRAM-backed category index without changing live DNS policy decisions yet.

## Goals

- ingest `domain|category[,category...]` records from local storage
- store domain strings once in a compact linear buffer
- store one compact `DomainCategoryMask` per entry
- sort once at load time
- resolve categories with binary search
- merge duplicate domain records by OR-ing category masks
- expose load/memory statistics for ESP32-S3 validation

## Source format

Default path:

```text
/data/domain-categories.txt
```

Example:

```text
doubleclick.net|ads,trackers
google-analytics.com|analytics,trackers
telemetry.vendor.example|telemetry
bad.example|malware,phishing
```

Comments start with `#`.

Supported category tokens are those defined by F4.1:

- `ads`
- `trackers`
- `analytics`
- `telemetry`
- `malware`
- `phishing`
- `unknown`

Unknown category tokens are ignored. A line that produces an empty category mask is skipped.

## Memory model

F4.2 deliberately avoids one full string list per category. The initial representation is:

```text
PSRAM
├── linear domain string storage
└── sorted CategoryIndexEntry[]
    ├── pointer to domain
    └── uint16_t category mask
```

The source is capped at `CATEGORY_INDEX_MAX_ENTRIES` (8192 for this spike).

The current loader allocates based on source size and candidate record count, then sorts and merges duplicate domain records in place. Duplicate strings may remain in the backing string buffer in F4.2; compaction can be evaluated only if measurements show it is worthwhile.

## Matching semantics

F4.2 uses **exact normalized domain matching only**.

Normalization currently:

- lowercases
- strips a trailing DNS root dot
- strips leading `www.`
- trims whitespace

Suffix/parent-domain matching is intentionally deferred. We should not assume that a category attached to `example.com` automatically applies to every subdomain until the production source format and expected semantics are defined.

## Runtime boundary

This milestone does **not** alter `externalDNS.cpp` decisions. F4.3 will integrate category lookup into the profile-aware DNS policy path after the index has been compiled and measured on hardware.

The included `data/domain-categories.txt` is a small validation fixture, not a production category or threat-intelligence feed. In particular, `.example` entries are intentionally non-production test names.

## Hardware measurements to collect

After the loader is temporarily hooked into startup for validation, record:

- `sourceBytes`
- `storageBytes`
- `entryBytes`
- `entries`
- `uniqueDomains`
- `loadMs`
- free internal heap before/after
- free PSRAM before/after
- repeated lookup time for hit and miss cases
- DNS behavior before/after loading the index
- NAPT stability with two clients connected

## Acceptance criteria

F4.2 is ready to merge when:

1. firmware compiles on ESP32-S3 N16R8 / Arduino-ESP32 3.3.11
2. fixture loads successfully from storage
3. known domains return the expected category masks
4. duplicate records merge categories correctly
5. an unknown domain returns `DOMAIN_CATEGORY_NONE`
6. loading the index does not alter existing DNS decisions
7. memory/load measurements are documented

## Next step

F4.3 will feed `categoryIndexLookup(domain)` into the profile policy resolver so Standard, IoT Strict and Unrestricted can make category-level decisions while preserving explicit reason codes and a safe legacy fallback.
