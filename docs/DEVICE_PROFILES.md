# Device Profiles

F3 introduces persistent profile assignments on top of the Device Registry and Friendly Device Names.

## Built-in profiles

- `standard` — current/default filtering behavior
- `iot-strict` — reserved for stricter IoT telemetry/analytics filtering
- `unrestricted` — reserved for reduced optional filtering while retaining resolver safety behavior

## Identity and persistence

Assignments are keyed by MAC and stored in:

```text
/data/device-profiles.txt
```

Unknown devices and invalid/missing assignments always resolve to `standard`.

The profile store is intentionally separate from the transient Device Registry. A reboot may reset registry timing counters, but the profile assignment remains.

## API

`GET /api/devices` exposes:

```json
{
  "profiles": ["standard", "iot-strict", "unrestricted"],
  "devices": [
    {
      "mac": "AA:BB:CC:DD:EE:FF",
      "name": "Living Room TV",
      "profile": "iot-strict"
    }
  ]
}
```

## UI

The Devices page presents a profile selector per device. Assignment is configuration metadata at this stage.

## Important boundary

Profile assignment does **not** yet mean per-device DNS enforcement is active.

The next F3 step must correlate each DNS request source IPv4 to the current Device Registry entry and then resolve the effective profile in the DNS decision path. Until that attribution path is implemented and validated, the global blocklist remains authoritative.

## Validation targets

1. select a profile for a device
2. assignment appears in `/api/devices`
3. assignment survives reboot
4. unknown device falls back to `standard`
5. no DNS/DHCP/NAPT regression
6. later: two simultaneous clients receive different DNS decisions for the same domain
