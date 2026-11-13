// STASIS engine — a tempo-free generative ambient voice. Copyright 2026 Narti Live. MIT license.
// Original code. Inspired by the idea of "cloud" oscillator modules such as the Synthesis Technology E352;
// not affiliated with Synthesis Technology or with any record label.
//
// Everything swells: no attack is shorter than about a second, nothing follows a tempo, so it sits under a
// track at any BPM. G natural minor at TONAL 0 %; the knob blends to microtonal and inharmonic at 100 %.
//
// Signal: CLOUD (generative swells, up to 8 voices x 4 wavetable partials) + DRONE (root / fifth / sub)
//         -> FILTER (LP..BP..HP) -> wow / flutter
//         + TEXTURE (wind, crackle, metal resonator, rumble)
//         -> delay + FDN reverb (freeze) -> drive, tilt, low cut, width, limiter.
// Slow random modulation: 4 drift lanes (smooth random walks, 15 s .. 4 min) and a wandering GUIDE point.

var PARAMS = [
  // eight macros: each one morphs several settings at once
  ['timbre', 0.3], ['density', 0.45], ['motion', 0.45], ['texture', 0.35], ['filter', 0.55], ['space', 0.55], ['drone', 0.45], ['register', 0.4],
  // harmony, guide, kick space (low cut at 120 Hz)
  ['atonal', 0.0], ['gx', 0.35], ['gy', 0.4], ['kick', 0]
];
var LA = 144, CEIL = 0.708, TARGET = 0.09;   // look-ahead 3 ms, ceiling -3 dBFS, loudness -20 dBFS RMS (above 60 Hz)
var KB0, KB1, KA1, KA2;
(function () { var w0 = 2 * Math.PI * 100 / sampleRate, al = Math.sin(w0) / (2 * 0.707), cw = Math.cos(w0), a0 = 1 + al;
  KB0 = (1 + cw) / 2 / a0; KB1 = -(1 + cw) / a0; KA1 = -2 * cw / a0; KA2 = (1 - al) / a0; })();
var PIX = {}; PARAMS.forEach(function (p, i) { PIX[p[0]] = i; });

var SCALE = [0, 2, 3, 5, 7, 8, 10];               // G natural minor (from G)
var WEIGHT = [6, 1.3, 3.2, 1.6, 4.5, 2.2, 2];       // root, 2, b3, 4, 5, b6, b7
var GROOT = 31;                                     // G1 (MIDI)

// HARMONY: one knob through six stations, each blending into the next.
//   w: chance of each of the 12 semitones above G; r: the 4 partials of a swell (ratios); dt: detune in cents;
//   mic: random offset in semitones (microtones); df: the drone's second tone (ratio)
var M5 = 1.4983;
var STATIONS = [
  { name: 'G minor',    w: [6, 0, 1.3, 3.2, 0, 1.6, 0, 4.5, 2.2, 0, 2, 0], r: [1, 1, 2, 0.5],          dt: 3,  mic: 0,    df: M5 },
  { name: 'detune',     w: [6, 0, 1.3, 3.2, 0, 1.6, 0, 4.5, 2.2, 0, 2, 0], r: [1, 1, 2, 0.5],          dt: 34, mic: 0,    df: M5 },
  { name: 'fifths',     w: [8, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0],            r: [1, M5, 2, 0.5 * M5],    dt: 12, mic: 0,    df: M5 },
  { name: 'octaves',    w: [10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],           r: [1, 2, 4, 0.5],          dt: 6,  mic: 0,    df: 2 },
  { name: 'microtonal', w: [6, 0, 1.3, 3.2, 0, 1.6, 0, 4.5, 2.2, 0, 2, 0], r: [1, 1.031, 2.04, 0.493],   dt: 18, mic: 0.5,  df: M5 * 1.024 },
  { name: 'atonal',     w: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],            r: [1, 1.37, 2.756, 0.532],  dt: 30, mic: 6,    df: 1.4142 }
];
function harmony(a) {   // a 0..1 -> the blend of the two nearest stations
  var x = Math.max(0, Math.min(0.99999, a)) * (STATIONS.length - 1), i = Math.floor(x), f = x - i, A = STATIONS[i], B = STATIONS[Math.min(STATIONS.length - 1, i + 1)], k;
  var h = { w: [], r: [], dt: A.dt + (B.dt - A.dt) * f, mic: A.mic + (B.mic - A.mic) * f, df: Math.exp(Math.log(A.df) + (Math.log(B.df) - Math.log(A.df)) * f) };
  for (k = 0; k < 12; k++) h.w.push(A.w[k] + (B.w[k] - A.w[k]) * f);
  for (k = 0; k < 4; k++) h.r.push(Math.exp(Math.log(A.r[k]) + (Math.log(B.r[k]) - Math.log(A.r[k])) * f));
  return h;
}

