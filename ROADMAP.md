# ESP32 AdBlock Gateway — Master Roadmap

This roadmap defines the path for this fork to evolve from a DNS sinkhole with an optional protected SoftAP into a small **privacy and network-policy gateway powered by ESP32**.

The project remains derived from [`s60sc/ESP32_AdBlocker`](https://github.com/s60sc/ESP32_AdBlocker) and keeps the upstream DNS sinkhole as its foundation. The fork's independent direction is centered on two deployment modes, client awareness, per-device policies, observability and appliance-like setup.

## Product direction

**ESP32 AdBlock Gateway** should support two first-class network modes:

1. **Router DNS mode** — the existing router provides DHCP and gateway; the ESP32 provides DNS filtering.
2. **ESP32 Gateway mode** — the ESP32 protected SoftAP provides DHCP + DNS + IPv4 gateway + NAPT over an upstream Wi-Fi connection.

The long-term differentiator is not simply a larger blocklist. It is a lightweight **Policy Engine** that can answer:

> Which device is asking for this domain, which profile applies, and should this request be allowed or blocked now?

---

## Status legend

- ✅ Done / validated
- 🟡 In progress / next
- ⬜ Planned
- 🔬 Research / validate before implementation

---

## F0 — Gateway foundation ✅

**Goal:** establish a safe, working fork with two usable deployment patterns.

Delivered:

- upstream ESP32_AdBlocker v3.4 retained as DNS sinkhole foundation
- protected SoftAP mode
- separate protected subnet (`192.168.4.0/24` by default)
- ESP32 DHCP in protected-gateway mode
- ESP32 DNS for protected clients
- IPv4 forwarding through NAPT
- STA default-route handling
- automatic NAPT/default-route recovery after upstream Wi-Fi reconnect
- local-only protected AP secrets (`natSecrets.h`)
- clone-safe fallback configuration
- multilingual documentation
- hardware validation on ESP32-S3 N16R8 / Arduino-ESP32 3.3.11

Known limitations carried forward:

- protected AP does not yet behave as a fully independent offline router when booting without upstream connectivity
- IPv6 policy/leak behavior is not yet solved
- encrypted client DNS (DoH/DoT) can bypass local DNS policy
- Gateway mode is not intended to replace a high-throughput Wi-Fi router

---

## F1 — Mode Selector + Setup Wizard ⬜

**Goal:** remove source-code editing from normal installation and make the two network modes explicit.

Deliverables:

- web UI selector: `Router DNS` / `ESP32 Gateway`
- persistent operating-mode setting
- setup wizard for upstream Wi-Fi
- protected SSID/password configuration from UI
- subnet selection with overlap checks
- clear reboot/apply flow
- current mode visible on status page
- safe transition logic between modes
- warning against running two DHCP servers on the same LAN

**Exit criteria:** a new user can install and choose either supported mode without editing `.h` files.

---

## F2 — Device Registry 🟡 NEXT

**Goal:** know which clients are connected and provide a stable identity for policy decisions.

Initial implementation target: **Gateway mode**, where the ESP32 controls the protected DHCP/AP environment.

Device model:

```text
Device
├── id
├── mac
├── ipv4
├── hostname
├── display_name
├── first_seen
├── last_seen
├── connected
└── profile_id
```

Deliverables:

- enumerate connected SoftAP clients
- capture MAC + assigned IPv4
- capture hostname when available
- stable registry keyed primarily by MAC
- first-seen / last-seen timestamps
- online/offline state
- user-editable friendly name
- persistent registry with bounded storage
- API/data structure reusable by dashboard and Policy Engine

**First engineering task:** prove reliable `MAC -> IPv4 -> hostname` correlation on the ESP32-S3 without destabilizing DHCP/DNS/NAPT.

**Exit criteria:** the web UI can list protected clients with stable identity across reconnects/reboots.

---

## F3 — Per-device profiles ⬜

**Goal:** assign different DNS policies to different devices.

Initial built-in profiles:

- `Standard` — ads + trackers
- `IoT Strict` — ads + trackers + analytics/telemetry categories
- `Unrestricted` — minimal security blocklist only

Deliverables:

- `Profile` data model
- device-to-profile assignment
- default profile for unknown devices
- profile editor
- per-device allow/block overrides
- inheritance rules: global -> profile -> device
- safe fallback if a profile is deleted or corrupted

**Exit criteria:** two clients on the same protected AP can receive different DNS decisions for the same domain.

---

## F4 — Policy Engine + categorized lists ⬜

**Goal:** decouple DNS resolution from policy evaluation.

Target flow:

```text
DNS Query
   |
   v
Client Resolver
(MAC/IP -> Device)
   |
   v
Policy Resolver
(Global + Profile + Device + Time)
   |
   +--> ALLOW
   |
   +--> BLOCK
   |
   +--> OVERRIDE
   v
DNS response
```

Categories may include:

- ads
- trackers
- analytics
- telemetry
- malware/phishing
- adult content
- gambling
- social media (optional policy category)

Deliverables:

- normalized policy-decision contract
- category-aware list storage/indexing
- deterministic rule precedence
- allowlist/blocklist overrides
- reason code for every block decision
- memory/latency benchmarks on N16R8

**Exit criteria:** every block decision can report `device + profile + category + rule/reason`.

---

## F5 — Schedules + Quick Controls ⬜

**Goal:** make policies time-aware and easy to override temporarily.

Deliverables:

- profile schedules
- per-device schedules
- temporary `Pause filtering`
- temporary `Pause Internet` where technically safe and enforceable
- one-tap profile switch
- configurable expiry for temporary overrides
- correct behavior across reboot/time-sync failure

Example:

```text
Family profile
07:00-21:00  normal policy
21:00-07:00  restricted policy
```

**Exit criteria:** a scheduled rule changes the effective policy automatically and recovers correctly after reboot.

---

## F6 — Dashboard + Observability ⬜

**Goal:** show what the gateway is doing without turning the ESP32 into a heavy analytics server.

Dashboard targets:

- Internet/uplink status
- active network mode
- protected AP status
- connected clients
- DNS queries today
- blocked queries today
- block percentage
- top blocked domains
- top devices by queries/blocks
- per-device recent summary
- NAPT/uplink recovery events
- memory/heap/PSRAM health

Architecture rule: use bounded counters/ring buffers and avoid unbounded query logs.

**Exit criteria:** users can diagnose normal operation and identify which device is generating blocked traffic.

---

## F7 — Guest + IoT network policies 🔬

**Goal:** make the ESP32 especially useful for Smart TVs and IoT devices.

Research/deliverables:

- `IoT Lockdown` profile
- guest profile
- optional client-to-client isolation if supported reliably by the networking stack
- device onboarding workflow
- QR code for protected Wi-Fi credentials
- optional separate policy defaults for newly seen devices

Do not claim L2 isolation until it is verified on the target Arduino-ESP32/ESP-IDF stack.

**Exit criteria:** a new IoT device can be onboarded quickly and automatically receives an intentionally restrictive DNS policy.

---

## F8 — DNS privacy + leak controls 🔬

**Goal:** reduce easy policy bypass while staying realistic about ESP32 resources.

Research/deliverables:

- encrypted upstream DNS feasibility (DoT and/or DoH)
- configurable upstream resolvers
- DNS failover
- IPv6 strategy for protected clients
- detection/documentation of client DoH/DoT bypass
- optional known-DoH endpoint policy, subject to memory/maintenance cost
- clear privacy model and limitations

**Exit criteria:** publish a tested, explicit DNS/IPv6 threat model and implement only features that are stable within ESP32 resource limits.

---

## F9 — Appliance reliability + OTA ⬜

**Goal:** make the project feel like a small network appliance instead of a development sketch.

Deliverables:

- guided first-run experience
- OTA firmware update path
- configuration backup/restore
- factory-reset flow that preserves explicit user intent
- health checks and watchdog strategy
- safer recovery after failed configuration
- version display
- migration of config schema between releases
- release notes/changelog

**Exit criteria:** routine updates do not require wiping user configuration or recompiling local secrets.

---

## F10 — Productization + ecosystem ⬜

**Goal:** make releases reproducible and easy for others to test, contribute and deploy.

Deliverables:

- semantic release/versioning policy for the fork
- compatibility matrix by ESP32 board / flash / PSRAM
- reproducible build instructions
- CI compile checks for supported boards
- release binaries where license/build constraints permit
- issue/bug templates
- architecture and contribution guide
- hardware validation checklist
- performance benchmarks for Router DNS vs Gateway mode
- upstream-sync strategy

Potential release naming:

```text
v0.1  Gateway foundation
v0.2  Mode Selector + Device Registry
v0.3  Per-device Policy Engine
v0.4  Dashboard + schedules
v1.0  Stable appliance-oriented release
```

---

# Architecture principles

1. **DNS filtering remains useful without Gateway mode.** Router DNS mode must remain lightweight.
2. **Gateway-only capabilities degrade gracefully.** Device-aware features may be richer when ESP32 owns DHCP/AP.
3. **No competing DHCP servers on one LAN.** ESP32 DHCP belongs to its own protected subnet.
4. **Bound memory use.** No unbounded logs, client histories or rule sets.
5. **Policy decisions must be explainable.** Every block should be attributable to a category/rule/profile where possible.
6. **Secrets stay local.** Never require real Wi-Fi credentials in Git.
7. **Do not overclaim security.** DNS filtering is not a firewall, endpoint security suite or complete parental-control system.
8. **Preserve upstream attribution and AGPL-3.0 obligations.**

# Immediate next sprint

The next development sprint is **F2 Device Registry**, preceded only by the minimum F1 configuration abstractions needed to avoid hard-coding new behavior.

Recommended implementation order:

```text
F2.1 Connected-client enumeration
   -> F2.2 MAC/IP correlation
   -> F2.3 Hostname discovery
   -> F2.4 Persistent registry
   -> F2.5 Web/API device list
   -> F2.6 Friendly names
   -> F2.7 Profile field placeholder
```

See [`docs/POLICY_ENGINE.md`](docs/POLICY_ENGINE.md) for the proposed internal architecture.
