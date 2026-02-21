
## project tree
rp2350-split-keyboard/
├── CMakeLists.txt
├── README.md
├── ai-guidelines.md
├── config/                  
│   └── system_config.h
├── docs/                    
│   ├── change-log.md
│   └── next-steps.md
├── src/                     
│   ├── subsystems/          # Actively developed internal modules
│   │   ├── joystick/
│   │   ├── leds/
│   │   └── comm/
│   └── main.cpp             # Main entry point
└── tests/                   
    └── test_joystick.cpp