// ---------------------------------------------------------------- wavetables (4 shapes x 9 band limits)
var TBL = 2048, NSH = 4, MIPS = 9, MIPH = [256, 128, 64, 32, 16, 8, 4, 2, 1];
var TABLES = [];
(function () {
  var seed = 7; function rnd() { seed = (seed * 16807) % 2147483647; return seed / 2147483647; }
  var ph = []; for (var n = 0; n <= 256; n++) ph.push(rnd() * 6.283);
  function amp(s, n) {
    if (s === 0) return n === 1 ? 1 : n === 2 ? 0.1 : n === 3 ? 0.04 : 0;                       // BREATH
    if (s === 1) return n % 2 ? 1 / Math.pow(n, 1.6) : 0.12 / (n * n);                         // GLASS
    if (s === 2) return 0.5 / n + 0.8 * Math.exp(-Math.pow((n - 4) / 1.6, 2)) + 0.45 * Math.exp(-Math.pow((n - 9) / 2.6, 2)); // CHOIR
    return (0.6 + 0.4 * Math.sin(n * 1.7)) / Math.pow(n, 0.95);                                  // IRON
  }
  for (var s = 0; s < NSH; s++) {
    TABLES.push([]);
    for (var m = 0; m < MIPS; m++) {
      var t = new Float32Array(TBL + 1), pk = 0, i, h;
      for (h = 1; h <= MIPH[m]; h++) {
        var a = amp(s, h); if (a < 1e-5) continue;
        var p0 = s === 3 ? ph[h] : 0;
        for (i = 0; i < TBL; i++) t[i] += a * Math.sin(6.283185307 * h * i / TBL + p0);
      }
      for (i = 0; i < TBL; i++) pk = Math.max(pk, Math.abs(t[i]));
      for (i = 0; i < TBL; i++) t[i] /= pk || 1;
      t[TBL] = t[0];
      TABLES[s].push(t);
    }
  }
})();
function mipFor(f, sr) { var lim = sr * 0.45 / Math.max(1, f); for (var m = 0; m < MIPS; m++) if (MIPH[m] <= lim) return m; return MIPS - 1; }

// ---------------------------------------------------------------- helpers
var nz = 22222;
function noise() { nz ^= nz << 13; nz ^= nz >>> 17; nz ^= nz << 5; return (nz >>> 0) / 2147483648 - 1; }
function th(x) { if (x > 3) return 1; if (x < -3) return -1; var x2 = x * x; return x * (27 + x2) / (27 + 9 * x2); }
function clamp(x, a, b) { return x < a ? a : x > b ? b : x; }
function lerp(a, b, t) { return a + (b - a) * t; }
function rdTab(da, db, f, p) { var x = p * TBL, k = x | 0, q = x - k, u = da[k] + (da[k + 1] - da[k]) * q, w = db[k] + (db[k + 1] - db[k]) * q; return u + (w - u) * f; }
function mtof(m) { return 440 * Math.pow(2, (m - 69) / 12); }

// a smooth random walk: new target every period (jittered), cosine glide between
function Walker(seed) { this.a = Math.random() * 2 - 1; this.b = Math.random() * 2 - 1; this.t = 0; this.T = 30; this.v = this.a; this.s = seed || 1; }
Walker.prototype.step = function (dt, period) {
  this.t += dt;
  if (this.t >= this.T) { this.t = 0; this.a = this.b; this.b = Math.random() * 2 - 1; this.T = period * (0.6 + Math.random() * 0.8); }
  var x = this.t / this.T; this.v = this.a + (this.b - this.a) * (0.5 - 0.5 * Math.cos(Math.PI * x));
  return this.v;
};

function SVF() { this.ic1 = 0; this.ic2 = 0; this.lp = 0; this.bp = 0; this.hp = 0; }
SVF.prototype.run = function (x, g, k) {   // TPT state-variable filter (Simper)
  var a1 = 1 / (1 + g * (g + k)), a2 = g * a1, a3 = g * a2;
  var v3 = x - this.ic2, v1 = a1 * this.ic1 + a2 * v3, v2 = this.ic2 + a2 * this.ic1 + a3 * v3;
  this.ic1 = 2 * v1 - this.ic1; this.ic2 = 2 * v2 - this.ic2;
  this.lp = v2; this.bp = v1; this.hp = x - k * v1 - v2;
  return v2;
};

function Line(n) { this.b = new Float32Array(n); this.n = n; this.w = 0; }
Line.prototype.write = function (x) { this.b[this.w] = x; this.w = (this.w + 1) % this.n; };
Line.prototype.read = function (d) {   // d samples back (fractional)
  var p = this.w - 1 - d; while (p < 0) p += this.n;
  var i = p | 0, f = p - i, a = this.b[i], c = this.b[(i + 1) % this.n];
  return a + (c - a) * f;
};
Line.prototype.readi = function (d) { var p = this.w - 1 - d; if (p < 0) p += this.n; return this.b[p]; };

