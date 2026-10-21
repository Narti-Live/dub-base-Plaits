#pragma once
#ifdef __cplusplus
extern "C" {
#endif
float sinf(float); float cosf(float); float tanf(float); float expf(float); float logf(float);
float powf(float, float); float log2f(float); float exp2f(float); float atanf(float); float tanhf(float);
static inline float fabsf(float x) { return __builtin_fabsf(x); }
static inline float sqrtf(float x) { return __builtin_sqrtf(x); }
static inline float floorf(float x) { return __builtin_floorf(x); }
static inline float ceilf(float x) { return __builtin_ceilf(x); }
static inline float roundf(float x) { return __builtin_roundf(x); }
static inline float fminf(float a, float b) { return a < b ? a : b; }
static inline float fmaxf(float a, float b) { return a > b ? a : b; }
static inline double fabs(double x) { return __builtin_fabs(x); }
#ifdef __cplusplus
}
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
