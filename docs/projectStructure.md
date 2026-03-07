
## project tree
rp2350-split-keyboard/
│   .gitignore
│   platformio.ini
│   README.md
│
├───.pio
│   └───build
│       │   project.checksum
│       │
│       └───proton
│           │   idedata.json
│           │
│           └───generated
│               │   pico_flash_region.ld
│               │
│               └───pico
│                       config_autogen.h
│
├───.vscode
│       c_cpp_properties.json
│       extensions.json
│       launch.json
│
├───docs
│   │   aiGuidelines.md
│   │   projectStructure.md
│   │   quickStart.md
│   │   subsystemGuide.md
│   │
│   └───changelogs
│       └───inputs
│               CHANGELOG.md
│
├───include
│   ├───config
│   │       hardwareMap.h
│   │       quertyMap.h
│   │
│   ├───InputEmulators
│   │       terminalEmulator.h
│   │
│   ├───InputHandlers
│   │       inputHandler.h
│   │
│   ├───Joysticks
│   │       PicoJoystick.h
│   │       vis.h
│   │
│   └───KeypadButtons
│           keyMap.h
│           keypadButtons.h
│
└───src
    │   main.cpp
    │
    ├───InputEmulators
    │       terminalEmulator.cpp
    │
    ├───InputHandlers
    │       inputHandler.cpp
    │
    ├───Joysticks
    │       PicoJoystick.cpp
    │       testing.md
    │
    └───KeypadButtons
            keyMap.cpp
            keypadButtons.cpp
