# RP2350 Split Keyboard Firmware

Richard Li, Ben Sisk, Conner Lu, Michael Wang 

A modular, C++ based firmware for an RP2350-powered split keyboard. This project integrates multiple hardware subsystems into a unified, non-blocking architecture.

## Description

Overview
This repository contains the custom firmware for a highly integrated 40% split ergonomic keyboard powered by the RP2350 microcontroller. The architecture features dual analog thumb joysticks, a multicore processing design, per-key WS2812 RGB lighting, and a robust wireless bridge.

The system is designed with a "main/peripheral" topology. The left half acts as a peripheral, securely transmitting its input matrix and joystick states over a UART-over-BLE bridge (utilizing Nordic transceivers) to the right half. The right half serves as the central hub, aggregating the unified state and handling the TinyUSB composite HID connection to the host machine.

Here is a link to the [diagrams](https://drive.google.com/file/d/1josPRPToi0hVMcOZqZrqSURNXhiCR-l0/view?usp=sharing).

For more information, see the the [docs](./docs) directory.

## Features

Hardware Specifications:
Microcontroller: Dual RP2350 (one per half).
SDK: C/C++ Pico
Key Matrix: 4x6 grid per half (40% layout total).
Joysticks: Dual analog thumb joysticks with integrated tactile switches.
Lighting: WS2812 RGB LED strips utilizing RP2350 PIO and DMA channels.
Wireless Bridge: UART over 2.4GHz BLE (Nordic chips) for split communication.## Hardware Stack


Algorithms: 
- **ADC Joystick Interface:** Configurable, math-driven transformation pipeline with an integrated Exponential Moving Average (EMA) filter for noise reduction.
- **LED Patterns:** (WIP) Dynamic visual feedback utilizing Programmable I/O (PIO).
- **Intra-communication:** (WIP) High-speed data transfer between split keyboard halves.
- **USB HID:** (WIP) Standardized host-computer interfacing.

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

# details: 


## Firmware Architecture

The firmware utilizes the RP2350's dual-core capabilities to strictly separate timing-critical LED animations from input polling and USB communication.

### Core 0: Input Processing & USB HID

Dedicated to hardware polling, state aggregation, and host communication.

- **TinyUSB Composite HID:** Concurrently manages Keyboard, Mouse (driven by analog joysticks), and Gamepad reports. Uses a smart report-chaining algorithm to prevent endpoint stalling.
    
- **Unified State Array:** Employs a flat 256-element `equipmentState` array mapped to global Equipment IDs. This establishes a single source of truth, cleanly merging local matrix reads with incoming UART peripheral data without array out-of-bounds risks.
    
- **Debouncing & Filtering:** * Keypad matrix features a threshold-based debouncer.
    
    - Analog joysticks utilize an Exponential Moving Average (EMA) filter and dynamic noise thresholds to eliminate cursor drift and prevent BLE buffer flooding.
        
    - Joystick tactile switches implement a strict multi-cycle debouncer to ensure clean UART transmissions.
        

### Core 1: WS2812 LED Management

Dedicated to visual feedback and animations.

- **PIO & DMA Driven:** Offloads the strict timing requirements of the WS2812 protocol from the CPU using programmable I/O and direct memory access.
    
- **Thread-Safe IPC:** Receives reactive lighting triggers from Core 0 via the Pico multicore FIFO hardware queues.
    

---

## Communication Protocol (UART over BLE)

To survive the inherent packet loss of 2.4GHz wireless environments, the inter-half communication uses a strictly framed 3-byte payload architecture: `[SYNC_BYTE] [EQUIPMENT_ID] [ACTION_VALUE]`

- **SYNC_BYTE (`0xFF`):** Ensures the receiver can instantly realign its buffer if a byte is dropped in transit, preventing fatal desynchronization.
    
- **EQUIPMENT_ID:** The globally mapped ID of the hardware component (e.g., `128` for Left Joystick X, `12` for the 'A' key).
    
- **ACTION_VALUE:** `0`/`1` for digital switches, or `0-255` for analog axis data.
    

---

## Project Structure & Key Modules

- `main.cpp`: The entry point. Initializes multicore execution, hardware peripherals, and maintains the primary execution loops for USB tasks and input polling.
    
- `inputHandler.h` / `.cpp`: The central aggregator. Maps physical row/col intersections to global Equipment IDs, processes UART streams, and maintains the absolute `equipmentState`.
    
- `PicoJoystick.h` / `.cpp`: A highly modular class managing ADC reads, EMA noise filtering, coordinate transformation (inversion/swapping), and localized switch debouncing.
    
- `keypadButtons.h` / `.cpp`: A "dumb" hardware scanner that strictly isolates physical GPIO state and bounce management from logical key mapping.
    
- `usb.hpp`: The TinyUSB interface layer. Translates the 256-element `equipmentState` into active HID descriptors and seamlessly cascades Keyboard, Mouse, and Gamepad reports.
    
- `hardwareMap.h`: The compile-time configuration file defining GPIO pins, global Equipment IDs, and QWERTY mapping rules.
    

---

## Building and Flashing

The codebase is unified; the exact same code runs on both the left and right microcontrollers. Handedness is determined at compile time to optimize out unused logic and save firmware space.

Before compiling, set the target half in `hardwareMap.h`:

C++

```
// Set to true when building for the Left (Peripheral) half.
// Set to false when building for the Right (Central/USB) half.
constexpr bool IS_LEFT_HALF = false; 



