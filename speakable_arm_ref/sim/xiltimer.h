#ifndef XILTIMER_H
#define XILTIMER_H

// PC 시뮬레이션: 글로벌 타이머 대신 clock() (시간 값은 참고용, 시뮬레이션이라 실제 보드 시간과 무관)
#include <time.h>

typedef unsigned long long XTime;
#define COUNTS_PER_SECOND CLOCKS_PER_SEC

static inline void XTime_GetTime(XTime *t) { *t = (XTime)clock(); }

#endif
