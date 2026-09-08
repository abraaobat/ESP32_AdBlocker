# F2.5c — Devices navigation integration

Goal: expose the validated Devices dashboard from the main ESP32 AdBlocker navigation without changing Device Registry, DNS, DHCP, or NAPT behavior.

Planned UI change:

- add a `Devices` button to the existing top navigation in `data/AdBlocker.htm`
- route to `/web?Devices.htm`
- keep `Devices.htm` read-only
- preserve the existing `Back to dashboard` link

Validation:

- main dashboard loads normally
- `Devices` opens `/web?Devices.htm`
- page works via both `192.168.100.95` and `192.168.4.1`
