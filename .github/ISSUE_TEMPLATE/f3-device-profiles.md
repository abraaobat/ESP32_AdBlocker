---
name: F3 Device Profiles
about: Track per-device DNS policy profiles
title: "F3.x — Device Profiles: "
labels: ""
assignees: ""
---

## Goal

Advance F3 — Per-device profiles from the Master Roadmap.

## Scope

- [ ] profile data model
- [ ] default profile for unknown devices
- [ ] Standard / IoT Strict / Unrestricted presets
- [ ] device-to-profile assignment
- [ ] per-device overrides
- [ ] deterministic precedence
- [ ] persistence and safe fallback

## Acceptance criteria

Two clients on the same protected AP can receive different DNS decisions for the same test domain according to their assigned profiles.

## Dependencies

- F2 Device Registry must provide reliable client identity.
- Policy behavior must preserve global DNS filtering if device identity is unavailable.

## Roadmap references

- `ROADMAP.md` — F3 Per-device profiles
- `docs/POLICY_ENGINE.md` — Profiles / Rule precedence
