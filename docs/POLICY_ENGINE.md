# ESP32 AdBlock Gateway — Policy Engine Architecture

This document defines the proposed architecture for device-aware DNS policies in the ESP32 AdBlock Gateway fork.

The design must work within ESP32-S3 resource limits, preserve the existing DNS sinkhole path, and avoid turning the firmware into a general-purpose router/firewall it cannot reliably be.

## 1. Architectural goal

Transform a DNS request from:

```text
Domain -> Blocklist -> Allow/Block
```

into:

```text
Client -> Device -> Profile -> Policy -> Domain Category -> Decision
```

while keeping the decision path fast and memory-bounded.

## 2. Major components

```text
+------------------------------------------------------+
|                    Web UI / API                      |
| setup | devices | profiles | dashboard | diagnostics|
+--------------------------+---------------------------+
                           |
                           v
+------------------------------------------------------+
|                  Configuration Layer                 |
| network mode | AP | profiles | schedules | overrides|
+--------------------------+---------------------------+
                           |
             +-------------+-------------+
             |                           |
             v                           v
+-------------------------+   +-------------------------+
|     Device Registry     |   |      Policy Store       |
| MAC/IP/host/name/state  |   | profiles/rules/schedule |
+------------+------------+   +------------+------------+
             |                             |
             +-------------+---------------+
                           |
                           v
+------------------------------------------------------+
|                   Policy Resolver                    |
| client -> device -> profile -> category -> decision  |
+--------------------------+---------------------------+
                           |
                           v
+------------------------------------------------------+
|                    DNS Pipeline                      |
| parse -> policy -> allow/resolve OR block/0.0.0.0    |
+------------------------------------------------------+
                           |
                           v
+------------------------------------------------------+
|              Counters / Bounded Telemetry            |
| device stats | category stats | reason codes | health|
+------------------------------------------------------+
```

## 3. Network-mode boundary

### Router DNS mode

The ESP32 receives DNS requests but may not reliably know the client's MAC address because the client is not directly attached to the ESP32 AP/DHCP environment.

Therefore:

- DNS filtering must continue to work globally.
- Device-aware policy may be limited to source IPv4 where observable.
- Persistent identity should not assume MAC visibility.
- The UI must clearly indicate when a feature requires Gateway mode.

### ESP32 Gateway mode

The ESP32 controls the protected SoftAP and DHCP environment.

This is the primary target for device-aware policy because it can correlate:

- SoftAP station MAC
- DHCP-assigned IPv4
- hostname when available
- connection/disconnection events
- DNS source IPv4

## 4. Device Registry

Suggested bounded structure:

```cpp
struct DeviceRecord {
  uint8_t mac[6];
  IPAddress ipv4;
  char hostname[33];
  char displayName[33];
  uint32_t firstSeen;
  uint32_t lastSeen;
  uint16_t profileId;
  bool connected;
};
```

The exact representation should be benchmarked before adoption.

### Identity rules

1. MAC is the preferred stable identity in Gateway mode.
2. IPv4 is an attachment attribute, not the permanent identity.
3. Hostname is metadata and may be absent, duplicated or misleading.
4. Friendly name is user-owned metadata.
5. Unknown clients receive the configured default profile.

### Storage rules

- fixed maximum number of persistent devices
- explicit eviction policy for stale unknown devices
- user-named devices should not be silently evicted
- no unbounded per-device query history
- persist only configuration/state required after reboot

## 5. Client Resolver

DNS packets provide a source IP. The Client Resolver maps this source to a Device Registry entry.

Target flow:

```text
DNS source IPv4
      |
      v
lookup active IP -> device
      |
      +-- found -> stable DeviceId
      |
      +-- missing -> anonymous/default client context
```

In Gateway mode, DHCP/AP events continuously refresh the `MAC <-> IPv4` mapping.

The resolver must never make DNS unavailable merely because device identity is incomplete.

## 6. Profiles

Suggested conceptual model:

```text
Profile
├── id
├── name
├── category_mask
├── schedule_id
├── allow_overrides[]
├── block_overrides[]
└── flags
```

Built-in starter profiles:

### Standard

- ads: block
- trackers: block
- malware/phishing: block
- other categories: allow

### IoT Strict

- ads: block
- trackers: block
- analytics: block
- telemetry: block
- malware/phishing: block

### Unrestricted

- malware/phishing: block
- optional minimal safety rules
- ads/trackers: allow

Profiles should be editable later, but early implementations can start with fixed presets to validate the architecture.

## 7. Rule precedence

Recommended deterministic precedence, highest first:

```text
1. Device explicit allow
2. Device explicit block
3. Profile explicit allow
4. Profile explicit block
5. Schedule override
6. Category policy
7. Global custom allow/block behavior
8. Default decision
```

