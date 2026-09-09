# F4.1 — Domain Category Model

F4.1 introduces the compact category and profile-policy contracts required before category data is attached to the DNS path.

## Categories

The initial category mask uses a bounded `uint16_t` bitset:

- `ads`
- `trackers`
- `analytics`
- `telemetry`
- `malware`
- `phishing`
- `unknown`

A domain may carry more than one category bit.

## Built-in profile matrix

| Category | Standard | IoT Strict | Unrestricted |
|---|---:|---:|---:|
| Ads | Block | Block | Allow |
| Trackers | Block | Block | Allow |
| Analytics | Allow | Block | Allow |
| Telemetry | Allow | Block | Allow |
| Malware | Block | Block | Block |
| Phishing | Block | Block | Block |
| Unknown / none | No category decision | No category decision | No category decision |

`Unrestricted` therefore means unrestricted for privacy categories, not unrestricted for safety categories, once a real safety-category source exists.

## Safety boundary

This milestone does **not** classify live DNS domains yet and does not claim malware/phishing protection from categorized data. The current DNS path remains unchanged until F4.2 provides a real category index/source and F4.3 integrates it.

Unknown or uncategorized domains deliberately produce no category block decision. Existing/legacy DNS behavior remains responsible for them during migration.

## API

`domainCategories.*` defines the category bitmask and helpers.

`profileCategoryPolicy.*` defines the fixed starter profile matrix:

```cpp
DomainCategoryMask profileCategoryBlockMask(const char* profileId);
bool profileBlocksCategories(const char* profileId, DomainCategoryMask categories);
```

Unknown profile IDs safely fall back to the Standard policy mask.

## Next: F4.2

Benchmark and implement the category index. Preferred first candidate from `docs/POLICY_ENGINE.md` is one merged sorted domain index with compact category metadata, because it avoids multiplying full domain-string storage across category lists.

Before DNS integration, measure at least:

- PSRAM use
- free internal heap
- index load time
- lookup p50/p95/p99
- DNS behavior with two simultaneous clients
- NAPT stability

Refs #21
