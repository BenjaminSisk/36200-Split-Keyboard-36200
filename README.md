# RP2350 Split Keyboard Firmware

A modular, C++ based firmware for an RP2350-powered split keyboard. This project integrates multiple hardware subsystems into a unified, non-blocking architecture.

## Features
* **ADC Joystick Interface:** Configurable, math-driven transformation pipeline with an integrated Exponential Moving Average (EMA) filter for noise reduction.
* **LED Patterns:** (WIP) Dynamic visual feedback utilizing Programmable I/O (PIO).
* **Intra-communication:** (WIP) High-speed data transfer between split keyboard halves.
* **USB HID:** (WIP) Standardized host-computer interfacing.

## Hardware Stack
* Microcontroller: Raspberry Pi Pico 2 / RP2350 (QFN-80 Package)
* C/C++ Pico SDK

## Folder Structure
* `src/`: Contains the main application logic and entry point.
* `src/subsystems/`: Contains actively developed, project-specific hardware and logical modules (e.g., joystick, leds, comm).
* `config/`: Global headers and hardware pinout definitions.
* `docs/`: Project tracking, change logs, and architectural planning.
* `tests/`: Unit testing for independent modules.

## Getting Started
1. Install the Pico C/C++ SDK.
2. Clone this repository.
3. Use CMake to generate the build files and compile the `.uf2` binary.