Before implementation, this order should be reconciled with the semantics of the existing custom blocklist so users are not surprised by regressions.

Every decision should return both an action and a reason.

Example:

```cpp
enum class PolicyAction {
  Allow,
  Block
};

enum class PolicyReason {
  DeviceAllow,
  DeviceBlock,
  ProfileAllow,
  ProfileBlock,
  CategoryBlock,
  GlobalBlocklist,
  DefaultAllow
};

struct PolicyDecision {
  PolicyAction action;
  PolicyReason reason;
  uint16_t profileId;
  uint16_t categoryId;
};
```

## 8. Category index

The current project is optimized around a DNS blocklist. Categorization must not multiply memory use blindly.

Possible strategies to benchmark:

1. one merged domain index plus compact category bitmask
2. multiple smaller sorted lists
3. hashed domain index plus category metadata
4. compile/download-time category consolidation

For N16R8, target metrics should include:

- total PSRAM consumption
- heap fragmentation
- lookup latency p50/p95/p99
- blocklist load time
- DNS throughput under concurrent clients

Do not choose a format solely for implementation simplicity.

## 9. Schedules

Schedules should be evaluated without expensive dynamic allocation.

Suggested model:

```text
Schedule
├── weekday mask
├── start minute
├── end minute
└── policy/profile override
```

Important cases:

- overnight windows crossing midnight
- time unavailable before NTP synchronization
- timezone / daylight-saving behavior
- reboot during an active schedule

Fail-safe behavior must be explicit and configurable where appropriate.

## 10. Dashboard telemetry

Keep telemetry aggregate and bounded.

Recommended counters:

```text
Global
- dns_total
- dns_blocked
- dns_allowed
- napt_recoveries
- uplink_disconnects

Per device (bounded)
- dns_total
- dns_blocked
- last_query_time

Per category
- blocked_count
```

Optional top-domain statistics should use a bounded heavy-hitter strategy or small fixed table rather than storing every query.

## 11. Performance guardrails

The Policy Engine must not break the primary purpose of the firmware: answering DNS reliably.

Initial targets for ESP32-S3 N16R8 should be measured, not assumed. Track:

- DNS lookup latency before/after Policy Engine
- free internal heap
- free PSRAM
- maximum connected clients tested
- sustained queries per second
- NAPT stability during DNS load

Any feature that materially destabilizes routing or DNS should be disabled or simplified.

## 12. Security and privacy model

The Device Registry and dashboard are local network-management features.

Rules:

- do not upload device identities or query history to external services by default
- do not persist complete browsing/query histories by default
- keep Wi-Fi credentials local
- expose administrative UI only according to the existing project's access/security model
- clearly state that MAC addresses and hostnames are identifiers but are not strong authentication

The project is a DNS policy gateway, not a substitute for a stateful security firewall or endpoint protection.

## 13. F2 implementation spike

The first engineering spike should answer only these questions:

1. Which Arduino-ESP32 APIs reliably enumerate SoftAP stations on the current core?
2. Can we obtain station MAC and IPv4 without unsafe internal APIs?
3. How do DHCP lease events expose or correlate hostnames, if at all?
4. Does enumeration remain stable across reconnect, roaming-like reconnect and upstream loss?
5. What happens with clients using randomized/private MAC addresses?
6. How much memory does a registry of 8, 16, 32 and 64 clients consume?

### Minimal F2 proof of concept

```text
[Devices]

MAC                IPv4           Hostname       State
AA:BB:CC:DD:EE:01  192.168.4.10   iphone         online
AA:BB:CC:DD:EE:02  192.168.4.11   samsung-tv     online
AA:BB:CC:DD:EE:03  192.168.4.12   -              offline
```

No per-device blocking should be added until identity correlation is reliable.

## 14. Proposed module boundaries

As the code grows, prefer explicit modules over adding more logic to `utils.cpp`.

Potential layout:

```text
network/
  networkMode.*
  protectedGateway.*

devices/
  deviceRegistry.*
  clientResolver.*

policy/
  policyEngine.*
  policyStore.*
  categories.*
  schedules.*

telemetry/
  counters.*

ui/
  device endpoints/pages
  profile endpoints/pages
```

Migration should be incremental; do not reorganize the entire upstream codebase before a feature requires it.

## 15. Definition of success

The Policy Engine architecture is successful when the firmware can answer:

```text
Why was this DNS request blocked?
```

with a bounded, deterministic result such as:

```text
Device: Samsung TV
Profile: IoT Strict
Domain: telemetry.vendor.example
Category: Telemetry
Decision: BLOCK
Reason: Profile category policy
```

without compromising the stability of DNS, DHCP, SoftAP or NAPT.
