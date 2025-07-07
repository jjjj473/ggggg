# Arch Browser

This repository contains a minimal web browser written in C using GTK and WebKit2GTK. It is intended for Arch Linux users who want a lightweight browser with simple domain blocking for improved privacy.

## Features
- Ephemeral browsing session (no persistent cookies or cache saved to disk)
- Blocks network requests for common tracking scripts from Google, Microsoft and
  TikTok (without blocking access to the main sites)
- Minimal interface with an address bar

## Building
Ensure the required dependencies are installed on your Arch system:

```bash
sudo pacman -S base-devel gtk3 webkit2gtk
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
