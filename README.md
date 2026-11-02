# Toda OS 📱⚡

> **An original, from-scratch mobile operating system designed for ARM64 devices.**  
> *Zero Linux. Zero Android. 100% Custom Microkernel Architecture.*

---

## 🌟 Overview

**Toda OS** is an independent, security-first mobile operating system written entirely in C and ARM64 Assembly. It does not use the Linux kernel, POSIX layers, or existing Android runtimes. Every component — from the low-level bootloader to the memory allocator, interrupt controllers, and graphical framebuffer — is built from the ground up.

Toda OS is being designed as an open, national-sovereignty smartphone platform targeting modern **Qualcomm Snapdragon** hardware, in future collaboration with **AI Plus** mobile hardware manufacturing.

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    TODA OS ARCHITECTURE                     │
├─────────────────────────────────────────────────────────────┤
│  [ APPLICATIONS & MOBILE SHELL ]                            │
│  ├── 480x800 ARGB32 Mobile Compositor                       │
│  ├── Status Bar (5G, Battery %, Clock)                      │
│  ├── App Grid (Phone, SMS, Settings, Toda Store, Camera)    │
│  └── Interactive Phone Dialer & Keypad                      │
├─────────────────────────────────────────────────────────────┤
│  [ SYSTEM SERVICES & DRIVERS ]                              │
│  ├── Linear Framebuffer Video Driver (fb.c)                 │
│  ├── PL011 UART Serial Controller                           │
│  ├── ARM Generic Timer (1000 Hz / 1ms ticks)                │
│  └── GICv2 Generic Interrupt Controller                     │
├─────────────────────────────────────────────────────────────┤
│  [ CORE REAL-TIME KERNEL ]                                  │
│  ├── Round-Robin Multitasking Process Scheduler             │
│  ├── ARM64 16-slot Exception Vector Table (VBAR_EL1)        │
│  ├── Physical Page Allocator (4KB pages, managing RAM)      │
│  ├── Dynamic Kernel Heap Allocator (kmalloc / kfree)        │
│  └── Cryptographic Boot Chain Guard & Verified Boot         │
├─────────────────────────────────────────────────────────────┤
│  [ BARE METAL BOOTLOADER ]                                  │
│  └── ARM64 Assembly Boot Code (boot.S)                      │
└─────────────────────────────────────────────────────────────┘
```

---

## 🚀 Getting Started

### Prerequisites

You need an ARM64 cross-compiler and QEMU emulator.

#### On Ubuntu / WSL2:
```bash
sudo apt update
sudo apt install -y gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu qemu-system-arm make
```

---

### Building Toda OS

Clone the repository and compile:

```bash
git clone https://github.com/<your-username>/toda-os.git
cd toda-os
make clean
make
```

---

### Running in QEMU Emulator

#### Mobile Graphical Window Mode:
```bash
make run
```

#### Terminal Console Mode:
```bash
make run-cli
```

*(To exit QEMU at any time, press `Ctrl + A` then `X`)*.

---

## 🎮 Interactive Commands

Toda OS includes an interactive diagnostic shell:

| Command | Description |
|---|---|
| `help` | Lists all available system commands |
| `sysinfo` | Full CPU, memory, security, and hardware diagnostics |
| `ps` | Live process table showing PID, state, and priority |
| `mem` | Physical RAM usage, free page counts, and heap stats |
| `uptime` | Live system timer tick counts |
| `test` | Automated memory allocation integrity verification |
| `about` | Toda OS mission & architecture specifications |
| `clear` | Clears the terminal screen |

---

## 🔒 Security Architecture

Toda OS implements a **Verified Boot Chain**:
- **Hardware Master Key**: Cryptographically hashes the bootloader and kernel.
- **Read-Only System Partition**: System binaries cannot be tampered with at runtime.
- **Signature Enforced Apps**: Only applications signed with the Toda OS cryptographic key can execute.

---

## 📜 License

Toda OS is open-source software licensed under the **Apache License 2.0**.

