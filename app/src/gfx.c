#include <stdarg.h>
#include "gfx.h"

Target T;

uint32_t mix(uint32_t a, uint32_t b, float t) {
    int k = (int)(clampf(t, 0.f, 1.f) * 256.f);
    uint32_t rb = ((((b & 0xff00ff) * k) + ((a & 0xff00ff) * (256 - k))) >> 8) & 0xff00ff;
    uint32_t g = ((((b & 0xff00) * k) + ((a & 0xff00) * (256 - k))) >> 8) & 0xff00;
    return 0xff000000u | rb | g;
}
uint32_t scale_rgb(uint32_t c, float k) {
    int r = (int)((c & 255) * k), g = (int)(((c >> 8) & 255) * k), b = (int)(((c >> 16) & 255) * k);
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    return RGB(r, g, b);
}
static inline void put(int x, int y, uint32_t c, int a) { /* a 0..256, y in target space */
    uint32_t *p = T.px + (size_t)y * T.stride + x;
    if (a >= 256) { *p = c; return; }
    uint32_t d = *p;
    uint32_t rb = ((((c & 0xff00ff) * a) + ((d & 0xff00ff) * (256 - a))) >> 8) & 0xff00ff;
    uint32_t g = ((((c & 0xff00) * a) + ((d & 0xff00) * (256 - a))) >> 8) & 0xff00;
    *p = 0xff000000u | rb | g;
}
/* clip a box (canvas coordinates) to the target */
static int clip(float x0, float y0, float x1, float y1, int *ix0, int *iy0, int *ix1, int *iy1) {
    y0 -= T.oy; y1 -= T.oy;
    int a = (int)x0, b = (int)y0, c = (int)x1 + 1, d = (int)y1 + 1;
    if (x0 < 0) a = 0;
    if (y0 < 0) b = 0;
    if (c > T.w) c = T.w;
    if (d > T.h) d = T.h;
    if (a >= c || b >= d) return 0;
    *ix0 = a; *iy0 = b; *ix1 = c; *iy1 = d;
    return 1;
}

void fill_rect(float x, float y, float w, float h, uint32_t c, float a) {
    int x0, y0, x1, y1;
    if (!clip(x, y, x + w - 1, y + h - 1, &x0, &y0, &x1, &y1)) return;
    int ia = (int)(a * 256.f);
    for (int j = y0; j < y1; j++) for (int i = x0; i < x1; i++) put(i, j, c, ia);
}

void round_rect(float x, float y, float w, float h, float r, uint32_t c, float a) {
    int x0, y0, x1, y1;
    if (!clip(x, y, x + w, y + h, &x0, &y0, &x1, &y1)) return;
    float cx0 = x + r, cx1 = x + w - r, cy0 = y + r, cy1 = y + h - r;
    for (int j = y0; j < y1; j++) {
        float py = j + 0.5f + T.oy;
        for (int i = x0; i < x1; i++) {
            float px = i + 0.5f;
            float dx = px < cx0 ? cx0 - px : (px > cx1 ? px - cx1 : 0.f);
            float dy = py < cy0 ? cy0 - py : (py > cy1 ? py - cy1 : 0.f);
            float cov;
            if (dx == 0.f && dy == 0.f) {
                float ex = px - x < x + w - px ? px - x : x + w - px;
                float ey = py - y < y + h - py ? py - y : y + h - py;
                float e = ex < ey ? ex : ey;
                cov = clampf(e + 0.5f, 0.f, 1.f);
            } else {
                cov = clampf(r - sqrt_(dx * dx + dy * dy) + 0.5f, 0.f, 1.f);
            }
            if (cov > 0.f) put(i, j, c, (int)(cov * a * 256.f));
        }
    }
}

void ring(float cx, float cy, float r0, float r1, uint32_t c, float a) {
    int x0, y0, x1, y1;
    if (!clip(cx - r1 - 1, cy - r1 - 1, cx + r1 + 1, cy + r1 + 1, &x0, &y0, &x1, &y1)) return;
    float o2 = (r1 + 0.5f) * (r1 + 0.5f), i2 = r0 > 0.5f ? (r0 - 0.5f) * (r0 - 0.5f) : 0.f;
    float fo2 = (r1 - 0.5f) * (r1 - 0.5f), fi2 = (r0 + 0.5f) * (r0 + 0.5f);
    for (int j = y0; j < y1; j++) {
        float dy = j + 0.5f + T.oy - cy;
        for (int i = x0; i < x1; i++) {
            float dx = i + 0.5f - cx, d2 = dx * dx + dy * dy;
            if (d2 > o2 || d2 < i2) continue;
            float cov;
            if (d2 <= fo2 && (r0 <= 0.f || d2 >= fi2)) cov = 1.f;
            else {
                float d = sqrt_(d2);
                cov = clampf(r1 - d + 0.5f, 0.f, 1.f);
                if (r0 > 0.f) cov *= clampf(d - r0 + 0.5f, 0.f, 1.f);
            }
            put(i, j, c, (int)(cov * a * 256.f));
        }
    }
}
void disc(float cx, float cy, float r, uint32_t c, float a) { ring(cx, cy, 0.f, r, c, a); }