// ---------------------------------------------------------------- the processor
class Stasis extends AudioWorkletProcessor {
  constructor() {
    super();
    var sr = sampleRate, self = this; this.sr = sr;
    this.P = new Float32Array(PARAMS.length); this.T = new Float32Array(PARAMS.length);   // current, target
    PARAMS.forEach(function (p, i) { self.P[i] = p[1]; self.T[i] = p[1]; });
    this.glide = new Float32Array(PARAMS.length);   // per-param glide rate (per control tick), 0 = smoothing only
    this.awake = false; this.fade = 0; this.fade2 = 0; this.freeze = false; this.frz = 0;
    // cloud
    this.V = [];
    for (var v = 0; v < 8; v++) this.V.push({ on: false, f: [0, 0, 0, 0], inc: [0, 0, 0, 0], ph: [Math.random(), Math.random(), Math.random(), Math.random()], amp: [0.5, 0.35, 0.18, 0.25],
      env: 0, stage: 3, t: 0, atk: 1, hold: 1, relc: 0.9999, gain: 0, gl: 0, gr: 0, lp: 0, lpr: 0, lpk: 0.1, midi: 60, vib: Math.random() * 6.28, vibr: 0.1, mip: 4, x: 0 });
    this.nextEv = sr * 1.5; this.memory = []; this.events = 0;
    // drone
    this.dph = new Float64Array(7); this.dlfo = 0;
    // filters
    this.fl = new SVF(); this.fr = new SVF(); this.wl = new SVF(); this.wr = new SVF();
    this.hp1 = [0, 0, 0, 0]; this.hp2 = [0, 0, 0, 0];   // low cut biquad states L/R
    this.tiltL = 0; this.tiltR = 0;
    // wow
    this.wowL = new Line(4096); this.wowR = new Line(4096); this.wph = 0; this.fph = 0;
    // texture
    this.crk = [0, 0]; this.crkEnv = 0; this.crkPan = 0.5; this.rum = [0, 0, 0, 0];
    this.combL = new Line(4096); this.combR = new Line(4096); this.cdl = 0; this.cdr = 0;
    this.apm = [new Line(512), new Line(512), new Line(512), new Line(512)];
    this.exLp = 0; this.exLp2 = 0;
    // delay
    this.dlL = new Line(Math.ceil(sr * 2.6)); this.dlR = new Line(Math.ceil(sr * 2.6)); this.dLp = [0, 0]; this.dHp = [0, 0]; this.dTs = 0.6 * sr;
    // reverb (FDN 8 + input diffusion)
    var base = [1557, 1617, 1491, 1422, 1277, 1356, 1188, 1116];
    this.fdnBase = base.map(function (x) { return x * sr / 44100; });
    this.fdn = base.map(function () { return new Line(16384); });
    this.fdnLp = new Float32Array(8); this.fdnLen = new Float32Array(8); this.fdnG = new Float32Array(8); this.fdnLenT = new Float32Array(8);
    this.dif = [new Line(1024), new Line(1024), new Line(1024), new Line(1024)]; this.difLen = [142, 107, 379, 277].map(function (x) { return Math.round(x * sr / 44100); });
    this.vmod = 0;
    // master
    this.lim = 1; this.peak = 0; this.rms = 0; this.agc = 1; this.agMs = 0; this.kw = new Float32Array(4); this.lenv = 0;
    this.la = new Float32Array(LA * 2); this.lap = 0;
    // modulation
    this.W = [new Walker(1), new Walker(2), new Walker(3), new Walker(4), new Walker(5), new Walker(6), new Walker(7), new Walker(8), new Walker(9)];
    this.lane = new Float32Array(4); this.wx = 0.35; this.wy = 0.4;
    this.k = 0; this.D = {}; this.derive();
    this.postIn = 0;
    this.port.onmessage = function (e) { self.msg(e.data); };
  }
  msg(m) {
    var i, k;
    if (m.t === 'set') { for (k in m.p) if (PIX[k] != null) { i = PIX[k]; this.T[i] = m.p[k]; this.glide[i] = 0; } }
    else if (m.t === 'glide') {   // move to new values over m.s seconds
      var ticks = Math.max(1, m.s * this.sr / 64);
      for (k in m.p) if (PIX[k] != null) { i = PIX[k]; this.T[i] = m.p[k]; this.glide[i] = Math.abs(this.T[i] - this.P[i]) / ticks; }
    }
    else if (m.t === 'awake') { this.awake = !!m.v; if (this.awake && this.nextEv > this.sr * 3) this.nextEv = this.sr * 0.8; }
    else if (m.t === 'freeze') this.freeze = !!m.v;
    else if (m.t === 'spawn') this.spawn();
  }
  p(n) { return this.P[PIX[n]]; }

