# AI Developer Guidelines

If you are an AI assistant (e.g., Cursor, Copilot, Gemini) contributing to this project, adhere to the following architectural and stylistic rules.

## 1. Architectural Principles
* **Subsystem Compartmentalization:**: Actively developed hardware and communication features live in own folders under `src/`. 
* **Interface Compartmentalization**: Keep hardware-specific includes and state contained within their respective folders under `include/`. Do not leak SDK headers into `src/main.cpp` unless required for system-wide initialization.
* **Low-Level Precision:** Prefer register-level configuration or direct SDK functions where execution speed is critical. Utilize hardware features like DMA or hardware interrupts (IRQs) for high-frequency tasks (like sensor polling) rather than blocking the main CPU loop.
* **Non-Blocking Logic:** The main loop must never be blocked. Do not use `sleep_ms()` outside of top-level system delays. Use state machines, hardware timers, and PIO for asynchronous operations.

## 2. Code Formatting & Syntax
* **Language:** C++17 or higher.
* **Encapsulation:** Use classes to represent peripherals. Use constructor delegation for default initialization profiles to improve call-site readability.
* **Documentation:** Provide strict Doxygen-style block comments (`/** ... */`) for all public class methods. Comment the interface `.h` file heavily with heavy descriptive comments, while keeping comments in the implementation `.cpp` / `.c` files lightly (functionaltiy clarification)
  * All files need to start with a comment describing its file name, purpose, functionality and connection to other files. 
* **Naming convention:** 
  * For C++: Use PascalCase to denote structural elements or custom types. When you see PascalCase, you should immediately know, "This is a blueprint, not the actual data itself." Eg: Classes and Structs, Enums, Namespaces
  * For C++: Use camelCase for the actual data you are working with and the actions being performed. Eg: variables, instances, functions / class methods, function parameters. 
  * For C: Use snake_case for methods. 


## 3. Communication Style
* Present code directly and avoid overly verbose pleasantries. 
* Output markdown that is highly scannable and link-compatible for local knowledge bases.