/* Minimal bare-metal runtime for the clang build of the Plaits firmware. */
#include <stddef.h>
#include <stdint.h>

void *memset(void *d, int c, size_t n) {
  unsigned char *p = d;
  while (n && ((uintptr_t)p & 3)) { *p++ = (unsigned char)c; n--; }
  uint32_t w = (unsigned char)c; w |= w << 8; w |= w << 16;
  uint32_t *q = (uint32_t *)p;
  while (n >= 16) { q[0] = w; q[1] = w; q[2] = w; q[3] = w; q += 4; n -= 16; }
  while (n >= 4) { *q++ = w; n -= 4; }
  p = (unsigned char *)q;
  while (n--) *p++ = (unsigned char)c;
  return d;
}
void *memcpy(void *d, const void *s, size_t n) {
  unsigned char *p = d; const unsigned char *q = s;
  if ((((uintptr_t)p | (uintptr_t)q) & 3) == 0) {
    uint32_t *a = (uint32_t *)p; const uint32_t *b = (const uint32_t *)q;
    while (n >= 16) { a[0] = b[0]; a[1] = b[1]; a[2] = b[2]; a[3] = b[3]; a += 4; b += 4; n -= 16; }
    while (n >= 4) { *a++ = *b++; n -= 4; }
    p = (unsigned char *)a; q = (const unsigned char *)b;
  }
  while (n--) *p++ = *q++;
  return d;
}
void *memmove(void *d, const void *s, size_t n) {
  unsigned char *p = d; const unsigned char *q = s;
  if (p < q) { while (n--) *p++ = *q++; } else { p += n; q += n; while (n--) *--p = *--q; }
  return d;
}
int memcmp(const void *a, const void *b, size_t n) { const unsigned char *p = a, *q = b; for (; n; n--, p++, q++) if (*p != *q) return *p - *q; return 0; }
size_t strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
void abort(void) { while (1) { } }

static float sin_core(float x) { /* |x| <= pi/2 */
  float x2 = x * x;
  return x * (1.f + x2 * (-1.f / 6.f + x2 * (1.f / 120.f + x2 * (-1.f / 5040.f + x2 * (1.f / 362880.f)))));
}
float sinf(float x) {
  const float twopi = 6.28318530718f, pi = 3.14159265359f, hpi = 1.57079632679f;
  float k = __builtin_roundf(x / twopi);
  x -= k * twopi;                 /* [-pi, pi] */
  if (x > hpi) x = pi - x;
  else if (x < -hpi) x = -pi - x;
  return sin_core(x);
}
float cosf(float x) { return sinf(x + 1.57079632679f); }
float tanf(float x) { float c = cosf(x); return sinf(x) / (c == 0.f ? 1e-30f : c); }

/* C++ support */
void __cxa_atexit(void) { }
void *__dso_handle = 0;
void _ZdlPv(void *p) { (void)p; }          /* operator delete(void*) */
void _ZdlPvj(void *p, unsigned n) { (void)p; (void)n; } /* operator delete(void*, unsigned) */

extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);
void __libc_init_array(void) {
  for (void (**f)(void) = __init_array_start; f < __init_array_end; f++) (*f)();
}
int atexit(void (*f)(void)) { (void)f; return 0; }