void glow(float cx, float cy, float r, uint32_t c, float a) {
    int x0, y0, x1, y1;
    if (!clip(cx - r, cy - r, cx + r, cy + r, &x0, &y0, &x1, &y1)) return;
    float k = 1.f / (r * r);
    for (int j = y0; j < y1; j++) {
        float dy = j + 0.5f + T.oy - cy;
        for (int i = x0; i < x1; i++) {
            float dx = i + 0.5f - cx, t = 1.f - (dx * dx + dy * dy) * k;
            if (t <= 0.f) continue;
            put(i, j, c, (int)(t * t * a * 256.f));
        }
    }
}

void line(float x0, float y0, float x1, float y1, float w, uint32_t c, float a) {
    float hw = w * 0.5f;
    int bx0, by0, bx1, by1;
    float mnx = (x0 < x1 ? x0 : x1) - hw - 1, mxx = (x0 > x1 ? x0 : x1) + hw + 1;
    float mny = (y0 < y1 ? y0 : y1) - hw - 1, mxy = (y0 > y1 ? y0 : y1) + hw + 1;
    if (!clip(mnx, mny, mxx, mxy, &bx0, &by0, &bx1, &by1)) return;
    float vx = x1 - x0, vy = y1 - y0, l2 = vx * vx + vy * vy;
    float inv = l2 > 0.f ? 1.f / l2 : 0.f;
    for (int j = by0; j < by1; j++) {
        float py = j + 0.5f + T.oy;
        for (int i = bx0; i < bx1; i++) {
            float px = i + 0.5f;
            float t = ((px - x0) * vx + (py - y0) * vy) * inv;
            t = clampf(t, 0.f, 1.f);
            float dx = px - (x0 + vx * t), dy = py - (y0 + vy * t);
            float d2 = dx * dx + dy * dy;
            if (d2 > (hw + 0.5f) * (hw + 0.5f)) continue;
            float cov = clampf(hw - sqrt_(d2) + 0.5f, 0.f, 1.f);
            if (cov > 0.f) put(i, j, c, (int)(cov * a * 256.f));
        }
    }
}

void dotted(float x0, float y0, float x1, float y1, float gap, float r, uint32_t c) {
    float dx = x1 - x0, dy = y1 - y0, l = sqrt_(dx * dx + dy * dy);
    int n = (int)(l / gap);
    for (int i = 0; i <= n; i++) {
        float t = n ? (float)i / n : 0.f;
        disc(x0 + dx * t, y0 + dy * t, r, c, 1.f);
    }
}
void dotted_curve(float x0, float y0, float cx, float cy, float x1, float y1, float gap, float r, uint32_t c) {
    /* quadratic bezier, dots spaced by arc length */
    float px = x0, py = y0, acc = gap;
    for (int i = 0; i <= 400; i++) {
        float t = i / 400.f, u = 1.f - t;
        float x = u * u * x0 + 2 * u * t * cx + t * t * x1, y = u * u * y0 + 2 * u * t * cy + t * t * y1;
        float d = sqrt_((x - px) * (x - px) + (y - py) * (y - py));
        acc += d; px = x; py = y;
        if (acc >= gap) { disc(x, y, r, c, 1.f); acc = 0.f; }
    }
}

/* a lit plastic dome: light from the top left */
void shaded_disc(float cx, float cy, float r, uint32_t base, float light, float a) {
    int x0, y0, x1, y1;
    if (!clip(cx - r - 1, cy - r - 1, cx + r + 1, cy + r + 1, &x0, &y0, &x1, &y1)) return;
    float ir = 1.f / r;
    uint32_t hi = scale_rgb(base, 1.f + 0.12f * light), lo = scale_rgb(base, 1.f - 0.22f * light);
    for (int j = y0; j < y1; j++) {
        float dy = (j + 0.5f + T.oy - cy) * ir;
        for (int i = x0; i < x1; i++) {
            float dx = (i + 0.5f - cx) * ir, d2 = dx * dx + dy * dy;
            float rr = (r + 0.5f) * ir;
            if (d2 > rr * rr) continue;
            float d = sqrt_(d2);
            float cov = clampf((1.f - d) * r + 0.5f, 0.f, 1.f);
            float l = 0.5f - 0.45f * (dx * 0.6f + dy * 0.8f);      /* directional */
            l -= 0.25f * d * d * d;                                 /* rim darkening */
            uint32_t col = mix(lo, hi, l);
            put(i, j, col, (int)(cov * a * 256.f));
        }
    }
}

