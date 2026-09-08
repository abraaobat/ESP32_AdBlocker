# ESP32 AdBlock Gateway

**DNS-level ad blocking for ESP32, with two deployment modes: use your router's DHCP, or let the ESP32 create its own protected Wi-Fi network with DHCP + DNS + gateway + NAPT.**

> This is a modified fork of [`s60sc/ESP32_AdBlocker`](https://github.com/s60sc/ESP32_AdBlocker), based on v3.4. The original project provides the DNS sinkhole. This fork adds the optional protected SoftAP/gateway mode, uplink recovery, safer local secret handling, and additional Wi-Fi setup diagnostics.

**Languages:** English · [Português (Brasil)](README.pt-BR.md) · [Español](README.es.md) · [Français](README.fr.md) · [简体中文](README.zh-CN.md)

## What is new in this fork?

The original ESP32_AdBlocker is primarily a DNS sinkhole: clients use the ESP32 as their DNS server and blocked domains resolve to `0.0.0.0`.

This fork keeps that mode and adds a second option: the ESP32 can also expose a separate protected Wi-Fi network. Clients connected to that network receive an IP address from the ESP32, use the ESP32 as their gateway and DNS server, and reach the Internet through NAPT over the ESP32's upstream Wi-Fi connection.

This is not a new DNS or routing protocol. It is a new deployment mode that combines existing ESP32 networking capabilities into a compact ad-blocking gateway.

## Choose your network mode

| | Mode 1 — Router DHCP | Mode 2 — ESP32 Gateway DHCP |
|---|---|---|
| DHCP server | Existing router | ESP32 |
| Default gateway | Existing router | ESP32 (`192.168.4.1`) |
| DNS server | ESP32 | ESP32 (`192.168.4.1`) |
| NAT | Existing router | ESP32 NAPT → upstream router |
| Client Wi-Fi | Existing Wi-Fi | ESP32 protected SoftAP |
| Router changes | Usually set custom DNS | Usually none |
| Best for | Routers that allow custom DNS | ISP routers with locked DNS/DHCP options |

### Mode 1 — Router provides DHCP

```text
Internet
   |
Router / DHCP / Gateway
   |
   +---- ESP32 AdBlocker (DNS)
   |
   +---- Phone
   +---- Laptop
   +---- TV / IoT
```

Your router remains the DHCP server and gateway. The ESP32 only handles DNS filtering.

Example DHCP information delivered by the router:

```text
Client IP: 192.168.1.x
Gateway:   192.168.1.1
DNS:       192.168.1.95   <- ESP32
```

Use this mode when your router lets you define a LAN DNS server. It offers the best throughput because normal Internet traffic does not pass through the ESP32.

If the router cannot advertise a custom DNS server, you can configure DNS manually on individual clients, or use Mode 2.

### Mode 2 — ESP32 provides DHCP, DNS and gateway

```text
Internet
   |
Upstream router / Wi-Fi
   |
   | Wi-Fi STA
   v
ESP32
+------------------------+
| DNS sinkhole           |
| DHCP                   |
| IPv4 gateway           |
| NAPT                   |
+------------------------+
   |
   | Protected SoftAP
   v
ESP32-AdBlock
192.168.4.1
   |
   +---- Phone      192.168.4.10+
   +---- Laptop
   +---- TV / IoT
```

Clients automatically receive:

```text
IP:      192.168.4.x
Gateway: 192.168.4.1
DNS:     192.168.4.1
```

The ESP32 stays connected to the normal Wi-Fi as a station, while simultaneously creating a separate Wi-Fi network for protected clients. Internet traffic is forwarded through NAPT. DNS queries are filtered locally.

This mode is especially useful when an ISP router does not let you change the DNS handed out by DHCP.

> Do not run the ESP32 as a second DHCP server on the same LAN as your router. In this project, ESP32 DHCP belongs to the separate SoftAP subnet. Also avoid overlapping the upstream network with `192.168.4.0/24`.

## Typical use cases

- ISP-provided router with locked DNS settings.
- Home network where you want DNS-level filtering without a Raspberry Pi or dedicated server.
- Separate Wi-Fi for Smart TVs, streaming devices or IoT devices.
- Temporary protected network for travel, labs or demonstrations.
- A simple family/guest network where DNS filtering should be automatic for connected clients.
- Learning and experimenting with DNS sinkholes, ESP32 SoftAP, DHCP and NAPT.

## What it can and cannot block

DNS filtering is effective for domains dedicated to advertising, tracking and telemetry. It cannot reliably block ads served from the same domain as the desired content. Services such as YouTube are a common example where DNS-only blocking is limited.

Clients may also bypass local DNS by using encrypted DNS such as DoH/DoT. Browser **Secure DNS** should be disabled if you want DNS requests to pass through the ESP32.

The protected gateway path is currently IPv4-focused. IPv6 DNS/routing bypass should be evaluated separately in each environment.

## Hardware

Recommended:

- ESP32-S3 with PSRAM.
- 8 MB PSRAM is recommended for current large blocklists.

Hardware used to validate this fork:

- ESP32-S3 N16R8
- 16 MB flash
- 8 MB OPI PSRAM
- Arduino-ESP32 3.3.11

The upstream project supports additional ESP32 configurations; see the original project documentation for broader compatibility.

## Arduino IDE installation

### 1. Install the ESP32 core

Install **esp32 by Espressif Systems** in Arduino IDE.

This fork was validated with **3.3.11**. The upstream project documents a minimum Arduino-ESP32 core version of 3.1.1.

### 2. Clone or download the repository

```bash
git clone https://github.com/abraaobat/ESP32_AdBlocker.git
cd ESP32_AdBlocker
```

Open `ESP32_AdBlocker.ino` in Arduino IDE.

### 3. Recommended settings for ESP32-S3 N16R8

```text
Board:            ESP32S3 Dev Module
CPU Frequency:    240 MHz
Flash Size:       16MB (128Mb)
Flash Mode:       QIO
PSRAM:            OPI PSRAM
Partition Scheme: 8M with spiffs
```

For normal firmware updates, keep **Erase All Flash Before Sketch Upload** disabled so configuration and cached data are preserved.

### 4. First boot / upstream Wi-Fi setup

On first installation, the original project starts a setup AP similar to:

```text
ESP32_AdBlocker_...
```

Connect to it and open:

```text
http://192.168.4.1
```

Enter the upstream Wi-Fi SSID and password. After reboot, the ESP32 should connect to your normal network and download/process the blocklist.

## Configure Mode 1 — Router DHCP

1. Give the ESP32 a stable IPv4 address using a router DHCP reservation or static configuration.
2. Set the router's LAN/DHCP DNS server to the ESP32 address.
3. Reconnect clients or renew their DHCP lease.
4. Verify that clients received the ESP32 as DNS.

Example:

```text
Router: 192.168.1.1
ESP32:  192.168.1.95
DNS handed to clients: 192.168.1.95
```

If the router does not let you change LAN DNS, use Mode 2 instead.

## Configure Mode 2 — ESP32 Gateway DHCP

The protected AP password is intentionally **not stored in Git**.

### 1. Create the local secrets file

```bash
cp natSecrets.example.h natSecrets.h
```

Edit `natSecrets.h`:

```cpp
#pragma once

#define PROTECTED_AP_SSID "ESP32-AdBlock"
static const char* PROTECTED_AP_PASS = "CHANGE_ME";
```

Replace `CHANGE_ME` with a private password between 8 and 63 characters. You can also change the SSID.

`natSecrets.h` is ignored by `.gitignore`. Never commit it.

A fresh clone still compiles without this file; the protected SoftAP simply remains disabled.

### 2. Compile and upload

Compile and upload normally. After the ESP32 connects to the upstream Wi-Fi, it should start the protected network.

Default protected network:

```text
SSID:       ESP32-AdBlock
Gateway:    192.168.4.1
DNS:        192.168.4.1
DHCP start: 192.168.4.10
```

### 3. Connect a client

Join the protected SSID from a phone, laptop, tablet or other device. The device should obtain a `192.168.4.x` address automatically.

## Validation

From macOS/Linux while connected to the protected AP:

```bash
ipconfig getifaddr en0                         # macOS
route -n get default | grep gateway           # macOS
dig google.com | grep SERVER
dig doubleclick.net A +short
ping -c 3 1.1.1.1
```

Expected protected-gateway behavior:

```text
Client:  192.168.4.x
Gateway: 192.168.4.1
DNS:     192.168.4.1
Blocked domain result: 0.0.0.0
Internet: reachable
```

On Linux, use the equivalent `ip addr`, `ip route` and `dig` commands.

## Uplink recovery

When the upstream Wi-Fi disappears after the protected AP is already active, the fork keeps the protected AP configuration. When the station reconnects, it restores the STA default route and reapplies NAPT automatically.

This recovery path was tested on ESP32-S3 hardware with a real upstream outage and return.

Current limitation: if the device boots while the upstream Wi-Fi is unavailable, the protected AP startup behavior is not yet designed as a fully independent offline router mode.

## Security

- The real protected Wi-Fi password belongs only in local `natSecrets.h`.
- `natSecrets.h` is ignored by Git.
- The firmware refuses to enable protected mode when the password is missing, still set to the example value, or outside the accepted length.
- Never publish credentials in commits, screenshots or logs.
- DNS filtering is not a substitute for endpoint security, a firewall or parental-control software.

## More documentation

See [`PROTECTED_AP.md`](PROTECTED_AP.md) for implementation notes about the protected SoftAP mode.

## Credits and license

This project is derived from **ESP32_AdBlocker** by `s60sc`. The DNS sinkhole and original application are upstream work; the protected gateway mode and associated integration in this fork are modifications.

Licensed under the **GNU Affero General Public License v3.0 (AGPL-3.0)**, consistent with the upstream project. See [`LICENSE`](LICENSE).
