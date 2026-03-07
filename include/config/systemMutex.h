#pragma once
#include "pico/mutex.h"

//literally just used for printf statements from multiple cores without interleaving characters.
// "extern" tells the compiler this lock exists somewhere in memory, 
// allowing any file that includes this header to see it.
extern mutex_t stdio_mutex;