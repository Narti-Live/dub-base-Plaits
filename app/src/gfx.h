/* Anti-aliased software drawing on a 32-bit canvas (Android RGBX_8888 byte order). */
#ifndef GFX_H
#define GFX_H
#include <stdint.h>
#include <stddef.h>
#include "fonts.h"

#define RGB(r, g, b) (0xff000000u | ((uint32_t)(b) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(r))
#define HEX(h) RGB(((h) >> 16) & 255, ((h) >> 8) & 255, (h) & 255)

typedef struct { uint32_t *px; int w, h, stride; float oy; } Target;
extern Target T;

static inline float clampf(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }
static inline float sqrt_(float x) { return __builtin_sqrtf(x); }
uint32_t mix(uint32_t a, uint32_t b, float t);       /* a..b */
uint32_t scale_rgb(uint32_t c, float k);

void fill_rect(float x, float y, float w, float h, uint32_t c, float a);
void round_rect(float x, float y, float w, float h, float r, uint32_t c, float a);
void disc(float cx, float cy, float r, uint32_t c, float a);
void ring(float cx, float cy, float r0, float r1, uint32_t c, float a);
void glow(float cx, float cy, float r, uint32_t c, float a);   /* soft radial falloff */
void line(float x0, float y0, float x1, float y1, float w, uint32_t c, float a);
void dotted(float x0, float y0, float x1, float y1, float gap, float r, uint32_t c);
void dotted_curve(float x0, float y0, float cx, float cy, float x1, float y1, float gap, float r, uint32_t c);
void shaded_disc(float cx, float cy, float r, uint32_t base, float light, float a);

enum { AL_LEFT, AL_CENTER, AL_RIGHT };
int text_width(const Font *f, const char *s);
/* y is the vertical centre of the text line */
int text(const Font *f, float x, float y, const char *s, uint32_t c, float a, int align);
/* word-wrapped text, returns the height used */
int text_wrap(const Font *f, float x, float y, float w, float lh, const char *s, uint32_t c, float a);

/* tiny printf replacement: %d %s %c %.Nf only */
int fmt(char *out, int n, const char *f, ...);
#endif
