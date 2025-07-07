# Arch Browser

This repository contains a minimal web browser written in C using GTK and WebKit2GTK. It is intended for Arch Linux users who want a lightweight browser with simple domain blocking for improved privacy. Recent updates add a tiny SQLite database for history and download tracking as well as basic developer tools.

## Features
- Ephemeral browsing session (no persistent cookies or cache saved to disk)
- Blocks network requests for common tracking scripts from major platforms without blocking access to the main sites. The default blocklist contains about 150 analytics and advertising domains.
- Built‑in pages including a customizable home page, history, downloads, settings and an about page
- Basic developer tools enabled (open with F12)
- Minimal interface with an address bar
- Custom error pages when URLs fail to load or are invalid
- Shared CSS and JavaScript on internal pages provide a dark theme toggle

## Building
Ensure the required dependencies are installed on your Arch system:

```bash
sudo pacman -S base-devel gtk3 webkit2gtk sqlite
```

Then build the browser using `make`:

```bash
make
```

The resulting binary will be named `archbrowser`.

## Running
Execute the binary and enter a URL in the address bar. If a URL does not include a protocol, `https://` will be prepended automatically.

## Notes
This is a simple example and not a full-featured browser. It demonstrates how to block certain requests for privacy. Feel free to extend it further.

### Built-in Pages

The browser provides a few pages that are rendered internally:

- `archbrowser://home` — default start page with navigation links and a theme toggle
- `archbrowser://history` — view browsing history stored in the SQLite database
- `archbrowser://downloads` — list completed downloads
- `archbrowser://settings` — settings page with a link to clear all saved data
- `archbrowser://about` — information about the browser

Internal pages share a small style sheet and script so the theme preference is
preserved as you navigate.
