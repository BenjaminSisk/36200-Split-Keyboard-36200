
# V0.1

## Architecture & Core Philosophy

* **Subsystem Compartmentalization:** Restructured the project directory to strictly separate active hardware logic (`src/subsystems`), static configurations (`src/config`), and testing utilities (`src/inputEmulators`).
* **Non-Blocking Execution:** Completely removed hardware timer interrupts (Alarms 0 and 1) and blocking `sleep_ms()` calls in favor of asynchronous, state-machine-driven `update()` methods that execute safely within the main CPU loop.
* **Multicore Delegation:** Implemented the Pico SDK `multicore_fifo` to shift data parsing and logical translation to Core 1, keeping Core 0 strictly dedicated to high-frequency hardware polling.

## Added

* **`KeypadButtons` Subsystem:** A C++ hardware driver utilizing Pico SDK functions (`gpio_put`, `gpio_get`) with an integrated software debounce state machine to mathematically filter electrical noise.
* **Universal `InputHandler`:** A centralized hub to poll multiple input peripherals (matrix, joysticks) non-blockingly and push events to the multicore queue.
* **2-Byte Communication Protocol:** Implemented a robust `[Equipment ID] [Action/Value]` standard to safely encode both digital button states (0-127) and 8-bit analog joystick values (128+).
* **Static Configuration Maps:** Added `hardwareMap.h` and `qwertyMap.h` using C++17 `constexpr` arrays to map electrical matrices directly into read-only flash memory (`.rodata`), eliminating RAM consumption.
* **`TerminalEmulator`:** A UART-based Software-in-the-Loop (SIL) testing subsystem that reads serial terminal commands non-blockingly to inject mock hardware states.
* **Constructor Delegation:** Added default hardware profiles to all subsystem classes for cleaner instantiation at the call site.

## Changed

* **Matrix Addressing Math:** Updated the button indexing formula from nested arbitrary strings to a strict sequential electrical mapping format (`(row * COLS) + col`) supporting up to a 12x4 matrix layout.
* **Hardware Abstraction:** Swapped direct register bit-banging (`sio_hw->gpio_togl`, `hw_clear_bits`) for standard Pico SDK hardware abstractions to improve readability, maintainability, and safety.
* **Event Callbacks:** Replaced the hardcoded, globally defined `KeyEvents kev` queue with decoupled C++ `std::function` callbacks.

## Removed

* **Global Variables:** Eliminated the global `col` tracker and `state[16]` arrays, encapsulating them entirely within object instances.
* **Dynamic Heap Allocation:** Removed string-based maps and `std::vector` configuration data in favor of zero-overhead static arrays.
