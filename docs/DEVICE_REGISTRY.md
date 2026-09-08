# F2.1 — Device Registry validation

This document describes the first Device Registry milestone for **ESP32 AdBlock Gateway**.

The goal of F2.1 is deliberately small: while operating in protected Gateway mode, enumerate clients associated with the ESP32 SoftAP and correlate each client MAC address with the IPv4 address assigned by the ESP32 DHCP server.

## Implementation

The first registry is RAM-only and intentionally bounded.

Each record currently tracks:

- MAC address
- current DHCP IPv4 address when available
- online/offline state
- first-seen time since boot
- last-seen time since boot
- connection count

The registry capacity is currently 16 records. If the registry becomes full, the oldest offline record can be reused. Online records are never evicted to make room for a newly seen client.

Hostname discovery, friendly names, persistence and per-device profiles are later F2/F3 work.

## How identity is resolved

F2.1 combines two ESP32 networking sources:

1. `esp_wifi_ap_get_sta_list()` enumerates stations currently associated with the SoftAP and provides their MAC addresses.
2. `esp_netif_dhcps_get_clients_by_mac()` asks the AP DHCP server for the IPv4 lease corresponding to each MAC address.

This avoids guessing identity from IP address alone.

## Events watched

The registry refreshes when the Arduino network layer reports:

- `ARDUINO_EVENT_WIFI_AP_STACONNECTED`
- `ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED`
- `ARDUINO_EVENT_WIFI_AP_STADISCONNECTED`

It also performs an initial refresh when protected Gateway mode starts.

## Expected serial output

A newly connected client should produce output similar to:

```text
[DEVICE] discovered MAC=AA:BB:CC:DD:EE:FF
[DEVICE] connected MAC=AA:BB:CC:DD:EE:FF count=1
[DEVICE] refresh=ap-connect online=1 known=1
```

The first refresh may show the IP as `pending` because Wi-Fi association happens before DHCP completes. After the DHCP lease is assigned, expect:

```text
[DEVICE] lease MAC=AA:BB:CC:DD:EE:FF IPv4=192.168.4.10 (was pending)
[DEVICE] refresh=dhcp-lease online=1 known=1
[DEVICE] #1 ONLINE MAC=AA:BB:CC:DD:EE:FF IPv4=192.168.4.10 ...
```

On disconnect:

```text
[DEVICE] disconnected MAC=AA:BB:CC:DD:EE:FF lastIPv4=192.168.4.10
```

## Hardware test matrix

Use the existing ESP32-S3 N16R8 / Arduino-ESP32 3.3.11 setup first.

### 1. Single client

- boot the ESP32 normally
- confirm the protected AP starts
- connect one phone/laptop to the protected SSID
- confirm one stable MAC record appears
- confirm the record later receives a `192.168.4.x` IPv4 address

### 2. Multiple clients

Connect at least two clients simultaneously and verify each receives a distinct record and IPv4 address.

### 3. Reconnect the same device

Disconnect and reconnect the same client. The same MAC should update the existing record and increment `connectCount`, rather than creating an uncontrolled duplicate.

> Modern phones may use a private/randomized Wi-Fi MAC per SSID. That MAC is still the correct stable identity from the gateway's point of view for that SSID, but changing/resetting the client's private address will look like a new device.

### 4. Uplink outage/recovery

While clients remain connected to the protected AP:

- interrupt the upstream Wi-Fi
- restore it
- confirm the existing NAPT recovery still succeeds
- confirm Device Registry records remain coherent

### 5. Regression checks

While connected to the protected AP, confirm the existing gateway behavior still works:

```bash
ipconfig getifaddr en0
route -n get default | grep gateway
dig google.com | grep SERVER
dig doubleclick.net A +short
ping -c 3 1.1.1.1
```

Expected:

```text
Client:  192.168.4.x
Gateway: 192.168.4.1
DNS:     192.168.4.1
Blocked domain: 0.0.0.0
Internet: reachable
```

## Acceptance criteria for F2.1

F2.1 is ready to merge only after real-hardware validation confirms:

1. associated SoftAP clients are keyed by MAC;
2. DHCP IPv4 can be correlated to the correct MAC;
3. reconnects update existing records;
4. multiple simultaneous clients remain distinct;
5. DNS filtering, DHCP, NAPT and uplink recovery still work as before.

Do not merge solely on static review: this milestone touches live AP/DHCP state and should be tested on the target ESP32-S3 hardware first.