  // ---- control rate: smoothing, glides, drift lanes, guide, derived values
  derive() {
    var sr = this.sr, P = this.P, T = this.T, G = this.glide, i, dt = 64 / sr;
    for (i = 0; i < P.length; i++) {
      if (G[i] > 0) { var d = T[i] - P[i]; if (Math.abs(d) <= G[i]) { P[i] = T[i]; G[i] = 0; } else P[i] += d > 0 ? G[i] : -G[i]; }
      else P[i] += (T[i] - P[i]) * 0.08;
    }
    // ---- the macros, unfolded into the voice's settings
    var M = {}; for (i = 0; i < PARAMS.length; i++) M[PARAMS[i][0]] = P[i];
    var t = M.timbre, de = M.density, mo = M.motion, tx = M.texture, fi = M.filter, sp = M.space, bal = M.drone, rg = M.register;
    var q = {
      gx: M.gx, gy: M.gy, atonal: M.atonal,
      // TIMBRE: breath > glass > choir > iron, then wider and more metallic
      shape: Math.min(1, t * 1.12), dShape: clamp(t * 0.85, 0, 1), spread: 0.15 + 0.7 * t, upper: 0.15 + 0.5 * t,
      // DENSITY: sparse long swells .. many shorter ones (never shorter than ~1.5 s)
      density: 0.12 + 0.88 * de, swell: 0.85 - 0.5 * de, tail: 0.9 - 0.35 * de, range: 0.2 + 0.8 * de, voices: 0.35 + 0.65 * de, repeat: 0.5 - 0.2 * de,
      // MOTION: how much and how fast everything drifts
      depth: 0.15 + 0.85 * mo, speed: 0.08 + 0.55 * mo, wow: 0.05 + 0.45 * mo * mo, evolve: 0.2 + 0.7 * mo, windMove: 0.3 + 0.6 * mo, fDrift: 0.25 + 0.65 * mo,
      beat: 0.15 + 0.45 * mo, movement: 0.3 + 0.6 * mo,
      // TEXTURE: air > wind > dust > iron and rumble
      wind: tx < 0.55 ? tx * 1.2 : 0.66 - (tx - 0.55) * 0.5, windTone: 0.25 + 0.35 * tx, crackle: clamp((tx - 0.3) * 0.7, 0, 0.45),
      metal: clamp((tx - 0.45) * 1.1, 0, 0.6) + t * t * 0.25, metalDecay: 0.55 + 0.35 * tx, rumble: 0.15 + 0.4 * tx, texture: 0.35 + 0.5 * tx,
      // FILTER: dark low-pass > open > resonant band > thin high-pass
      fType: fi < 0.62 ? 0 : fi < 0.82 ? (fi - 0.62) / 0.2 * 0.5 : 0.5 + (fi - 0.82) / 0.18 * 0.5,
      cutoff: fi < 0.62 ? 0.5 + fi / 0.62 * 0.38 : fi < 0.82 ? 0.88 - (fi - 0.62) * 1.6 : 0.56 + (fi - 0.82) * 1.2,
      res: fi < 0.62 ? 0.15 + fi * 0.2 : fi < 0.82 ? 0.27 + (fi - 0.62) * 1.8 : 0.63 - (fi - 0.82) * 1.2,
      // SPACE: close and dry > huge > nearly endless
      verb: 0.15 + 0.7 * sp, size: 0.35 + 0.65 * sp, decay: 0.3 + 0.62 * sp, damp: 0.7 - 0.3 * sp, delay: 0.05 + 0.33 * sp, dFb: 0.3 + 0.4 * sp, dTime: 0.3 + 0.6 * sp, dTone: 0.25 + 0.2 * sp,
      // DRONE: cloud <> drone (the drone never quite leaves, so there are no holes in the sound)
      cloud: Math.min(1, 2 * (1 - bal)) * 0.85, drone: (0.3 + 0.7 * Math.min(1, 2 * bal)) * 0.8, sub: 0.2 + 0.45 * bal, fifth: 0.3 + 0.3 * bal,
      // REGISTER: deep <> high
      octave: rg, dOct: rg < 0.34 ? 0 : rg < 0.67 ? 0.5 : 1,
      // fixed (safe, good-sounding) settings
      lowcut: M.kick > 0.5 ? 0.66 : 0.12, tilt: 0.45, drive: 0.2, width: 0.65, level: 0.7
    };
    var p = function (n) { return q[n]; }, D = this.D;
    // guide wanders around the point the player sets (EVOLVE = how far)
    var gyR = p('gy');
    var period = 240 * Math.pow(0.0625, p('speed')) / (0.6 + gyR);         // 4 min .. 15 s, faster when restless
    for (i = 0; i < 4; i++) this.lane[i] = this.W[i].step(dt, period * [1, 1.3, 0.8, 1.7][i]);
    var ev = p('evolve');
    this.wx = clamp(p('gx') + this.W[4].step(dt, period * 1.5) * ev * 0.28, 0, 1);
    this.wy = clamp(gyR + this.W[5].step(dt, period * 1.9) * ev * 0.28, 0, 1);
    var gx = this.wx, gy = this.wy;
    var depth = p('depth') * (0.4 + 1.2 * gy), L = this.lane;
    D.depth = depth;
    // cloud
    D.rate = (2 + 28 * Math.pow(p('density'), 1.4)) / 60 * (0.35 + 1.5 * gy) * (1 + 0.35 * L[2] * depth);   // events per second
    D.atk = 1.2 + 18 * Math.pow(p('swell'), 1.5);            // s (never a pluck)
    D.rel = 2.5 + 30 * Math.pow(p('tail'), 1.6);
    D.oct = Math.round(p('octave') * 3);                     // G1..G4
    D.range = 1 + Math.round(p('range') * 2);
    D.shape = clamp(p('shape') * 3 + L[1] * depth * 0.6, 0, 2.999);
    D.spread = 2 + 38 * p('spread') * (1 + 0.4 * L[2] * depth);  // cents
    D.cloud = Math.pow(p('cloud'), 2) * 0.32;
    D.voices = 2 + Math.round(p('voices') * 6);
    D.atonal = p('atonal');
    D.repeat = p('repeat');
    D.bright = clamp(0.25 + gx * 0.75 + L[0] * depth * 0.2, 0.08, 1);
    // drone
    D.drone = Math.pow(p('drone'), 2) * 0.22;
    D.droot = mtof(GROOT + 12 * Math.round(p('dOct') * 2));  // G1 / G2 / G3
    var a = D.atonal; if (this.harmA !== a || !D.harm) { D.harm = harmony(a); this.harmA = a; if (Math.abs(a - (this.memA || 0)) > 0.04) { this.memory = []; this.memA = a; } }
    D.dfifth = D.harm.df; D.fifth = p('fifth');
    D.beat = 0.03 + 1.6 * Math.pow(p('beat'), 2) + D.harm.dt * 0.02;   // Hz: the detune station beats faster
    D.dshape = clamp(p('dShape') * 3 + L[1] * depth * 0.4, 0, 2.999);
    D.move = p('movement'); D.sub = Math.pow(p('sub'), 2) * 0.9; D.upper = p('upper') * (0.5 + 0.5 * L[3]);
    // texture
    D.wind = Math.pow(p('wind'), 2) * 1.6; D.wTone = 150 * Math.pow(2, p('windTone') * 4.5 + (gx - 0.5)); D.wMove = p('windMove');
    D.crackle = Math.pow(p('crackle'), 2) * (1 + gy); D.metal = Math.pow(p('metal'), 2) * (0.7 + 0.6 * gy);
    D.mFb = 0.9 + 0.095 * p('metalDecay'); D.rumble = Math.pow(p('rumble'), 2) * 3.5; D.texture = p('texture');
    var am = Math.max(0, (a - 0.6) / 0.4); D.mDelL = sr / (mtof(GROOT + 12) * Math.pow(2, am * 0.31)); D.mDelR = sr / (mtof(GROOT + 19) * Math.pow(2, -am * 0.23));
    // filter
    var ft = p('fType');
    D.fLP = clamp(1 - ft * 2, 0, 1); D.fBP = 1 - Math.abs(ft * 2 - 1); D.fHP = clamp(ft * 2 - 1, 0, 1);
    var co = 40 * Math.pow(2, p('cutoff') * 8.2 + (gx - 0.5) * 2.2 + L[0] * p('fDrift') * depth * 1.6);
    D.fg = Math.tan(Math.PI * clamp(co, 20, sr * 0.42) / sr); D.fk = 2 - 1.9 * Math.pow(p('res'), 0.8); D.cut = co;
    var lc = 20 * Math.pow(2, p('lowcut') * 3.65);           // 20..250 Hz
    var w0 = 2 * Math.PI * lc / sr, al = Math.sin(w0) / (2 * 0.707), cw = Math.cos(w0), a0 = 1 + al;
    D.hb0 = (1 + cw) / 2 / a0; D.hb1 = -(1 + cw) / a0; D.hb2 = D.hb0; D.ha1 = -2 * cw / a0; D.ha2 = (1 - al) / a0;
    D.tilt = (p('tilt') - 0.5) * 2 * (0.6 + 0.4 * gx) + (gx - 0.5) * 0.4; D.tk = 1 - Math.exp(-2 * Math.PI * 700 / sr);
    D.drive = 1 + p('drive') * 4;
    D.width = p('width') * 1.6;
    D.wow = p('wow') * (1 + 0.6 * gy);
    // space
    var size = 0.5 + 2.6 * p('size'), rt = 2 + 58 * Math.pow(p('decay'), 2);
    for (i = 0; i < 8; i++) { this.fdnLenT[i] = this.fdnBase[i] * size; if (!this.fdnLen[i]) this.fdnLen[i] = this.fdnLenT[i]; this.fdnLen[i] += (this.fdnLenT[i] - this.fdnLen[i]) * 0.002; this.fdnG[i] = Math.pow(10, -3 * this.fdnLen[i] / (sr * rt)); }
    this.frz += ((this.freeze ? 1 : 0) - this.frz) * 0.02;
    D.damp = 1 - Math.exp(-2 * Math.PI * (1200 * Math.pow(2, (1 - p('damp')) * 3.5 + (gx - 0.5))) / sr);
    D.verb = p('verb') * (1 + 0.25 * L[3] * depth); D.verb = clamp(D.verb, 0, 1);
    D.dTime = (0.28 + 1.9 * p('dTime')) * sr; D.dFb = 0.88 * p('dFb'); D.dTone = 1 - Math.exp(-2 * Math.PI * (600 * Math.pow(2, p('dTone') * 4)) / sr);
    D.delay = p('delay');
    D.level = Math.pow(p('level'), 2) * 2.0;
  }

