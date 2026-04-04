# RP2350 Split Keyboard Firmware

A modular, C++ based firmware for an RP2350-powered split keyboard. This project integrates multiple hardware subsystems into a unified, non-blocking architecture.

## Diagrams

Here is a link to the [diagrams](https://drive.google.com/file/d/1josPRPToi0hVMcOZqZrqSURNXhiCR-l0/view?usp=sharing).

For more information, see the the [docs](./docs) directory.

## Features

- **ADC Joystick Interface:** Configurable, math-driven transformation pipeline with an integrated Exponential Moving Average (EMA) filter for noise reduction.
- **LED Patterns:** (WIP) Dynamic visual feedback utilizing Programmable I/O (PIO).
- **Intra-communication:** (WIP) High-speed data transfer between split keyboard halves.
- **USB HID:** (WIP) Standardized host-computer interfacing.

## Hardware Stack

- Microcontroller: Raspberry Pi Pico 2 / RP2350 (QFN-80 Package)
- C/C++ Pico SDK

## Folder Structure

- `src/`: Contains the main application logic and entry point.
- `src/`: Contains actively developed, project-specific hardware and logical modules (e.g., joystick, leds, comm).
- `include/`: Global headers and hardware pinout definitions.
- `docs/`: Project tracking, change logs, and architectural planning.
- `tests/`: Unit testing for independent modules.

## Getting Started

1. Install the Pico C/C++ SDK.
2. Clone this repository.
- git pull
- git submodule init
- git submodule update
3. Build + Upload and Monitor on PlatformIO
