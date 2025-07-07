# GTK3 Zip Tool

This repository contains a small GTK3-based application written in C that allows you to
view the contents of zip archives, extract them, and create new zip files.

## Building

The program requires GTK3 and the common `zip` and `unzip` utilities. On Arch Linux
you can install the dependencies with:

```bash
sudo pacman -S base-devel gtk3 zip unzip
```

After installing the dependencies, build the program with make:

```bash
make
```

This will produce an executable named `gtkzip`.

## Usage

Run the application with:

```bash
./gtkzip
```

Use the buttons at the top to open existing zip files, extract them to a folder,
create new archives from a selected directory, or query package information with pacman.

You'll see a small watermark in the lower-right corner stating this tool is for Arch Linux users.

