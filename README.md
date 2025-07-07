# Arch Browser

This repository contains a minimal web browser written in C using GTK and WebKit2GTK. It is intended for Arch Linux users who want a lightweight browser with simple domain blocking for improved privacy. Recent updates add several built in tools backed by SQLite and additional libraries.

## Features
- Ephemeral browsing session (no persistent cookies or cache saved to disk)
- Blocks network requests for common tracking scripts from major platforms without blocking access to the main sites. The default blocklist contains about 150 analytics and advertising domains. Requests are filtered as resources load so tracking scripts never reach the network.
- Built‑in pages including a customizable home page, history, downloads, settings, bookmarks, notes and more. The home page is shown automatically on startup.
- Basic developer tools enabled (open with F12)
- Simple interface with a toolbar containing Back, Forward, Reload and Home buttons plus an address bar and load progress
- Custom error pages when URLs fail to load or are invalid
- Shared CSS and JavaScript on internal pages provide a dark theme toggle
- Uses GTK, WebKit2GTK, SQLite3, libxml2, libarchive and OpenSSL
- Pop-up windows from websites are opened in separate windows so they can easily be closed

## Building
Ensure the required dependencies are installed on your Arch system:

```bash
sudo pacman -S base-devel gtk3 webkit2gtk sqlite libxml2 libarchive openssl
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

The browser provides many pages that are rendered internally:

- `archbrowser://home` — default start page with navigation links and a theme toggle
- `archbrowser://history` — view browsing history stored in the SQLite database
- `archbrowser://downloads` — list completed downloads
- `archbrowser://bookmarks` — manage saved bookmarks
- `archbrowser://notes` — take quick notes stored in the database
- `archbrowser://network` — see a log of all network requests
- `archbrowser://settings` — settings page with a link to clear all saved data
- `archbrowser://extensions` — placeholder extension manager
- `archbrowser://about` — information about the browser
- `archbrowser://help` — simple help page

Navigation links on the home page use this custom scheme and now load correctly within the browser.
The browser also accepts optional trailing slashes or query strings after each
internal URL so `archbrowser://history/` will work the same as
`archbrowser://history`.

Internal pages share a small style sheet and script so the theme preference is
preserved as you navigate.
