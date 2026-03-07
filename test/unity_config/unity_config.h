#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

#include <stdio.h>
#include "pico/stdlib.h"

// Tell Unity how to output characters using the Pico SDK
#ifndef UNITY_OUTPUT_CHAR
#define UNITY_OUTPUT_CHAR(a) putchar_raw(a)
#endif

// Optional: Flush the serial buffer
#ifndef UNITY_OUTPUT_FLUSH
#define UNITY_OUTPUT_FLUSH() stdio_flush()
#endif

// Optional: Initialize hardware before tests start
#ifndef UNITY_OUTPUT_START
#define UNITY_OUTPUT_START() stdio_init_all()
#endif

#endif /* UNITY_CONFIG_H */