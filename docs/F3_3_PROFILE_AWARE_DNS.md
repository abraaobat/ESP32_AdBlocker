# F3.3 — Profile-aware DNS Decisions

F3.3 is the first phase where the attributed device profile changes the DNS decision path.

## Current behavior

The DNS request already carries a `DnsClientContext` from F3.2:

```text
source IPv4 -> online Device Registry entry -> MAC -> persistent profile
```

F3.3 adds a small profile dispatcher before the existing DNS result construction.

### Standard

Uses the existing global blocklist behavior unchanged.

```text
profile=standard -> checkBlocklist(domain)
```

### IoT Strict

For this first enforcement milestone it also uses the existing global blocklist.

```text
profile=iot-strict -> checkBlocklist(domain)
```

The stricter telemetry / analytics semantics require F4 domain categorization. F3.3 must not pretend that the current monolithic blocklist can distinguish those categories.

### Unrestricted

Bypasses the current monolithic blocklist and resolves directly through the existing upstream resolver:

```text
profile=unrestricted -> resolveDomainStatus(domain)
```

This gives F3.3 a real, hardware-testable per-device policy difference for the same queried domain.

## Important safety boundary

The current blocklist does not distinguish ads/trackers from malware/phishing or user custom entries. Therefore `unrestricted` in F3.3 means **no current blocklist enforcement** for that device. It must not be described as providing minimal safety filtering yet.

F4 must add category-aware policy before the intended long-term `Unrestricted = minimal safety filtering only` semantics can be claimed.

## Fallback

If a DNS source cannot be attributed to an active Device Registry entry, the request uses the existing Standard blocklist path.

This preserves current behavior for router-DNS mode, incomplete DHCP state, or unknown clients.

## Acceptance test

Use two simultaneous protected-AP clients with different profiles and query a domain known to be in the current blocklist, for example `doubleclick.net`.

Expected:

```text
standard client     -> 0.0.0.0
unrestricted client -> real upstream address
```

Then return the second client to `iot-strict` and confirm it again receives `0.0.0.0`.

Also verify a normal domain such as `example.com` resolves for all profiles.

## Non-goals

F3.3 does not yet implement:

- domain categories
- telemetry/analytics-specific IoT rules
- per-profile custom allow/block lists
- schedules
- malware/phishing safety category
- query-history persistence

Those belong to later Policy Engine phases.
