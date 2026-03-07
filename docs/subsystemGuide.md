# Subsystem Development Guide

This document defines the architectural standards for creating and integrating new hardware or logical modules (subsystems) into the RP2350 split keyboard firmware. Adhering to this guide ensures a unified, non-blocking, and highly modular codebase.

## 1. File Structure & Naming Conventions

All actively developed subsystems must reside in their own dedicated directory within `src/`.

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
| `debugPrint()` | A unified wrapper that outputs the subsystem's state to the terminal. | Must flush `stdout` |


## 4. Integration with `main.cpp`

When a subsystem is complete, follow these exact steps to integrate it into the main application logic:

1. **Include the Header:** Add `#include "subsystems/<module_name>/ModuleName.h"` at the top of `src/main.cpp`.
2. **Update the Debug Visualizer:** - Add a boolean flag to the `debug_visualizer` function signature (e.g., `bool isModule`).
* Add a constant reference to the subsystem object in the signature (e.g., `const ModuleName& module`).
* Inside the function, add an `if (isModule) { module.debugPrint(); }` block.


3. **Instantiate:** Create an instance of the object using the default constructor before the `while(true)` loop.
4. **Initialize:** Call `module.testing_QuickInit()` (or `init()` + `startTimer()` manually) right after instantiation.
5. **Add to Main Loop:** Pass the newly instantiated object into the `debug_visualizer` call inside the `while(true)` loop, toggling the boolean flag as needed for debugging.
