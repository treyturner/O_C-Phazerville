#pragma once

// Boot-time profiling of app Process() stages, shown by the T32 boot
// breadcrumbs (PHZ_BOOT_BREADCRUMBS). Compiles to nothing otherwise.

#include <Arduino.h>

#ifdef PHZ_BOOT_BREADCRUMBS
namespace PhzProfile {
  enum Stage : uint8_t {
    LOAD,     // HS::frame.Load(), including CLOCK and TRIGS
    CLOCK,    // internal clock (ProcessClock)
    TRIGS,    // trigger mapping pre-calculation
    APPLET_L, // left applet Controller()
    APPLET_R, // right applet Controller()
    CURSORS,  // HemisphereApplet::ProcessCursors()
    SEND,     // HS::frame.Send()
    STAGE_COUNT
  };
  extern uint32_t cycles[STAGE_COUNT];
  extern const char *applet_names[2];
}

#define PHZ_PROFILE(stage, ...) \
  do { \
    const uint32_t phz_profile_start_ = ARM_DWT_CYCCNT; \
    __VA_ARGS__; \
    PhzProfile::cycles[stage] += ARM_DWT_CYCCNT - phz_profile_start_; \
  } while (0)
#else
#define PHZ_PROFILE(stage, ...) do { __VA_ARGS__; } while (0)
#endif
