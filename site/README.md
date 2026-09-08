# ESP32 AdBlock Gateway landing page

Static landing page for GitHub Pages.

## Publishing

The repository includes `.github/workflows/pages.yml`, which deploys the contents of `site/` whenever site files are changed on `main`.

One repository setting is required once:

1. GitHub repository → **Settings** → **Pages**
2. Under **Build and deployment**, choose **GitHub Actions** as the source.
3. Run the `Deploy landing page to GitHub Pages` workflow if it does not start automatically.

Expected project URL:

`https://abraaobat.github.io/ESP32_AdBlocker/`

## Search indexing

The site includes:

- semantic HTML headings
- title + meta description
- Open Graph metadata
- `SoftwareApplication` JSON-LD structured data
- canonical URL
- `robots.txt`
- `sitemap.xml`
- responsive layout
- real text content instead of a page-sized image

After the page is live, add the GitHub Pages URL (or a future custom domain) to Google Search Console and submit `/sitemap.xml`.

## Languages

The landing page ships with a client-side language switcher for:

- English
- Português (Brasil)
- Español
- Français
- 简体中文

English is the default crawlable page. Separate localized URLs can be introduced later if independent per-language SEO becomes a priority.

## Purchase button

The initial **Buy ESP32-S3 board** button points to a neutral Mercado Livre search for `ESP32-S3 N16R8` and does not claim an affiliate relationship.

If an affiliate or preferred supplier URL is added later, update the `href` in `site/index.html` and disclose the affiliate relationship near the button.

## Firmware button

The download CTA points to the repository's GitHub Releases page. Publish tested binary firmware there before advertising a one-click binary download.

## US$1 support button

The support CTA is prepared for GitHub Sponsors using campaign metadata:

`https://github.com/sponsors/abraaobat?metadata_campaign=esp32-adblock-gateway`

The maintainer must first have an active GitHub Sponsors profile. Until Sponsors is active, the page explicitly says that payment buttons require Sponsors setup.

Do not imply that a contribution buys exclusive rights to the AGPL software. Position it as optional support for firmware maintenance, documentation, hardware testing and future development.

## Project claims

Keep the landing page precise:

- DNS filtering reduces many ad/tracker domains; it does not guarantee 100% ad removal.
- apps using their own DoH/DoT may bypass local DNS filtering.
- current validated Gateway behavior is IPv4-focused; IPv6 is not claimed as solved.
- the gateway is not positioned as a professional firewall replacement.
- retain attribution to `s60sc/ESP32_AdBlocker` and the AGPL-3.0 license.
