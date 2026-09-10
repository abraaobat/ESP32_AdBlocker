#!/usr/bin/env python3
"""Validate the F4.2 domain-category source without requiring ESP32 hardware."""

from __future__ import annotations

import ipaddress
from collections import defaultdict
from pathlib import Path

ALLOWED_CATEGORIES = {
    "ads",
    "trackers",
    "analytics",
    "telemetry",
    "malware",
    "phishing",
    "unknown",
}
SOURCE = Path("data/domain-categories.txt")
MAX_ENTRIES = 8192


def normalize_domain(value: str) -> str:
    domain = value.strip().lower().rstrip(".")
    if domain.startswith("www."):
        domain = domain[4:]
    return domain


def validate_domain(domain: str, line_number: int) -> None:
    if not domain or len(domain) > 253:
        raise ValueError(f"line {line_number}: invalid domain length")
    try:
        ipaddress.ip_address(domain)
    except ValueError:
        pass
    else:
        raise ValueError(f"line {line_number}: IP literals are not domain entries")

    labels = domain.split(".")
    if any(not label or len(label) > 63 for label in labels):
        raise ValueError(f"line {line_number}: invalid DNS label")
    for label in labels:
        if label.startswith("-") or label.endswith("-"):
            raise ValueError(f"line {line_number}: label cannot start/end with '-' ")
        if not all(ch.isalnum() or ch == "-" for ch in label):
            raise ValueError(f"line {line_number}: unsupported domain character")


def main() -> int:
    if not SOURCE.exists():
        raise SystemExit(f"missing fixture: {SOURCE}")

    merged: dict[str, set[str]] = defaultdict(set)
    parsed_entries = 0

    for line_number, raw in enumerate(SOURCE.read_text(encoding="utf-8").splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if "|" not in line:
            raise SystemExit(f"line {line_number}: expected domain|category[,category...]")

        raw_domain, raw_categories = line.split("|", 1)
        domain = normalize_domain(raw_domain)
        validate_domain(domain, line_number)

        categories = {item.strip().lower() for item in raw_categories.split(",") if item.strip()}
        if not categories:
            raise SystemExit(f"line {line_number}: empty category set")
        unknown = categories - ALLOWED_CATEGORIES
        if unknown:
            raise SystemExit(f"line {line_number}: unknown categories: {sorted(unknown)}")

        merged[domain].update(categories)
        parsed_entries += 1
        if parsed_entries > MAX_ENTRIES:
            raise SystemExit(f"fixture exceeds MAX_ENTRIES={MAX_ENTRIES}")

    if not merged:
        raise SystemExit("fixture contains no valid entries")

    expected = {
        "doubleclick.net": {"ads", "trackers"},
        "multi-category.example": {"ads", "analytics", "telemetry", "trackers"},
    }
    for domain, categories in expected.items():
        if merged.get(domain) != categories:
            raise SystemExit(
                f"{domain}: expected {sorted(categories)}, got {sorted(merged.get(domain, set()))}"
            )

    print(
        f"category fixture valid: entries={parsed_entries} unique_domains={len(merged)} "
        f"categories={sorted(ALLOWED_CATEGORIES)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
