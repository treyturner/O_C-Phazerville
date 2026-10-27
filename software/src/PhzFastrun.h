#pragma once

// Teensy 3.2 runs code from flash with wait states, which makes the per-tick
// app path markedly slower than code in RAM. PHZ_FASTRUN_HOT is an experiment
// that places the hot shared per-tick functions in RAM, like FASTRUN.
//
// Each function gets its own .fastrun.<name> section, which the linker script
// places in RAM. A shared ".fastrun" section is silently dropped under LTO for
// functions defined in a class body, which are COMDAT.

#include <Arduino.h>

#if defined(__MK20DX256__) && defined(PHZ_FASTRUN_HOT)
#define PHZ_HOT(name) __attribute__ ((section(".fastrun." #name), noinline, noclone))
#else
#define PHZ_HOT(name)
#endif
