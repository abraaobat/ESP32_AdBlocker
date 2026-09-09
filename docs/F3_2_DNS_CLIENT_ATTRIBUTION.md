# F3.2 — DNS Client Attribution

## Goal

Attribute each DNS query received on the ESP32 Gateway to the active client that originated it, then resolve that client to its persistent MAC-keyed profile.

Target path:

```text
DNS source IPv4
  -> Device Registry active IPv4 lookup
  -> MAC identity
  -> persistent device profile
  -> client context for policy evaluation
```

## Safety boundary

Attribution must never make DNS unavailable. If the source IPv4 cannot be matched to an online Device Registry entry, the request receives an anonymous client context with the `standard` profile.

This phase establishes attribution only. It does not yet change allow/block decisions by profile.

## Implementation

`deviceRegistryFindOnlineByIPv4()` performs a bounded lookup over at most 16 registry entries.

`dnsClientAttributionResolve()` returns:

- whether attribution succeeded
- source IPv4
- MAC when known
- effective profile ID

Unknown clients fall back to `standard`.

## DNS integration

The DNS worker already receives the client address through `recvfrom()` in `sockaddr_in cli`. The integration hook passes `cli.sin_addr.s_addr` into the DNS query processing path, where `dnsClientAttributionResolve()` can be invoked before policy evaluation.

The registry stores DHCP addresses from `esp_netif_pair_mac_ip_t.ip.addr`, so both values use the lwIP IPv4 address representation and can be compared directly on the ESP32 target.

## Validation targets

With two clients online and assigned different profiles:

1. generate DNS queries from both clients
2. confirm logs attribute each source IPv4 to the correct MAC
3. confirm each query resolves to the expected profile
4. reboot the ESP32 and repeat after DHCP addresses change
5. confirm unknown/unmatched queries fall back to `standard`
6. confirm DNS answers remain identical to pre-F3.2 behavior

## Next step

F3.3 wires the attributed profile into the Policy Resolver. Only then should two clients receive different DNS decisions for the same domain.
