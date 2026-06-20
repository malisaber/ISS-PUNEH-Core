# PUNEH Simple Processor ISS

## Description

This repository contains a SystemC instruction-set simulator for the PUNEH
simple 16-bit processor. The model includes:

- the CPU core and instruction decode/execute pipeline
- a shared bus/channel abstraction
- a TLM memory subsystem
- a register file
- a UART/USART peripheral model
- interrupt handling and semihosting-style intercepts

The code also boots a small demo program in simulated memory. That program
computes a sequence, stores intermediate values, and prints values through the
`PAC` instruction path.

The hardware RTL for PUNEH lives in the upstream project:

[UTehran-NavabiLab/PUNEH-simple-processor](https://github.com/UTehran-NavabiLab/PUNEH-simple-processor)

## Repository Layout

- `include/` - public headers for the simulator
- `src/` - SystemC/TLM implementation files and the `sc_main` entry point
- `docs/images/` - screenshots from the original project
- `docs/presentation/` - the original presentation material
- `artifacts/` - generated logs and other non-source leftovers


## Build Requirements

- GNU Make
- a C++17-capable compiler such as `g++` or `clang++`
- `git`
- SystemC

The Makefile supports two modes:

- default mode uses `vcpkg` to bootstrap and install SystemC into
  `.deps/vcpkg/installed/...`
- external mode uses an existing SystemC install by setting `SYSTEMC_ROOT`

On Windows, the Makefile uses PowerShell for its build recipes. On Unix-like
systems it uses `/bin/sh`.

## Build

Clone the repository:

```bash
git clone https://github.com/malisaber/ISS-PUNEH-Core.git
cd ISS-PUNEH-Core
```

Build with the default `vcpkg`-managed SystemC setup:

```bash
make install-deps
make
make run
```

The executable is written to:

- `bin/puneh-iss.exe` on Windows
- `bin/puneh-iss` on Unix-like systems

If you already have SystemC installed somewhere else, point the build at it by
setting `SYSTEMC_ROOT` before invoking `make`:

```bash
make SYSTEMC_ROOT=/path/to/systemc
```

In that mode, `make install-deps` is skipped and the build links against the
SystemC installation you provided.

## What the Model Does

At a high level, the simulator:

- fetches instructions from memory
- decodes full-address and no-address opcodes
- updates the accumulator, flags, program counter, and operand register
- reads and writes memory-mapped peripherals
- services interrupts by saving state in the register file area
- supports semihosting-style instruction interception for selected opcodes

The built-in demo program in `src/memory.cpp` initializes a sequence and prints
values during execution so you can see the processor state evolve.

## Notes

- Generated logs are ignored by default.
- The repo is arranged in a conventional source/include layout so it is easier
  to browse on GitHub and easier to hook into CI later.
