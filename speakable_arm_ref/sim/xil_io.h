#ifndef XIL_IO_H
#define XIL_IO_H

// PC 시뮬레이션: 레지스터 쓰기는 무시 (글로벌 타이머 켜기 등)
#include "xil_types.h"

static inline void Xil_Out32(UINTPTR a, u32 v) { (void)a; (void)v; }

#endif
