# Task 1 — Bare-Metal Boot on Raspberry Pi 4B

**Course:** CS5600 Operating Systems  
**Date:** March 6, 2026  
**Team:** Abdul Wasey Mohammed, Ateeq  

## Overview

Boots a Raspberry Pi 4B without any OS. The kernel:
- Parks CPU cores 1–3; core 0 continues
- Initializes mini UART (GPIO 14/15, 115200 baud)
- Queries the GPU via mailbox for ARM/VC memory regions and board revision
- Reads and prints the current exception level
- Prints `hello from bare metal`

## File Structure

```
Task1/
├── boot.S      # AArch64 entry point; parks extra cores, sets up stack & BSS
├── kernel.c    # Main kernel logic and boot messages
├── uart.c / uart.h   # Mini UART (UART1) driver
├── mbox.c / mbox.h   # Mailbox property tag interface (ARM ↔ GPU)
├── Makefile    # Cross-compilation build system
├── link.ld     # Linker script — places entry at 0x80000
└── config.txt  # SD card firmware config (64-bit mode, core_freq=500)
```

## Build

Requires `aarch64-none-elf-gcc` on your PATH:

```bash
make
```

This produces `kernel8.img`.

## Deploy

1. Format a micro-SD card as FAT32.
2. Copy the Raspberry Pi 4 firmware files onto it:
   - `bootcode.bin`
   - `start4.elf`
   - `fixup4.dat`
3. Copy `config.txt` and `kernel8.img` from this directory onto the SD card.
4. Insert the SD card into the Pi 4 and power on.

## Serial Output

Connect a USB-to-TTL serial adapter to GPIO 14 (TXD) and 15 (RXD), then open a terminal at **115200 baud** (e.g. PuTTY on Windows).

## Key Lessons

- The VideoCore GPU controls the early boot sequence on the Pi 4, not the ARM.
- All four ARM cores start at the same address; the assembly stub parks cores 1–3.
- The mini UART baud rate depends on the core clock — locking it with `core_freq=500` in `config.txt` is essential.
- GPIO alternate function selection must match the datasheet exactly; wrong alt mode = silent failure.
