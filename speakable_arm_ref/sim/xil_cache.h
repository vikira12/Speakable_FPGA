#ifndef XIL_CACHE_H
#define XIL_CACHE_H

// PC 시뮬레이션: 캐시가 없으므로 아무것도 하지 않음
#include "xil_types.h"

static inline void Xil_DCacheFlushRange(UINTPTR a, u32 n) { (void)a; (void)n; }
static inline void Xil_DCacheInvalidateRange(UINTPTR a, u32 n) { (void)a; (void)n; }

#endif
