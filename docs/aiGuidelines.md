# AI Developer Guidelines

If you are an AI assistant (e.g., Cursor, Copilot, Gemini) contributing to this project, adhere to the following architectural and stylistic rules.

## 1. Architectural Principles
* **Subsystem Compartmentalization:** Actively developed hardware and communication features live in `src/subsystems/`. Keep hardware-specific includes and state contained within their respective subsystem classes. Do not leak SDK headers into `src/main.cpp` unless required for system-wide initialization.
* **Low-Level Precision:** Prefer register-level configuration or direct SDK functions where execution speed is critical. Utilize hardware features like DMA or hardware interrupts (IRQs) for high-frequency tasks (like sensor polling) rather than blocking the main CPU loop.
* **Non-Blocking Logic:** The main loop must never be blocked. Do not use `sleep_ms()` outside of top-level system delays. Use state machines, hardware timers, and PIO for asynchronous operations.

## 2. Code Formatting & Syntax
* **Language:** C++17 or higher.
* **Encapsulation:** Use classes to represent peripherals. Use constructor delegation for default initialization profiles to improve call-site readability.
* **Documentation:** Provide strict Doxygen-style block comments (`/** ... */`) for all public class methods.
* **Naming convention:** Use camelCase for all file names, but capitalize "Pico" when first variable. Use plurals for folder names. 

## 3. Communication Style
* Present code directly and avoid overly verbose pleasantries. 
* Output markdown that is highly scannable and link-compatible for local knowledge bases.