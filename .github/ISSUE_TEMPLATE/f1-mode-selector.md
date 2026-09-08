---
name: F1 Mode Selector
about: Track setup wizard and Router DNS / ESP32 Gateway mode selection
title: "F1.x — Mode Selector: "
labels: ""
assignees: ""
---

## Goal

Advance F1 — Mode Selector + Setup Wizard from the Master Roadmap.

## Scope

- [ ] explicit Router DNS / ESP32 Gateway selection
- [ ] persistent mode configuration
- [ ] web setup flow
- [ ] safe DHCP behavior
- [ ] protected SSID/password UI
- [ ] subnet overlap validation
- [ ] status page mode indicator

## Acceptance criteria

A new user can choose and activate either supported mode without editing source files.

## Safety checks

- no competing DHCP server is introduced on the upstream LAN
- credentials are never logged or committed
- switching mode has deterministic reboot/apply behavior

## Roadmap reference

- `ROADMAP.md` — F1 Mode Selector + Setup Wizard
