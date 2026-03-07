
# V0.2

[Unreleased] - 2026-03-06
Added
Centralized Hardware Pin Mapping: Added a Pins namespace within hardwareMap.h to define all physical GPIO connections (e.g., LEFT_JOY_SW, KEYBOARD_ROW_PINS) in a single location. WHY: This ensures that if the physical PCB layout changes, you only need to update one configuration file instead of hunting down pin numbers across multiple C++ classes.

Virtual Component Support (NO_PIN): Added a NO_PIN (255) constant to represent unconnected or virtual hardware pins. WHY: This allows the firmware to compile a complete system while gracefully skipping adc_read() or gpio_get() calls for missing components, preventing floating voltages from overwriting simulated states injected by the emulator.

Global Hardware Mutex (stdio_mutex): Introduced a global mutex lock using pico/mutex.h to wrap all printf statements. WHY: Because the RP2350 uses a multicore architecture, Core 0 and Core 1 were simultaneously attempting to write to the USB serial bus, resulting in interleaved, garbled text. The mutex acts as a token, forcing the cores to wait their turn before printing.

Changed
Hardware Initialization (Board Support Package Pattern): Refactored hardware instantiation out of main.cpp and into a dedicated SystemHardware configuration struct. WHY: Previously, main.cpp was becoming cluttered. Instead of hardcoding hardware strictly inside InputHandler (which traps the data), this Dependency Injection pattern instantiates everything in one clean block and passes references to the subsystems that need them.

Separation of Concerns for KeyMap: Moved KeyMap logic out of the physical KeypadButtons hardware class and into the logical InputEmulators application layer. WHY: Hardware should only care about physical circuit closures (integer IDs), while the application layer should handle the translation to human-readable characters (like QWERTY or specific Markdown triggers).

Terminal Emulator Type Parsing: Updated sscanf parsing in TerminalEmulator::parseCommand() to use standard int variables and %d format specifiers instead of uint8_t and %hhu. WHY: The Pico SDK uses a highly compressed C library (newlib-nano) that strips out support for obscure format specifiers. Using standard integers prevents memory overflow errors during parsing.

Fixed
Multicore FIFO Lockup (Silent Failure): Fixed an issue where Core 1 appeared frozen by calling init() on the PicoJoystick before the main loop. WHY: Uninitialized ADC pins were reading floating electrical noise, causing the joystick filter to detect constant "movement" and instantly flood the multicore FIFO queue, dropping all legitimate keypad presses.

Terminal Emulator Blind Typing: Added putchar(c) and fflush(stdout) to the TerminalEmulator::update() loop. WHY: The USB serial interface utilizes a line buffer and does not automatically echo keystrokes back to the user's terminal. Manually bouncing the characters and forcing a flush ensures the developer can see what they are typing in real-time.

CRLF Double Execution Bug: Modified the emulator's UART read loop to trigger strictly on Carriage Return (\r) and actively ignore Line Feed (\n). WHY: Standard terminal monitors send both characters sequentially when you press the "Enter" key. Ignoring the \n prevents the emulator from executing the command logic twice and printing duplicate prompt messages.

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
