#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
void abort(void);
#ifdef __cplusplus
}
#endif
static inline int abs(int x) { return x < 0 ? -x : x; }
