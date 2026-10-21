/* Minimal platform declarations (no system headers needed for the Android build). */
#ifndef PLAT_H
#define PLAT_H
#include <stdint.h>
#include <stddef.h>

float sinf(float);
float cosf(float);
float tanf(float);
float expf(float);
float powf(float, float);
float logf(float);
float sqrtf(float);
void *memset(void *, int, size_t);
void *memcpy(void *, const void *, size_t);
void *memmove(void *, const void *, size_t);
int memcmp(const void *, const void *, size_t);
void *malloc(size_t);
void free(void *);

#define PI_F 3.14159265f
#define CLAMP(x, a, b) ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))

static inline float tanh_fast(float x) {
    if (x > 3.f) return 1.f;
    if (x < -3.f) return -1.f;
    float x2 = x * x;
    return x * (27.f + x2) / (27.f + 9.f * x2);
}
static inline float absf(float x) { return x < 0 ? -x : x; }

#endif