  // ---- generative events: a new swell somewhere in the key (or out of it, with ATONAL)
  pickNote() {
    var D = this.D, mem = this.memory, n;
    if (mem.length >= 3 && Math.random() < D.repeat * 0.8) n = mem[Math.floor(Math.random() * mem.length)];
    else {
      var W = D.harm.w, tot = 0, i, r; for (i = 0; i < 12; i++) tot += W[i];
      r = Math.random() * tot; for (i = 0; i < 12; i++) { r -= W[i]; if (r <= 0) break; }
      n = GROOT + 12 * D.oct + 12 * Math.floor(Math.random() * D.range) + Math.min(11, i);
      mem.push(n); if (mem.length > 8) mem.shift();
    }
    // microtonal / atonal stations: drift off the grid
    if (D.harm.mic > 0) n += (Math.random() * 2 - 1) * D.harm.mic;
    return n;
  }
  spawn() {
    var D = this.D, V = this.V, best = -1, bestE = 9, i, active = 0;
    for (i = 0; i < 8; i++) if (V[i].on) active++;
    for (i = 0; i < 8; i++) if (!V[i].on && i < D.voices) { best = i; break; }
    if (best < 0) {     // steal the quietest voice that is already fading
      for (i = 0; i < 8; i++) if (V[i].stage === 2 && V[i].env < bestE) { bestE = V[i].env; best = i; }
      if (best < 0 || active > D.voices) return;
    }
    var v = V[best], midi = this.pickNote(), f = mtof(midi), hr = D.harm, sp = (D.spread + hr.dt) / 1200;
    var rat = [hr.r[0] * Math.pow(2, -sp * 0.25), hr.r[1] * Math.pow(2, sp * 0.75), hr.r[2] * Math.pow(2, -sp * 0.5), hr.r[3] * Math.pow(2, sp * 0.3)];
    for (i = 0; i < 4; i++) v.f[i] = f * rat[i];
    v.midi = midi; v.on = true; v.stage = v.env > 0.001 ? 0 : 0; v.t = 0;
    v.atk = D.atk * (0.7 + Math.random() * 0.6); v.hold = D.atk * (0.3 + Math.random() * 1.2);
    v.relc = Math.exp(Math.log(0.001) / (D.rel * (0.7 + Math.random() * 0.6) * this.sr));
    v.env0 = v.env;
    v.gain = (0.65 + Math.random() * 0.35) * (midi < 44 ? 0.75 : 1);
    var pan = 0.5 + (Math.random() - 0.5) * 0.9; v.gl = Math.cos(pan * 1.5708); v.gr = Math.sin(pan * 1.5708); v.x = pan;
    v.vibr = 0.05 + Math.random() * 0.15;
    v.mip = mipFor(f * 2.8, this.sr);
    this.events++;
  }

