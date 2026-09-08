# Protected SoftAP mode

This fork can expose a separate Wi-Fi network whose clients use the ESP32 as their IPv4 gateway and DNS server. The ESP32 remains connected to the upstream Wi-Fi as a station and forwards client traffic through NAPT while DNS requests continue through ESP32_AdBlocker.

## Local configuration

The protected AP password is intentionally not stored in Git.

1. Copy `natSecrets.example.h` to `natSecrets.h`.
2. Set a private password of 8 to 63 characters.
3. Optionally change `PROTECTED_AP_SSID` to the Wi-Fi name you want to expose.
4. Compile and upload normally.

`natSecrets.h` is ignored by `.gitignore` and must never be committed.

A fresh clone still compiles when `natSecrets.h` is absent. In that case the protected SoftAP is disabled and the firmware logs an instruction to create the local secrets file; the original ESP32_AdBlocker behavior remains available.

## Default protected network

Unless overridden in `natSecrets.h`:

- SSID: `ESP32-AdBlock`
- AP / gateway: `192.168.4.1`
- DHCP leases begin at `192.168.4.10`
- client DNS: `192.168.4.1`

The protected AP starts only after the upstream Wi-Fi station has connected successfully. If the upstream connection is lost after startup, the firmware keeps the protected AP and reapplies the station default route and NAPT when the upstream connection returns.

## Notes

This path currently provides IPv4 DHCP, DNS and NAPT. IPv6 DNS bypass is outside the scope of this protected-AP implementation and should be validated separately for each client environment.

The protected AP setup is optional. If `natSecrets.h` is absent or still contains the example password, protected-AP startup is refused so the example credential is never exposed as a live Wi-Fi password.