/* ---------------------------------------------------------------- text */
int text_width(const Font *f, const char *s) {
    int w = 0;
    for (; *s; s++) { int c = (unsigned char)*s; if (c >= 32 && c < 127) w += f->g[c - 32].adv; }
    return w;
}
int text(const Font *f, float x, float y, const char *s, uint32_t c, float a, int align) {
    int w = text_width(f, s);
    float sx = align == AL_CENTER ? x - w * 0.5f : (align == AL_RIGHT ? x - w : x);
    int ox = (int)(sx + 0.5f), top = (int)(y - f->height * 0.5f + 0.5f - T.oy);
    for (; *s; s++) {
        int ch = (unsigned char)*s;
        if (ch < 32 || ch >= 127) continue;
        const Glyph *g = &f->g[ch - 32];
        const uint8_t *px = f->px + g->off;
        for (int j = 0; j < g->h; j++) {
            int yy = top + g->oy + j;
            if (yy < 0 || yy >= T.h) continue;
            for (int i = 0; i < g->w; i++) {
                int xx = ox + g->ox + i;
                if (xx < 0 || xx >= T.w) continue;
                int v = px[j * g->w + i];
                if (v) put(xx, yy, c, (int)(v * a * (256.f / 255.f)));
            }
        }
        ox += g->adv;
    }
    return w;
}
int text_wrap(const Font *f, float x, float y, float w, float lh, const char *s, uint32_t c, float a) {
    char lb[256];
    float yy = y;
    const char *p = s;
    while (*p) {
        int n = 0;
        const char *q = p;
        while (*q && *q != '\n') {
            const char *we = q;
            while (*we && *we != ' ' && *we != '\n') we++;
            int m = n;
            for (const char *r = q; r < we && m < 250; r++) lb[m++] = *r;
            lb[m] = 0;
            if (n > 0 && text_width(f, lb) > w) break;
            n = m; q = we;
            while (*q == ' ') { if (n < 250) lb[n++] = ' '; q++; }
        }
        while (n > 0 && lb[n - 1] == ' ') n--;
        lb[n] = 0;
        text(f, x, yy + lh * 0.5f, lb, c, a, AL_LEFT);
        yy += lh;
        if (*q == '\n') q++;
        if (q == p) q++;
        p = q;
    }
    return (int)(yy - y);
}

/* ---------------------------------------------------------------- fmt */
static int put_str(char *o, int p, int n, const char *s) { while (*s && p < n - 1) o[p++] = *s++; return p; }
int fmt(char *out, int n, const char *f, ...) {
    va_list ap; va_start(ap, f);
    int p = 0;
    for (; *f && p < n - 1; f++) {
        if (*f != '%') { out[p++] = *f; continue; }
        f++;
        int prec = 2, width = 0;
        while (*f >= '0' && *f <= '9') { width = width * 10 + (*f - '0'); f++; }
        if (*f == '.') { f++; prec = *f - '0'; f++; }
        char tmp[40]; int t = 0;
        if (*f == 'd') {
            int v = va_arg(ap, int); unsigned u = v < 0 ? -v : v;
            char r[16]; int k = 0;
            do { r[k++] = '0' + u % 10; u /= 10; } while (u);
            if (v < 0) r[k++] = '-';
            while (k < width) r[k++] = ' ';
            while (k) tmp[t++] = r[--k];
        } else if (*f == 'f') {
            double v = va_arg(ap, double);
            if (v < 0) { tmp[t++] = '-'; v = -v; }
            double m = 1; for (int i = 0; i < prec; i++) m *= 10;
            long long q = (long long)(v * m + 0.5);
            long long ip = q / (long long)m, fp = q % (long long)m;
            char r[24]; int k = 0;
            do { r[k++] = '0' + ip % 10; ip /= 10; } while (ip);
            while (k) tmp[t++] = r[--k];
            if (prec) {
                tmp[t++] = '.';
                for (int i = prec - 1; i >= 0; i--) { long long d = fp; for (int j = 0; j < i; j++) d /= 10; tmp[t++] = '0' + d % 10; }
            }
        } else if (*f == 's') {
            p = put_str(out, p, n, va_arg(ap, const char *)); continue;
        } else if (*f == 'c') {
            tmp[t++] = (char)va_arg(ap, int);
        } else tmp[t++] = *f;
        tmp[t] = 0;
        p = put_str(out, p, n, tmp);
    }
    out[p] = 0;
    va_end(ap);
    return p;
}
