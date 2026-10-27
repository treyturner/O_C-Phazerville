#pragma once

// T32 diagnostics (PHZ_TRACE): PHZ_TRACE_STEP shows a labelled screen and
// pauses, so the last screen left up marks where a hang happened. A hard
// fault shows its registers and the last step instead. Compiles to nothing
// otherwise.

#ifdef PHZ_TRACE
void PhzTrace(const char *label, const char *detail = nullptr);
#define PHZ_TRACE_STEP(...) PhzTrace(__VA_ARGS__)
#else
#define PHZ_TRACE_STEP(...) do {} while (0)
#endif
