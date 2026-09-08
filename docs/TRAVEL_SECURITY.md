# Travel Security / Public Wi-Fi

This document defines the planned **Travel Security** capability for ESP32 AdBlock Gateway.

Travel Security is a first-class use case of **ESP32 Gateway mode**. It is not a third network mode: the ESP32 still joins an upstream Wi-Fi network as a station and exposes its own protected SoftAP to the user's devices.

```text
Public / hotel / café Wi-Fi
          |
          v
   ESP32 AdBlock Gateway
   - protected SoftAP
   - DHCP + DNS
   - IPv4 NAPT
   - policy controls
          |
          v
 phone / tablet / laptop
```

## Security goals

The objective is to reduce exposure when using untrusted or semi-trusted Wi-Fi networks while preserving the lightweight nature of the ESP32 platform.

Planned goals:

- keep user devices behind the ESP32 private IPv4 subnet
- avoid exposing protected clients directly to other clients on the upstream hotspot
- keep the protected SSID/password stable while changing the upstream network
- use filtered/configurable DNS instead of blindly trusting DNS supplied by the public network
- reduce accidental leakage of local discovery/services toward the upstream network
- provide a one-tap `Travel Security` policy preset
- make captive-portal networks usable without pretending to bypass their authentication or terms
- expose clear status and limitations in the web UI

## Planned baseline controls

### Network separation

- protected SoftAP on its own subnet
- IPv4 NAPT between protected clients and the upstream Wi-Fi
- no unsolicited inbound forwarding to protected clients by default
- no automatic port forwards
- recovery behavior that restores the expected protected route after uplink reconnect

NAPT is useful isolation, but it must not be described as a complete firewall or endpoint-security product.

### DNS protection

- configurable trusted upstream DNS resolvers
- option to ignore DNS supplied by the public hotspot
- existing domain filtering and future categorized policy engine
- DNS failover
- encrypted upstream DNS (DoT/DoH) only if resource and reliability testing is acceptable
- explicit handling/documentation of client-side DoH/DoT bypass
- IPv6 leak strategy before claiming equivalent IPv6 protection

### Local-service leak reduction

Research safe controls for traffic that should normally stay local, including:

- SMB (`445/tcp`)
- NetBIOS (`137-139`)
- SSDP/UPnP (`1900/udp`)
- mDNS (`5353/udp`)
- other broadcast/multicast discovery traffic

Controls must be validated before becoming defaults because aggressive filtering can break legitimate workflows.

### Client isolation

Research client-to-client isolation on the protected SoftAP. Do not claim L2 isolation until it is verified on the target Arduino-ESP32 / ESP-IDF networking stack.

### Captive portals

Planned behavior:

1. user connects devices to the protected ESP32 SSID
2. ESP32 joins the selected public Wi-Fi uplink
3. firmware detects likely captive-portal state
4. UI provides an assisted path for the user to complete the portal normally
5. normal protected routing resumes after upstream access is available

The project must not attempt to bypass payment, authentication, access controls, or terms of service of public networks.

### Travel preset

A future `Travel Security` quick profile should prefer conservative defaults such as:

```text
Travel Security
- protected private subnet: ON
- IPv4 NAPT: ON
- unsolicited inbound forwarding: OFF
- public-hotspot DNS trust: OFF
- filtered DNS: ON
- local-service leak controls: ON where validated
- client isolation: ON only where validated
- captive-portal assistance: ON
```

## VPN research

A VPN tunnel such as WireGuard could provide stronger confidentiality between the ESP32 and a trusted VPN endpoint, but this is **research only** until throughput, RAM use, reconnect behavior and stability are measured on the supported ESP32-S3 configuration.

The project must not imply that ESP32 AdBlock Gateway provides VPN-equivalent protection unless a VPN feature is actually implemented and validated.

## Threat model

Travel Security is intended to help with:

- direct exposure of user devices to other clients on a public hotspot
- untrusted hotspot DNS configuration
- common tracking/malware/phishing domains that can be addressed by DNS policy
- accidental exposure of local network services toward the uplink
- maintaining a consistent private SSID for the user's own devices

It does **not** by itself protect against:

- phishing pages the user voluntarily visits
- malicious HTTPS content served from otherwise allowed domains
- compromised endpoints
- credential theft outside the gateway's visibility
- traffic analysis by the upstream provider
- client-controlled encrypted DNS unless separately constrained
- IPv6 leaks until an explicit IPv6 strategy is implemented

## Validation targets

Before Travel Security is considered stable, validate at minimum:

- open Wi-Fi uplink
- WPA2-Personal uplink
- reconnect/change of upstream SSID while protected clients remain configured
- captive-portal lab/test network
- protected-client Internet access through NAPT
- no unsolicited inbound reachability to protected clients
- configured DNS path does not silently fall back to hotspot DNS
- service-leak controls do not destabilize DHCP/DNS/NAPT
- documented throughput, heap/PSRAM use and failure behavior

## Relationship to the roadmap

Travel Security depends primarily on:

- **F1** — Mode Selector + Setup Wizard
- **F2** — Device Registry
- **F4/F5** — Policy Engine and quick controls
- **F7** — Guest/IoT and client-isolation research
- **F8** — DNS privacy + leak controls
- **F9** — appliance reliability

The first implementation should remain small and testable. Security claims must follow measured behavior, not precede it.