  process(inputs, outputs) {
    var out = outputs[0], L = out[0], R = out[1] || out[0], n = L.length, sr = this.sr;
    var D = this.D, V = this.V, TB = TABLES, i, j, s;
    for (s = 0; s < n; s++) {
      if ((this.k++ & 63) === 0) {
        this.derive();
        // per-voice pitch drift and increments
        var ish = Math.floor(D.shape), ifr = D.shape - ish;
        this.sh0 = ish; this.shf = ifr;
        for (i = 0; i < 8; i++) {
          var v = V[i]; if (!v.on) continue;
          v.vib += v.vibr * 6.283 * 64 / sr;
          var cents = Math.sin(v.vib) * (4 + 2 * D.harm.mic + D.harm.dt * 0.15) / 1200;
          var w = Math.pow(2, cents);
          for (j = 0; j < 4; j++) v.inc[j] = v.f[j] * w / sr;
          v.lpk = 1 - Math.exp(-2 * Math.PI * Math.min(sr * 0.4, v.f[0] * (1.5 + 10 * D.bright)) / sr);
        }
        // fade in/out of the whole instrument
        this.fade += ((this.awake ? 1 : 0) - this.fade) * (this.awake ? 0.0016 : 0.0009);
        this.fade2 += ((this.awake ? 1 : 0) - this.fade2) * (this.awake ? 0.01 : 0.0006);   // the tails fade too
        this.dlfo += 64 / sr;
        // loudness control: only while awake and playing, never boosting silence, +-12 dB at most
        if (this.awake && this.fade > 0.97 && this.agMs > 1e-7) {
          var gt = clamp(TARGET / Math.sqrt(this.agMs), 0.18, 5.6);   // +-15 dB
          this.agc += (gt - this.agc) * (gt < this.agc ? 0.006 : 0.0025);
        }
      }
      // ---- generative clock (free time, Poisson)
      if (this.awake && --this.nextEv <= 0) { this.spawn(); this.nextEv = sr * (-Math.log(1 - Math.random() * 0.98) / D.rate); this.nextEv = Math.max(sr * 0.4, this.nextEv); }

      // ---- cloud
      var cl = 0, cr = 0, t0 = TB[this.sh0], t1 = TB[Math.min(3, this.sh0 + 1)], f = this.shf;
      for (i = 0; i < 8; i++) {
        var v = V[i]; if (!v.on) continue;
        // envelope: raised-cosine swell, hold, slow exponential release
        if (v.stage === 0) { v.t += 1 / sr; var x = v.t / v.atk; if (x >= 1) { v.stage = 1; v.t = 0; v.env = 1; } else v.env = v.env0 + (1 - v.env0) * (0.5 - 0.5 * Math.cos(Math.PI * x)); }
        else if (v.stage === 1) { v.t += 1 / sr; if (v.t >= v.hold) v.stage = 2; }
        else { v.env *= v.relc; if (v.env < 0.0005) { v.on = false; v.env = 0; v.stage = 3; continue; } }
        var a0 = t0[v.mip], a1 = t1[v.mip], sum = 0;
        for (j = 0; j < 4; j++) {
          var ph = v.ph[j] + v.inc[j]; if (ph >= 1) ph -= 1; v.ph[j] = ph;
          var idx = ph * TBL, ii = idx | 0, fr = idx - ii;
          var sa = a0[ii] + (a0[ii + 1] - a0[ii]) * fr, sb = a1[ii] + (a1[ii + 1] - a1[ii]) * fr;
          sum += (sa + (sb - sa) * f) * v.amp[j];
        }
        v.lp += (sum - v.lp) * v.lpk; v.lpr += (v.lp - v.lpr) * v.lpk;
        var y = v.lpr * v.env * v.gain;
        cl += y * v.gl; cr += y * v.gr;
      }
      cl *= D.cloud; cr *= D.cloud;

      // ---- drone: root L/R beating, fifth, upper octave, sub
      var dp = this.dph, rf = D.droot, bh = D.beat * 0.5, mv = D.move;
      var amod = 1 - mv * 0.45 * (0.5 + 0.5 * Math.sin(this.dlfo * 6.283 * 0.031));
      dp[0] += (rf - bh) / sr; dp[1] += (rf + bh) / sr; dp[2] += (rf * D.dfifth + bh * 0.7) / sr; dp[3] += (rf * D.dfifth - bh * 0.7) / sr;
      dp[4] += rf * 2.003 / sr; dp[5] += rf * 1.997 / sr; dp[6] += rf * 0.5 / sr;
      for (j = 0; j < 7; j++) if (dp[j] >= 1) dp[j] -= 1;
      var dm = mipFor(rf * 4, sr), ds0 = Math.floor(D.dshape), dsf = D.dshape - ds0, da = TB[ds0][dm], db = TB[Math.min(3, ds0 + 1)][dm];
      var r0 = rdTab(da, db, dsf, dp[0]), r1 = rdTab(da, db, dsf, dp[1]), f0 = rdTab(da, db, dsf, dp[2]), f1 = rdTab(da, db, dsf, dp[3]);
      var u0 = Math.sin(6.283185 * dp[4]), u1 = Math.sin(6.283185 * dp[5]), sb = Math.sin(6.283185 * dp[6]);
      var fv = D.fifth * 0.7, up = D.upper * 0.35 * (0.5 + 0.5 * Math.sin(this.dlfo * 6.283 * 0.019 + 1));
      var dl = (r0 + f0 * fv + u0 * up) * D.drone * amod + sb * D.sub * 0.25;
      var dr = (r1 + f1 * fv + u1 * up) * D.drone * amod + sb * D.sub * 0.25;

      // ---- filter on cloud + drone
      var xl = cl + dl, xr = cr + dr;
      this.fl.run(xl, D.fg, D.fk); this.fr.run(xr, D.fg, D.fk);
      var fl = this.fl.lp * D.fLP + this.fl.bp * D.fBP * 1.4 + this.fl.hp * D.fHP;
      var frr = this.fr.lp * D.fLP + this.fr.bp * D.fBP * 1.4 + this.fr.hp * D.fHP;
      // ---- wow / flutter (tape)
      this.wph += (0.27 + 0.1 * this.lane[2]) / sr; this.fph += 5.3 / sr; if (this.wph > 1) this.wph -= 1; if (this.fph > 1) this.fph -= 1;
      this.wowL.write(fl); this.wowR.write(frr);
      var wd = (0.012 + D.wow * (0.004 * Math.sin(6.283 * this.wph) + 0.0004 * Math.sin(6.283 * this.fph))) * sr;
      var sl = D.wow > 0.001 ? this.wowL.read(wd) : fl, srr = D.wow > 0.001 ? this.wowR.read(wd + D.wow * 0.0007 * sr) : frr;

      // ---- texture
      var n1 = noise(), n2 = noise();
      var wg = Math.tan(Math.PI * clamp(D.wTone * Math.pow(2, this.W[6].v * D.wMove * 1.4), 40, 12000) / sr), wk = 1.6 - D.wMove * 1.2;
      var gust = 0.55 + 0.45 * this.W[7].v * (0.3 + 0.7 * D.wMove);
      this.wl.run(n1, wg, wk); this.wr.run(n2, wg * 1.07, wk);
      var tl = this.wl.bp * D.wind * gust, tr = this.wr.bp * D.wind * gust;
      // crackle (dust on old tape)
      if (Math.random() < D.crackle * 22 / sr) { this.crkEnv = 0.15 + Math.random() * 0.5; this.crkPan = Math.random(); }
      this.crkEnv *= 0.993;
      var cn = noise() * this.crkEnv; this.crk[0] += (cn - this.crk[0]) * 0.5; var ch = cn - this.crk[0];
      tl += ch * (1 - this.crkPan) * 0.25; tr += ch * this.crkPan * 0.25;
      // metal: low noise excites dispersive combs tuned to G (and to the fifth)
      var ex = noise(); this.exLp += (ex - this.exLp) * 0.06; this.exLp2 += (this.exLp - this.exLp2) * 0.06;
      var exs = this.exLp2 * D.metal * (0.3 + 0.7 * Math.max(0, this.W[8].v)) * 0.6;
      var ap = this.apm, c1 = this.combL.read(D.mDelL - 2), c2 = this.combR.read(D.mDelR - 2);
      var y1 = ap[0].readi(173) - 0.5 * c1; ap[0].write(c1 + 0.5 * y1); var z1 = ap[1].readi(97) - 0.5 * y1; ap[1].write(y1 + 0.5 * z1);
      var y2 = ap[2].readi(149) - 0.5 * c2; ap[2].write(c2 + 0.5 * y2); var z2 = ap[3].readi(83) - 0.5 * y2; ap[3].write(y2 + 0.5 * z2);
      this.cdl += (z1 - this.cdl) * 0.45; this.cdr += (z2 - this.cdr) * 0.45;
      this.combL.write(exs + this.cdl * D.mFb); this.combR.write(exs + this.cdr * D.mFb);
      tl += this.cdl * 0.35; tr += this.cdr * 0.35;
      // rumble
      var rm = this.rum; rm[0] += (noise() - rm[0]) * 0.006; rm[1] += (rm[0] - rm[1]) * 0.006; rm[2] += (noise() - rm[2]) * 0.006; rm[3] += (rm[2] - rm[3]) * 0.006;
      var rg = D.rumble * (0.6 + 0.4 * this.lane[3]);
      tl += rm[1] * rg; tr += rm[3] * rg;
      tl *= D.texture; tr *= D.texture;

      // ---- space: delay + reverb (sends)
      var fade = this.fade;
      var inL = (sl + tl) * fade, inR = (srr + tr) * fade;
      var dT = this.dTs += (D.dTime - this.dTs) * 0.00005;
      var dlo = this.dlL.read(dT + 40 * Math.sin(6.283 * this.wph * 2)), dro = this.dlR.read(dT * 1.333);
      this.dLp[0] += (dlo - this.dLp[0]) * D.dTone; this.dLp[1] += (dro - this.dLp[1]) * D.dTone;
      this.dHp[0] += (this.dLp[0] - this.dHp[0]) * 0.012; this.dHp[1] += (this.dLp[1] - this.dHp[1]) * 0.012;
      var eL = this.dLp[0] - this.dHp[0], eR = this.dLp[1] - this.dHp[1];
      this.dlL.write(inL * D.delay + (eL * 0.3 + eR * 0.7) * D.dFb); this.dlR.write(inR * D.delay + (eR * 0.3 + eL * 0.7) * D.dFb);
      // diffusion
      var frz = this.frz, vin = 1 - frz;
      var a = (inL + eL * 0.5) * vin, b = (inR + eR * 0.5) * vin, dd = this.dif, dln = this.difLen;
      var q0 = dd[0].readi(dln[0]); var o0 = q0 - 0.6 * a; dd[0].write(a + 0.6 * o0);
      var q1 = dd[1].readi(dln[1]); var o1 = q1 - 0.6 * o0; dd[1].write(o0 + 0.6 * o1);
      var q2 = dd[2].readi(dln[2]); var o2 = q2 - 0.6 * b; dd[2].write(b + 0.6 * o2);
      var q3 = dd[3].readi(dln[3]); var o3 = q3 - 0.6 * o2; dd[3].write(o2 + 0.6 * o3);
      // FDN 8, Hadamard
      this.vmod += 0.37 / sr; if (this.vmod > 1) this.vmod -= 1;
      var m8 = Math.sin(6.283 * this.vmod) * 9, y = this._y || (this._y = new Float32Array(8)), lp = this.fdnLp, fd = this.fdn, g = this.fdnG, dmp = D.damp + (1 - D.damp) * frz;
      for (j = 0; j < 8; j++) {
        var rdv = j === 0 ? fd[j].read(this.fdnLen[j] + m8) : j === 5 ? fd[j].read(this.fdnLen[j] - m8) : fd[j].readi(this.fdnLen[j] | 0);
        lp[j] += (rdv - lp[j]) * dmp; y[j] = lp[j] * (g[j] + (1 - g[j]) * frz * 0.999);
      }
      var s1, s2, s3, s4, s5, s6, s7, s8;
      s1 = y[0] + y[1]; s2 = y[0] - y[1]; s3 = y[2] + y[3]; s4 = y[2] - y[3]; s5 = y[4] + y[5]; s6 = y[4] - y[5]; s7 = y[6] + y[7]; s8 = y[6] - y[7];
      var h1 = s1 + s3, h2 = s2 + s4, h3 = s1 - s3, h4 = s2 - s4, h5 = s5 + s7, h6 = s6 + s8, h7 = s5 - s7, h8 = s6 - s8;
      var hs = 0.35355339;
      fd[0].write((h1 + h5) * hs + o1); fd[1].write((h2 + h6) * hs + o3); fd[2].write((h3 + h7) * hs + o1 * 0.7); fd[3].write((h4 + h8) * hs + o3 * 0.7);
      fd[4].write((h1 - h5) * hs - o1 * 0.5); fd[5].write((h2 - h6) * hs + o3 * 0.5); fd[6].write((h3 - h7) * hs); fd[7].write((h4 - h8) * hs);
      var vl = (y[0] + y[2] + y[4] + y[6]) * 0.3, vr = (y[1] + y[3] + y[5] + y[7]) * 0.3;
      var vm = D.verb;
      var oL = inL * (1 - vm * 0.65) + vl * vm * 1.2 + eL * 0.8, oR = inR * (1 - vm * 0.65) + vr * vm * 1.2 + eR * 0.8;

      // ---- master: drive, tilt, low cut, width, limiter
      oL = th(oL * D.drive) / D.drive; oR = th(oR * D.drive) / D.drive;   // unity for quiet signals
      this.tiltL += (oL - this.tiltL) * D.tk; this.tiltR += (oR - this.tiltR) * D.tk;
      var tg = D.tilt, lo = 1 + tg * 0.5, hi = 1 - tg * 0.5;
      oL = this.tiltL * lo + (oL - this.tiltL) * hi; oR = this.tiltR * lo + (oR - this.tiltR) * hi;
      var h = this.hp1, zL = D.hb0 * oL + h[0]; h[0] = D.hb1 * oL - D.ha1 * zL + h[1]; h[1] = D.hb2 * oL - D.ha2 * zL;
      var h2b = this.hp2, zR = D.hb0 * oR + h2b[0]; h2b[0] = D.hb1 * oR - D.ha1 * zR + h2b[1]; h2b[1] = D.hb2 * oR - D.ha2 * zR;
      var mid = (zL + zR) * 0.5, side = (zL - zR) * 0.5 * D.width;
      oL = mid + side; oR = mid - side;
      // constant loudness: a slow (about 3 s) level follower pulls everything to the same loudness
      // measured like a loudness meter: 2nd-order high-pass at 100 Hz (the sub does not count), both channels
      var kh = this.kw, xl2 = oL, xr2 = oR;
      var yl = KB0 * xl2 + kh[0]; kh[0] = KB1 * xl2 - KA1 * yl + kh[1]; kh[1] = KB0 * xl2 - KA2 * yl;
      var yr = KB0 * xr2 + kh[2]; kh[2] = KB1 * xr2 - KA1 * yr + kh[3]; kh[3] = KB0 * xr2 - KA2 * yr;
      this.agMs += ((yl * yl + yr * yr) * 0.5 - this.agMs) * 0.000014;   // ~1.5 s
      oL *= this.agc * this.fade2; oR *= this.agc * this.fade2;
      // safety: 3 ms look-ahead limiter at -3 dBFS, then a hard ceiling
      var la = this.la, lp2 = this.lap; la[lp2 * 2] = oL; la[lp2 * 2 + 1] = oR; this.lap = (lp2 + 1) % LA;
      var pk = Math.max(Math.abs(oL), Math.abs(oR));
      this.lenv = pk > this.lenv ? pk : this.lenv * 0.99996;
      var want = this.lenv > CEIL ? CEIL / this.lenv : 1;
      this.lim += (want - this.lim) * (want < this.lim ? 0.03 : 0.0004);
      var rp = this.lap; oL = la[rp * 2] * this.lim; oR = la[rp * 2 + 1] * this.lim;
      oL = oL > CEIL ? CEIL : oL < -CEIL ? -CEIL : oL; oR = oR > CEIL ? CEIL : oR < -CEIL ? -CEIL : oR;
      L[s] = oL; R[s] = oR;
      var ap2 = Math.max(Math.abs(oL), Math.abs(oR)); if (ap2 > this.peak) this.peak = ap2;
      this.rms += (oL * oL - this.rms) * 0.00005;
    }
    // ---- screen data, 20 times a second
    this.postIn -= n;
    if (this.postIn <= 0) {
      this.postIn = Math.floor(sr / 20);
      var vs = [];
      for (i = 0; i < 8; i++) { var v = V[i]; if (v.on) vs.push([i, v.midi, v.env, v.x, v.stage]); }
      this.port.postMessage({ v: vs, P: Array.prototype.slice.call(this.P), lane: Array.prototype.slice.call(this.lane), wx: this.wx, wy: this.wy,
        peak: this.peak, rms: this.rms, fade: this.fade, frz: this.frz, cut: D.cut, ev: this.events, next: this.nextEv / sr });
      this.peak = 0;
    }
    return true;
  }
}
registerProcessor('stasis', Stasis);
if (typeof module !== 'undefined') module.exports = { PARAMS: PARAMS };
