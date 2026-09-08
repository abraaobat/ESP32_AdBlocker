---
name: F2 Device Registry
about: Track work for connected-client identification and registry
title: "F2.x — Device Registry: "
labels: ""
assignees: ""
---

## Goal

Advance F2 — Device Registry from the Master Roadmap.

## Problem

Describe the client-identification problem being solved. Prefer Gateway-mode observations where the ESP32 controls SoftAP + DHCP.

## Scope

- [ ] connected-client enumeration
- [ ] MAC/IP correlation
- [ ] hostname discovery if available
- [ ] reconnect behavior
- [ ] bounded memory/storage behavior
- [ ] no regression to DNS/DHCP/NAPT

## Test environment

- Board:
- Flash / PSRAM:
- Arduino-ESP32 version:
- Upstream Wi-Fi/router:
- Number of clients:

## Acceptance criteria

Define observable criteria that can be reproduced on hardware.

## Evidence

Attach serial logs, command output, screenshots or memory/latency measurements without exposing Wi-Fi credentials.

## Roadmap references

- `ROADMAP.md` — F2 Device Registry
- `docs/POLICY_ENGINE.md` — Device Registry / Client Resolver
