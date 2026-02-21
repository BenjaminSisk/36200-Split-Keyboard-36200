# Subsystem Development Guide

This document defines the architectural standards for creating and integrating new hardware or logical modules (subsystems) into the RP2350 split keyboard firmware. Adhering to this guide ensures a unified, non-blocking, and highly modular codebase.

## 1. File Structure & Naming Conventions

All actively developed subsystems must reside in their own dedicated directory within `src/subsystems/`.

* Create a new folder for the subsystem: `src/subsystems/<module_name>/`
* Use PascalCase for class files: `ModuleName.h` and `ModuleName.cpp`
* Keep all hardware-specific SDK includes (e.g., `hardware/pio.h`, `hardware/timer.h`) completely encapsulated within the subsystem's header or source files. Do not leak them into `main.cpp`.

## 2. Class Architecture Standards

Every subsystem class must implement a strict "contract" of standard constructors and methods. This predictability makes `main.cpp` extremely clean and easy to maintain.

### Required Constructors
You must provide two constructors. Use C++ constructor delegation to keep the code DRY (Don't Repeat Yourself).

* **Parameterized Constructor:** Accepts all hardware pins, configuration structs, and tuning variables (like timer intervals or filter alphas). 
* **Overloaded Default Constructor:** Takes zero arguments and delegates to the parameterized constructor using the default/standard hardware configurations for the project.

### Required Methods
Implement the following methods in your public interface.

| Method | Purpose | Notes |
| :--- | :--- | :--- |
| `init()` | Initializes GPIO, PIO, or ADC hardware pins. | Called once before the main loop. |
| `startTimer()` | Registers the hardware timer IRQ for the subsystem. | Required if the subsystem relies on non-blocking background polling. |
| `testing_QuickInit()` | Wrapper that sequentially calls `init()` and `startTimer()`. | Used for rapid prototyping and testing in `main.cpp`. |
| `update()` | Contains the core logic to read sensors, update state, or write to hardware. | Should be called exclusively by the timer IRQ callback, not the main loop. |
| `debugPrint()` | A unified wrapper that outputs the subsystem's state to the terminal. | Must flush `stdout`. Use `\r` for single-line overwrite or ANSI codes for multi-line. |

## 3. Implementation Example (Header)

```cpp
#pragma once
#include "pico/stdlib.h"

class ExampleSubsystem {
private:
    uint8_t status_pin;
    struct repeating_timer timer;
    static bool timerCallback(struct repeating_timer *t);

public:
    ExampleSubsystem(); // Delegates to parameterized
    ExampleSubsystem(uint8_t pin); 

    void init();
    void startTimer();
    void testing_QuickInit();
    void update();
    void debugPrint() const;
};

```

## 4. Integration with `main.cpp`

When a subsystem is complete, follow these exact steps to integrate it into the main application logic:

1. **Include the Header:** Add `#include "subsystems/<module_name>/ModuleName.h"` at the top of `src/main.cpp`.
2. **Update the Debug Visualizer:** - Add a boolean flag to the `debug_visualizer` function signature (e.g., `bool isModule`).
* Add a constant reference to the subsystem object in the signature (e.g., `const ModuleName& module`).
* Inside the function, add an `if (isModule) { module.debugPrint(); }` block.


3. **Instantiate:** Create an instance of the object using the default constructor before the `while(true)` loop.
4. **Initialize:** Call `module.testing_QuickInit()` (or `init()` + `startTimer()` manually) right after instantiation.
5. **Add to Main Loop:** Pass the newly instantiated object into the `debug_visualizer` call inside the `while(true)` loop, toggling the boolean flag as needed for debugging.
