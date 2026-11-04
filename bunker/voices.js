// BUNKER voices (preview) — generated. MIT license.
(function () {

// ---- gen~ runtime (as test/runtime.js), worklet-safe
var Data = function (n, c) { this.n = n; this.a = new Float64Array(n * c); };
function peek(d, i, c) { return d.a[c * d.n + (i | 0)]; }
function poke(d, v, i, c) { d.a[c * d.n + (i | 0)] = v; }
var Delay = function (n) { this.b = new Float64Array(n); this.w = 0; this.n = n; this.last = 0; };
Delay.prototype.write = function (x) { this.b[this.w] = x; this.last = this.w; this.w = (this.w + 1) % this.n; };
Delay.prototype.read = function (t) {
  t = t < 0 ? 0 : (t > this.n - 1 ? this.n - 1 : t);
  var p = this.last - t, i = Math.floor(p), f = p - i, n = this.n;
  var a = this.b[((i % n) + n) % n], b = this.b[(((i + 1) % n) + n) % n];
  return a + (b - a) * f;
};
var nseed = 12345;
function noise() { nseed = (nseed * 1664525 + 1013904223) >>> 0; return nseed / 2147483648 - 1; }
function clamp(x, a, b) { return x < a ? a : (x > b ? b : x); }
function exp2(x) { return Math.pow(2, x); }
function fract(x) { return x - Math.floor(x); }
function dbtoa(x) { return Math.pow(10, x / 20); }
var sin = Math.sin, cos = Math.cos, tan = Math.tan, abs = Math.abs, floor = Math.floor, min = Math.min,
    max = Math.max, pow = Math.pow, sqrt = Math.sqrt, exp = Math.exp, tanh = Math.tanh, log2 = Math.log2;
function makeBunkerVoices(SR) {
  "use strict";
  var M = Math;
  var v0p0 = 0.3;
  var v0p1 = 0.5;
  var v0p2 = 0.45;
  var v0p3 = 0.45;
  var v0p4 = 0.35;
  var v0p5 = 0.3;
  var v0p6 = 0.15;
  var v0p7 = 0.2;
  var v0lvl = 0.9;
  var v0pan = 0.5;
  var v0echo = 0.0;
  var v0void = 0.0;
  var v0cut = 0.5;
  var v0trig = 0.0;
  var v1p0 = 0.5;
  var v1p1 = 0.45;
  var v1p2 = 0.35;
  var v1p3 = 0.6;
  var v1p4 = 0.4;
  var v1p5 = 0.55;
  var v1p6 = 0.25;
  var v1p7 = 0.35;
  var v1lvl = 0.85;
  var v1pan = 0.5;
  var v1echo = 0.0;
  var v1void = 0.08;
  var v1cut = 0.5;
  var v1trig = 0.0;
  var v2p0 = 0.4;
  var v2p1 = 0.45;
  var v2p2 = 0.4;
  var v2p3 = 0.45;
  var v2p4 = 0.6;
  var v2p5 = 0.3;
  var v2p6 = 0.5;
  var v2p7 = 0.35;
  var v2lvl = 0.85;
  var v2pan = 0.55;
  var v2echo = 0.15;
  var v2void = 0.2;
  var v2cut = 0.5;
  var v2trig = 0.0;
  var v3p0 = 0.3;
  var v3p1 = 0.5;
  var v3p2 = 0.25;
  var v3p3 = 0.55;
  var v3p4 = 0.7;
  var v3p5 = 0.3;
  var v3p6 = 0.2;
  var v3p7 = 0.0;
  var v3lvl = 0.85;
  var v3pan = 0.42;
  var v3echo = 0.0;
  var v3void = 0.05;
  var v3cut = 0.5;
  var v3trig = 0.0;
  var v4p0 = 0.3;
  var v4p1 = 0.5;
  var v4p2 = 0.32;
  var v4p3 = 0.5;
  var v4p4 = 0.7;
  var v4p5 = 0.3;
  var v4p6 = 0.1;
  var v4p7 = 0.0;
  var v4lvl = 0.85;
  var v4pan = 0.58;
  var v4echo = 0.05;
  var v4void = 0.1;
  var v4cut = 0.5;
  var v4trig = 0.0;
  var v5p0 = 0.3;
  var v5p1 = 0.35;
  var v5p2 = 0.22;
  var v5p3 = 0.35;
  var v5p4 = 0.4;
  var v5p5 = 0.25;
  var v5p6 = 0.3;
  var v5p7 = 0.1;
  var v5lvl = 0.85;
  var v5pan = 0.38;
  var v5echo = 0.1;
  var v5void = 0.12;
  var v5cut = 0.5;
  var v5trig = 0.0;
  var v6p0 = 0.2;
  var v6p1 = 0.5;
  var v6p2 = 0.35;
  var v6p3 = 0.5;
  var v6p4 = 0.4;
  var v6p5 = 0.5;
  var v6p6 = 0.15;
  var v6p7 = 0.1;
  var v6lvl = 0.85;
  var v6pan = 0.62;
  var v6echo = 0.35;
  var v6void = 0.15;
  var v6cut = 0.5;
  var v6trig = 0.0;
  var v7p0 = 0.0;
  var v7p1 = 0.45;
  var v7p2 = 0.4;
  var v7p3 = 0.15;
  var v7p4 = 0.45;
  var v7p5 = 0.55;
  var v7p6 = 0.0;
  var v7p7 = 0.05;
  var v7lvl = 0.85;
  var v7pan = 0.3;
  var v7echo = 0.25;
  var v7void = 0.2;
  var v7cut = 0.5;
  var v7trig = 0.0;
  var v8p0 = 1.0;
  var v8p1 = 0.65;
  var v8p2 = 0.3;
  var v8p3 = 0.5;
  var v8p4 = 0.3;
  var v8p5 = 0.75;
  var v8p6 = 0.5;
  var v8p7 = 0.0;
  var v8lvl = 0.85;
  var v8pan = 0.7;
  var v8echo = 0.3;
  var v8void = 0.25;
  var v8cut = 0.5;
  var v8trig = 0.0;
  var mvol = 1.0;
  var rumamt = 0.0;
  var rumdec = 0.5;
  var rumtone = 0.4;
  var rumdrive = 0.4;
  var rumduck = 0.7;
  var rumgroove = 0.0;
  var rummotion = 0.3;
  var rumwidth = 0.4;
  var ptempo = 126.0;
  var tl0 = 0.0;
  var av0 = 0.0;
  var tt0 = 0.0;
  var vel0 = 0.0;
  var lpc0 = 0.0;
  var hpc0 = 0.0;
  var hpx0 = 0.0;
  var gn0 = 0.0;
  var gl0 = 0.0;
  var gr0 = 0.0;
  var tl1 = 0.0;
  var av1 = 0.0;
  var tt1 = 0.0;
  var vel1 = 0.0;
  var lpc1 = 0.0;
  var hpc1 = 0.0;
  var hpx1 = 0.0;
  var gn1 = 0.0;
  var gl1 = 0.0;
  var gr1 = 0.0;
  var tl2 = 0.0;
  var av2 = 0.0;
  var tt2 = 0.0;
  var vel2 = 0.0;
  var lpc2 = 0.0;
  var hpc2 = 0.0;
  var hpx2 = 0.0;
  var gn2 = 0.0;
  var gl2 = 0.0;
  var gr2 = 0.0;
  var tl3 = 0.0;
  var av3 = 0.0;
  var tt3 = 0.0;
  var vel3 = 0.0;
  var lpc3 = 0.0;
  var hpc3 = 0.0;
  var hpx3 = 0.0;
  var gn3 = 0.0;
  var gl3 = 0.0;
  var gr3 = 0.0;
  var tl4 = 0.0;
  var av4 = 0.0;
  var tt4 = 0.0;
  var vel4 = 0.0;
  var lpc4 = 0.0;
  var hpc4 = 0.0;
  var hpx4 = 0.0;
  var gn4 = 0.0;
  var gl4 = 0.0;
  var gr4 = 0.0;
  var tl5 = 0.0;
  var av5 = 0.0;
  var tt5 = 0.0;
  var vel5 = 0.0;
  var lpc5 = 0.0;
  var hpc5 = 0.0;
  var hpx5 = 0.0;
  var gn5 = 0.0;
  var gl5 = 0.0;
  var gr5 = 0.0;
  var tl6 = 0.0;
  var av6 = 0.0;
  var tt6 = 0.0;
  var vel6 = 0.0;
  var lpc6 = 0.0;
  var hpc6 = 0.0;
  var hpx6 = 0.0;
  var gn6 = 0.0;
  var gl6 = 0.0;
  var gr6 = 0.0;
  var tl7 = 0.0;
  var av7 = 0.0;
  var tt7 = 0.0;
  var vel7 = 0.0;
  var lpc7 = 0.0;
  var hpc7 = 0.0;
  var hpx7 = 0.0;
  var gn7 = 0.0;
  var gl7 = 0.0;
  var gr7 = 0.0;
  var tl8 = 0.0;
  var av8 = 0.0;
  var tt8 = 0.0;
  var vel8 = 0.0;
  var lpc8 = 0.0;
  var hpc8 = 0.0;
  var hpx8 = 0.0;
  var gn8 = 0.0;
  var gl8 = 0.0;
  var gr8 = 0.0;
  var ph0 = 0.0;
  var ph1 = 0.0;
  var ph1b = 0.0;
  var nl1 = 0.0;
  var nh1 = 0.0;
  var c1 = 0.0;
  var c2 = 0.0;
  var ph5 = 0.0;
  var ph5b = 0.0;
  var ph6 = 0.0;
  var ph6b = 0.0;
  var r1 = 0.0;
  var r2 = 0.0;
  var hp0 = 0.0;
  var hp1 = 0.0;
  var hp2 = 0.0;
  var hp3 = 0.0;
  var hp4 = 0.0;
  var hp5 = 0.0;
  var a1_3 = 0.0;
  var a2_3 = 0.0;
  var a1_4 = 0.0;
  var a2_4 = 0.0;
  var gh3 = 0.0;
  var gx3 = 0.0;
  var gh4 = 0.0;
  var gx4 = 0.0;
  var lph7 = 0.0;
  var sph7 = 0.0;
  var lph8 = 0.0;
  var sph8 = 0.0;
  var kenv = 0.0;
  var rmph = 0.0;
  var rq0 = 0.0;
  var rq1 = 0.0;
  var rq2 = 0.0;
  var rq3 = 0.0;
  var ro1l = 0.0;
  var ro2l = 0.0;
  var ro1r = 0.0;
  var ro2r = 0.0;
  var rhl = 0.0;
  var rhr = 0.0;
  var rhxl = 0.0;
  var rhxr = 0.0;
  var rdk = 1.0;
  var cg3 = 1.0;
  var cg4 = 1.0;
  var rpd = new Delay(200000);
  var rd0 = new Delay(24000);
  var rd1 = new Delay(24000);
  var rd2 = new Delay(24000);
  var rd3 = new Delay(24000);
  var sr, r48, out1 = 0, out2 = 0, out3 = 0, out4 = 0, out5 = 0, out6 = 0;
  function tick(in1, in2, in3, in4) { var ae, ax, bdec, be, body, bp, cl, cu, dec, dg, dk, dl, dr, e, el, env, er, f, f0, fa, fb, fc, g, gap, gl, gsel, hb0, hb1, hb2, hb3, hb4, hb5, hf, hh, hp, ht, lf, lp, lr, m, met, mx, n, nb, nh, nlf, pa, pe, rbeat, rdg, rf, rg, rg2, rgain, rhg, ri, rl, rmd, rmono, rpre, rq, rr, rs, rt, s, sb, sdec, se, sh, sn, sq, sqr, src, swp, swt, t, tb, tri, wl, wr, x, xl, xr, y, z0, z1, z2, z3;
























































































































































































































































































sr = SR;
dl = 0;
dr = 0;
el = 0;
er = 0;
wl = 0;
wr = 0;

hb0 = 0;
hb1 = 0;
hb2 = 0;
hb3 = 0;
hb4 = 0;
hb5 = 0;
if (av3 + av4 > 0) {
	ht = exp2((v3p1 - 0.5) * 0.8);
	hp0 = fract(hp0 + 205.3 * ht / sr);
	hp1 = fract(hp1 + 304.4 * ht / sr);
	hp2 = fract(hp2 + 369.6 * ht / sr);
	hp3 = fract(hp3 + 522.7 * ht / sr);
	hp4 = fract(hp4 + 540 * ht / sr);
	hp5 = fract(hp5 + 800 * ht / sr);
	hb0 = hp0 < 0.5 ? 1 : -1;
	hb1 = hp1 < 0.5 ? 1 : -1;
	hb2 = hp2 < 0.5 ? 1 : -1;
	hb3 = hp3 < 0.5 ? 1 : -1;
	hb4 = hp4 < 0.5 ? 1 : -1;
	hb5 = hp5 < 0.5 ? 1 : -1;
}



if (v0trig != tl0) {
	tl0 = v0trig;
	vel0 = (v0trig - 256 * floor(v0trig / 256)) / 127;
	av0 = 1;
	tt0 = 0;
	ph0 = 0;
}

x = 0;
if (av0 > 0) {
	tt0 = tt0 + 1;
	t = tt0 / sr;

	
	m = v0p0;
	f0 = 36.708 * exp2(v0p1 * 0.83333);
	dec = 0.03 * exp2(v0p2 * 5.3);
	swt = 0.004 * exp2(v0p4 * 5.2);
	pe = exp(-t / swt);
	f = f0 * exp2(v0p3 * 4 * pe);
	ph0 = fract(ph0 + f / sr);
	s = sin(6.283185307179586 * ph0);
	sh = 1 + 2.5 * m;
	s = tanh(s * sh) / tanh(sh);
	ae = exp(-t / dec) * (1 + (0.35 * m + 0.8 * v0p7) * exp(-t / 0.014));
	cl = v0p5 * ((1 - m) * sin(6.283185307179586 * 1650 * t) * exp(-t / 0.0012) * 0.7 + m * noise() * exp(-t / 0.0022) * 0.6);
	x = s * ae + cl;
	dg = 1 + 7 * v0p6;
	x = tanh(x * dg) / tanh(dg);
	if (t > dec * 9 + 0.05) {
		av0 = 0;
	}
	x = x * (0.3 + 0.7 * vel0) * v0lvl * 0.6912;
	
	ax = abs(x);
	if (ax > 0.6) {
		x = (0.6 + 0.4 * tanh((ax - 0.6) / 0.4)) * (x < 0 ? -1 : 1);
	}
	cu = v0cut;
	if (cu < 0.48) {
		lf = 18000 * exp2(-(0.48 - cu) * 14);
		lpc0 = lpc0 + (x - lpc0) * (1 - exp(-6.283185307179586 * lf / sr));
		x = lpc0;
	} else if (cu > 0.52) {
		hf = 20 * exp2((cu - 0.52) * 16);
		hpc0 = exp(-6.283185307179586 * hf / sr) * (hpc0 + x - hpx0);
		hpx0 = x;
		x = hpc0;
	}
}
gn0 = x;
pa = v0pan;
xl = x * sqrt(1 - pa);
xr = x * sqrt(pa);
dl = dl + xl;
dr = dr + xr;
el = el + xl * v0echo;
er = er + xr * v0echo;
wl = wl + xl * v0void;
wr = wr + xr * v0void;




kenv = max(abs(gn0), kenv * exp(-1 / (0.14 * sr)));
if (rumamt > 0.001) {
	rbeat = 60 / max(ptempo, 20) * sr;
	gsel = floor(rumgroove * 3.99);
	rpre = gsel == 0 ? 1 : rbeat * 0.25 * gsel;
	rpd.write(gn0);
	ri = rpd.read(max(1, min(rpre, 190000)));
	rmph = fract(rmph + 0.21 / sr);
	rmd = rummotion * 0.003 * sr;
	z0 = rd0.read(0.0371 * sr + rmd * (1 + sin(6.283185307179586 * rmph)));
	z1 = rd1.read(0.0533 * sr + rmd * (1 + sin(6.283185307179586 * fract(rmph + 0.25))));
	z2 = rd2.read(0.0713 * sr + rmd * (1 + sin(6.283185307179586 * fract(rmph + 0.5))));
	z3 = rd3.read(0.0897 * sr + rmd * (1 + sin(6.283185307179586 * fract(rmph + 0.75))));
	rt = 0.3 * exp2(rumdec * 4.3);
	rf = 45 * exp2(rumtone * 3.4);
	rg = 1 - exp(-6.283185307179586 * rf * 1.6 / sr);
	rq0 = rq0 + rg * (z0 - rq0);
	rq1 = rq1 + rg * (z1 - rq1);
	rq2 = rq2 + rg * (z2 - rq2);
	rq3 = rq3 + rg * (z3 - rq3);
	rs = (rq0 + rq1 + rq2 + rq3) * 0.5;
	rd0.write(ri * 0.5 + (rq0 - rs) * exp(-6.9078 * 0.0371 / rt));
	rd1.write(ri * 0.5 + (rq1 - rs) * exp(-6.9078 * 0.0533 / rt));
	rd2.write(ri * 0.5 + (rq2 - rs) * exp(-6.9078 * 0.0713 / rt));
	rd3.write(ri * 0.5 + (rq3 - rs) * exp(-6.9078 * 0.0897 / rt));
	rl = rq0 + rq2 * 0.7;
	rr = rq1 + rq3 * 0.7;
	rdg = 1 + 9 * rumdrive;
	rl = tanh(rl * rdg) / (1 + 1.5 * rumdrive);
	rr = tanh(rr * rdg) / (1 + 1.5 * rumdrive);
	
	rg2 = 1 - exp(-6.283185307179586 * rf * 2.2 / sr);
	ro1l = ro1l + rg2 * (rl - ro1l);
	ro2l = ro2l + rg2 * (ro1l - ro2l);
	ro1r = ro1r + rg2 * (rr - ro1r);
	ro2r = ro2r + rg2 * (ro1r - ro2r);
	rhg = exp(-6.283185307179586 * 28 / sr);
	rhl = rhg * (rhl + ro2l - rhxl);
	rhxl = ro2l;
	rhr = rhg * (rhr + ro2r - rhxr);
	rhxr = ro2r;
	
	dk = 1 - rumduck * min(1, kenv * 4);
	rdk = rdk + (dk - rdk) * (1 - exp(-1 / (0.004 * sr)));
	rmono = 0.5 * (rhl + rhr);
	rgain = rumamt * rumamt * 5.5 * rdk;
	
	dl = dl + 0.45 * tanh((rmono + (rhl - rmono) * rumwidth) * rgain / 0.45);
	dr = dr + 0.45 * tanh((rmono + (rhr - rmono) * rumwidth) * rgain / 0.45);
}



if (v1trig != tl1) {
	tl1 = v1trig;
	vel1 = (v1trig - 256 * floor(v1trig / 256)) / 127;
	av1 = 1;
	tt1 = 0;
}

x = 0;
if (av1 > 0) {
	tt1 = tt1 + 1;
	t = tt1 / sr;

	
	m = v1p0;
	f0 = 146.83 * exp2(v1p1 * 1.16667);
	f = f0 * exp2(v1p6 * 2 * exp(-t / 0.018));
	ph1 = fract(ph1 + f / sr);
	ph1b = fract(ph1b + f * 1.47 / sr);
	tri = 4 * abs(ph1 - 0.5) - 1;
	sb = sin(6.283185307179586 * ph1);
	body = sb + (tri - sb) * m * 0.5 + v1p7 * sin(6.283185307179586 * ph1b) * (1 - 0.5 * m);
	bdec = 0.03 * exp2(v1p2 * 4.3);
	be = exp(-t / bdec);
	n = noise();
	nlf = 900 * exp2(v1p5 * 3.7);
	gl = 1 - exp(-6.283185307179586 * min(nlf * 2.2, 0.45 * sr) / sr);
	nl1 = nl1 + gl * (n - nl1);
	nh = nl1 - nh1;
	nh1 = nh1 + (nl1 - nh1) * (1 - exp(-6.283185307179586 * (nlf * (0.35 + 0.4 * m)) / sr));
	sdec = 0.03 * exp2(v1p4 * 4.3);
	se = exp(-t / sdec);
	x = body * be * (0.9 - 0.35 * v1p3) + nh * se * v1p3 * (1.4 + 0.6 * m);
	x = tanh(x * 1.3);
	if (t > max(bdec, sdec) * 9 + 0.03) {
		av1 = 0;
	}
	x = x * (0.3 + 0.7 * vel1) * v1lvl * 0.5941;
	
	ax = abs(x);
	if (ax > 0.6) {
		x = (0.6 + 0.4 * tanh((ax - 0.6) / 0.4)) * (x < 0 ? -1 : 1);
	}
	cu = v1cut;
	if (cu < 0.48) {
		lf = 18000 * exp2(-(0.48 - cu) * 14);
		lpc1 = lpc1 + (x - lpc1) * (1 - exp(-6.283185307179586 * lf / sr));
		x = lpc1;
	} else if (cu > 0.52) {
		hf = 20 * exp2((cu - 0.52) * 16);
		hpc1 = exp(-6.283185307179586 * hf / sr) * (hpc1 + x - hpx1);
		hpx1 = x;
		x = hpc1;
	}
}
gn1 = x;
pa = v1pan;
xl = x * sqrt(1 - pa);
xr = x * sqrt(pa);
dl = dl + xl;
dr = dr + xr;
el = el + xl * v1echo;
er = er + xr * v1echo;
wl = wl + xl * v1void;
wr = wr + xr * v1void;


if (v2trig != tl2) {
	tl2 = v2trig;
	vel2 = (v2trig - 256 * floor(v2trig / 256)) / 127;
	av2 = 1;
	tt2 = 0;
}

x = 0;
if (av2 > 0) {
	tt2 = tt2 + 1;
	t = tt2 / sr;

	
	m = v2p0;
	gap = 0.004 + 0.016 * v2p3;
	nb = 1 + floor(v2p4 * 4.99);
	tb = t - gap * (nb - 1);
	env = 0;
	if (tb < 0) {
		env = exp(-(t - gap * floor(t / gap)) / (0.0018 + 0.003 * v2p6));
	} else {
		env = exp(-tb / (0.02 + 0.5 * v2p2 * v2p2)) * (0.75 + 0.25 * v2p6);
	}
	env = env + v2p5 * 0.35 * exp(-t / (0.08 + 0.6 * v2p2)) * (t > 0.01 ? 1 : 0);
	fc = (700 + 400 * m) * exp2(v2p1 * 2.3);
	g = tan(3.141592653589793 * min(fc, 0.42 * sr) / sr);
	rq = 1 / (0.8 + v2p7 * 6 * (1 - 0.4 * m));
	hh = 1 / (1 + rq * g + g * g);
	n = noise();
	hp = (n - rq * c1 - g * c1 - c2) * hh;
	bp = g * hp + c1;
	c1 = g * hp + bp;
	lp = g * bp + c2;
	c2 = g * bp + lp;
	x = tanh((bp * rq * 2.2 + hp * 0.25 * m) * env * (1 - 0.4 * v2p7));
	if (tb > (0.02 + 0.5 * v2p2 * v2p2) * 9 + (0.08 + 0.6 * v2p2) * 6 * v2p5) {
		av2 = 0;
	}
	x = x * (0.3 + 0.7 * vel2) * v2lvl * 1.1444;
	
	ax = abs(x);
	if (ax > 0.6) {
		x = (0.6 + 0.4 * tanh((ax - 0.6) / 0.4)) * (x < 0 ? -1 : 1);
	}
	cu = v2cut;
	if (cu < 0.48) {
		lf = 18000 * exp2(-(0.48 - cu) * 14);
		lpc2 = lpc2 + (x - lpc2) * (1 - exp(-6.283185307179586 * lf / sr));
		x = lpc2;
	} else if (cu > 0.52) {
		hf = 20 * exp2((cu - 0.52) * 16);
		hpc2 = exp(-6.283185307179586 * hf / sr) * (hpc2 + x - hpx2);
		hpx2 = x;
		x = hpc2;
	}
}
gn2 = x;
pa = v2pan;
xl = x * sqrt(1 - pa);
xr = x * sqrt(pa);
dl = dl + xl;
dr = dr + xr;
el = el + xl * v2echo;
er = er + xr * v2echo;
wl = wl + xl * v2void;
wr = wr + xr * v2void;


if (v3trig != tl3) {
	tl3 = v3trig;
	vel3 = (v3trig - 256 * floor(v3trig / 256)) / 127;
	av3 = 1;
	tt3 = 0;
	cg4 = 0.9999;
	cg3 = 1;
}

x = 0;
if (av3 > 0) {
	tt3 = tt3 + 1;
	t = tt3 / sr;

	
	m = v3p0;
	met = v3p4 * (1 - 0.6 * m);
	mx = hb0 + hb1 + hb2 + hb3 + hb4 + hb5;
	src = mx * 0.17 * met + noise() * (1 - met) * (0.6 + 0.4 * m);
	fc = 6500 * exp2(v3p3 * 1.2 + (v3p1 - 0.5) * 0.8);
	g = tan(3.141592653589793 * min(fc, 0.42 * sr) / sr);
	rq = 1 / (0.7 + v3p5 * 5);
	hh = 1 / (1 + rq * g + g * g);
	hp = (src - rq * a1_3 - g * a1_3 - a2_3) * hh;
	bp = g * hp + a1_3;
	a1_3 = g * hp + bp;
	lp = g * bp + a2_3;
	a2_3 = g * bp + lp;
	y = hp * (0.6 + 0.3 * m) + bp * rq * 1.2;
	dec = 0.012 * exp2(v3p2 * 5);
	e = exp(-t / dec) * cg3 + v3p6 * exp(-t / 0.0015) * 0.8;
	x = y * e;
	if (v3p7 > 0.01) {
		gh3 = gh3 + (1 - v3p7 * 0.85) * 0.5;
		if (gh3 >= 0.5) {
			gh3 = gh3 - 0.5;
			gx3 = floor(x * (40 - 36 * v3p7)) / (40 - 36 * v3p7);
		}
		x = gx3;
	}
	if (t > dec * 9 + 0.02 || cg3 < 0.001) {
		av3 = 0;
	}
	x = x * (0.3 + 0.7 * vel3) * v3lvl * 0.6544;
	
	ax = abs(x);
	if (ax > 0.6) {
		x = (0.6 + 0.4 * tanh((ax - 0.6) / 0.4)) * (x < 0 ? -1 : 1);
	}
	cu = v3cut;
	if (cu < 0.48) {
		lf = 18000 * exp2(-(0.48 - cu) * 14);
		lpc3 = lpc3 + (x - lpc3) * (1 - exp(-6.283185307179586 * lf / sr));
		x = lpc3;
	} else if (cu > 0.52) {
		hf = 20 * exp2((cu - 0.52) * 16);
		hpc3 = exp(-6.283185307179586 * hf / sr) * (hpc3 + x - hpx3);
		hpx3 = x;
		x = hpc3;
	}
}
gn3 = x;
pa = v3pan;
xl = x * sqrt(1 - pa);
xr = x * sqrt(pa);
dl = dl + xl;
dr = dr + xr;
el = el + xl * v3echo;
er = er + xr * v3echo;
wl = wl + xl * v3void;
wr = wr + xr * v3void;


if (v4trig != tl4) {
	tl4 = v4trig;
	vel4 = (v4trig - 256 * floor(v4trig / 256)) / 127;
	av4 = 1;
	tt4 = 0;
	cg4 = 1;
}

x = 0;
if (av4 > 0) {
	tt4 = tt4 + 1;
	t = tt4 / sr;
	if (cg4 < 1) {
		cg4 = cg4 * exp(-1 / (0.006 * sr));
	}

	
	m = v4p0;
	met = v4p4 * (1 - 0.6 * m);
	mx = hb0 + hb1 + hb2 + hb3 + hb4 + hb5;
	src = mx * 0.17 * met + noise() * (1 - met) * (0.6 + 0.4 * m);
	fc = 6500 * exp2(v4p3 * 1.2 + (v4p1 - 0.5) * 0.8);
	g = tan(3.141592653589793 * min(fc, 0.42 * sr) / sr);
	rq = 1 / (0.7 + v4p5 * 5);
	hh = 1 / (1 + rq * g + g * g);
	hp = (src - rq * a1_4 - g * a1_4 - a2_4) * hh;
	bp = g * hp + a1_4;
	a1_4 = g * hp + bp;
	lp = g * bp + a2_4;
	a2_4 = g * bp + lp;
	y = hp * (0.6 + 0.3 * m) + bp * rq * 1.2;
	dec = 0.08 * exp2(v4p2 * 5);
	e = exp(-t / dec) * cg4 + v4p6 * exp(-t / 0.0015) * 0.8;
	x = y * e;
	if (v4p7 > 0.01) {
		gh4 = gh4 + (1 - v4p7 * 0.85) * 0.5;
		if (gh4 >= 0.5) {
			gh4 = gh4 - 0.5;
			gx4 = floor(x * (40 - 36 * v4p7)) / (40 - 36 * v4p7);
		}
		x = gx4;
	}
	if (t > dec * 9 + 0.02 || cg4 < 0.001) {
		av4 = 0;
	}
	x = x * (0.3 + 0.7 * vel4) * v4lvl * 0.9179;
	
	ax = abs(x);
	if (ax > 0.6) {
		x = (0.6 + 0.4 * tanh((ax - 0.6) / 0.4)) * (x < 0 ? -1 : 1);
	}
	cu = v4cut;
	if (cu < 0.48) {
		lf = 18000 * exp2(-(0.48 - cu) * 14);
		lpc4 = lpc4 + (x - lpc4) * (1 - exp(-6.283185307179586 * lf / sr));
		x = lpc4;
	} else if (cu > 0.52) {
		hf = 20 * exp2((cu - 0.52) * 16);
		hpc4 = exp(-6.283185307179586 * hf / sr) * (hpc4 + x - hpx4);
		hpx4 = x;
		x = hpc4;
	}
}
gn4 = x;
pa = v4pan;
xl = x * sqrt(1 - pa);
xr = x * sqrt(pa);
dl = dl + xl;
dr = dr + xr;
el = el + xl * v4echo;
er = er + xr * v4echo;
wl = wl + xl * v4void;
wr = wr + xr * v4void;


if (v5trig != tl5) {
	tl5 = v5trig;
	vel5 = (v5trig - 256 * floor(v5trig / 256)) / 127;
	av5 = 1;
	tt5 = 0;
}

x = 0;
if (av5 > 0) {
	tt5 = tt5 + 1;
	t = tt5 / sr;

	
	m = v5p0;
	f0 = 61.735 * exp2(v5p1 * 3);
	swt = 0.005 * exp2(v5p4 * 4.6);
	f = f0 * exp2(v5p3 * 2 * exp(-t / swt) + m * 0.3 * exp(-t / 0.004));
	ph5 = fract(ph5 + f / sr);
	ph5b = fract(ph5b + f * (1.5 + 0.08 * m) / sr);
	s = sin(6.283185307179586 * ph5) + v5p6 * 0.5 * sin(6.283185307179586 * ph5b) * exp(-t / 0.05);
	dec = (0.05 + 0.35 * (1 - m)) * exp2(v5p2 * 3);
	x = s * exp(-t / dec) + v5p5 * noise() * exp(-t / (0.004 + 0.01 * (1 - m))) * 0.7;
	dg = 1 + 5 * v5p7;
	x = tanh(x * dg) / tanh(dg);
	if (t > dec * 9 + 0.02) {
		av5 = 0;
	}
	x = x * (0.3 + 0.7 * vel5) * v5lvl * 0.2845;
	
	ax = abs(x);
	if (ax > 0.6) {
		x = (0.6 + 0.4 * tanh((ax - 0.6) / 0.4)) * (x < 0 ? -1 : 1);
	}
	cu = v5cut;
	if (cu < 0.48) {
		lf = 18000 * exp2(-(0.48 - cu) * 14);
		lpc5 = lpc5 + (x - lpc5) * (1 - exp(-6.283185307179586 * lf / sr));
		x = lpc5;
	} else if (cu > 0.52) {
		hf = 20 * exp2((cu - 0.52) * 16);
		hpc5 = exp(-6.283185307179586 * hf / sr) * (hpc5 + x - hpx5);
		hpx5 = x;
		x = hpc5;
	}
}
gn5 = x;
pa = v5pan;
xl = x * sqrt(1 - pa);
xr = x * sqrt(pa);
dl = dl + xl;
dr = dr + xr;
el = el + xl * v5echo;
er = er + xr * v5echo;
wl = wl + xl * v5void;
wr = wr + xr * v5void;


if (v6trig != tl6) {
	tl6 = v6trig;
	vel6 = (v6trig - 256 * floor(v6trig / 256)) / 127;
	av6 = 1;
	tt6 = 0;
}

x = 0;
if (av6 > 0) {
	tt6 = tt6 + 1;
	t = tt6 / sr;

	
	m = v6p0;
	fa = 277.18 * exp2(v6p1 * 2);        

	fb = fa * (3.66 + (1.48 + 0.4 * v6p5 - 3.66) * m);
	ph6 = fract(ph6 + fa / sr);
	ph6b = fract(ph6b + fb / sr);
	sq = (ph6 < 0.5 ? 1 : -1) + (ph6b < 0.5 ? 1 : -1);
	sn = sin(6.283185307179586 * ph6) + sin(6.283185307179586 * ph6b);
	src = sn + (sq * 0.5 - sn) * m + v6p6 * noise();
	fc = fa * (3.7 - 1.0 * m) * exp2((v6p3 - 0.5) * 2);
	g = tan(3.141592653589793 * min(fc, 0.42 * sr) / sr);
	rq = 1 / (1.5 + 2 * m);
	hh = 1 / (1 + rq * g + g * g);
	hp = (src - rq * r1 - g * r1 - r2) * hh;
	bp = g * hp + r1;
	r1 = g * hp + bp;
	lp = g * bp + r2;
	r2 = g * bp + lp;
	dec = (0.012 + 0.09 * m) * exp2(v6p2 * 3.5);
	e = exp(-t / dec) * (1 - m * 0.5) + m * 0.5 * exp(-t / (dec * 0.12));
	x = (bp * rq * 1.5 + hp * 0.2) * e + v6p4 * noise() * exp(-t / 0.0008) * 0.6;
	dg = 1 + 5 * v6p7;
	x = tanh(x * dg) / tanh(dg);
	if (t > dec * 9 + 0.02) {
		av6 = 0;
	}
	x = x * (0.3 + 0.7 * vel6) * v6lvl * 0.244;
	
	ax = abs(x);
	if (ax > 0.6) {
		x = (0.6 + 0.4 * tanh((ax - 0.6) / 0.4)) * (x < 0 ? -1 : 1);
	}
	cu = v6cut;
	if (cu < 0.48) {
		lf = 18000 * exp2(-(0.48 - cu) * 14);
		lpc6 = lpc6 + (x - lpc6) * (1 - exp(-6.283185307179586 * lf / sr));
		x = lpc6;
	} else if (cu > 0.52) {
		hf = 20 * exp2((cu - 0.52) * 16);
		hpc6 = exp(-6.283185307179586 * hf / sr) * (hpc6 + x - hpx6);
		hpx6 = x;
		x = hpc6;
	}
}
gn6 = x;
pa = v6pan;
xl = x * sqrt(1 - pa);
xr = x * sqrt(pa);
dl = dl + xl;
dr = dr + xr;
el = el + xl * v6echo;
er = er + xr * v6echo;
wl = wl + xl * v6void;
wr = wr + xr * v6void;


if (v7trig != tl7) {
	tl7 = v7trig;
	vel7 = (v7trig - 256 * floor(v7trig / 256)) / 127;
	av7 = 1;
	tt7 = 0;
	sph7 = 0;
}

x = 0;
if (av7 > 0) {
	tt7 = tt7 + 1;
	t = tt7 / sr;

	
	swp = (v7p3 - 0.5) * 2;
	swt = 0.01 * exp2(v7p4 * 7);
	lr = 0.5 * exp2(v7p5 * 6.3);
	lph7 = fract(lph7 + lr / sr);
	lf = lph7 < 0.5 ? 1 : -1;
	f = 30.868 * exp2(v7p1 * 6 + swp * 4 * (1 - exp(-t / swt)) + lf * v7p6 * v7p6 * 1.5 + v7p7 * v7p7 * noise() * 0.8);
	sph7 = fract(sph7 + min(f, 0.45 * sr) / sr);
	tri = 4 * abs(sph7 - 0.5) - 1;
	sqr = sph7 < 0.5 ? 0.8 : -0.8;
	s = tri + (sqr - tri) * v7p0;
	dec = 0.03 * exp2(v7p2 * 6.2);
	x = (s + v7p7 * noise() * 0.6) * exp(-t / dec) * (1 - exp(-t / 0.0007));
	if (t > dec * 9 + 0.02) {
		av7 = 0;
	}
	x = x * (0.3 + 0.7 * vel7) * v7lvl * 0.2993;
	
	ax = abs(x);
	if (ax > 0.6) {
		x = (0.6 + 0.4 * tanh((ax - 0.6) / 0.4)) * (x < 0 ? -1 : 1);
	}
	cu = v7cut;
	if (cu < 0.48) {
		lf = 18000 * exp2(-(0.48 - cu) * 14);
		lpc7 = lpc7 + (x - lpc7) * (1 - exp(-6.283185307179586 * lf / sr));
		x = lpc7;
	} else if (cu > 0.52) {
		hf = 20 * exp2((cu - 0.52) * 16);
		hpc7 = exp(-6.283185307179586 * hf / sr) * (hpc7 + x - hpx7);
		hpx7 = x;
		x = hpc7;
	}
}
gn7 = x;
pa = v7pan;
xl = x * sqrt(1 - pa);
xr = x * sqrt(pa);
dl = dl + xl;
dr = dr + xr;
el = el + xl * v7echo;
er = er + xr * v7echo;
wl = wl + xl * v7void;
wr = wr + xr * v7void;


if (v8trig != tl8) {
	tl8 = v8trig;
	vel8 = (v8trig - 256 * floor(v8trig / 256)) / 127;
	av8 = 1;
	tt8 = 0;
	sph8 = 0;
}

x = 0;
if (av8 > 0) {
	tt8 = tt8 + 1;
	t = tt8 / sr;

	
	swp = (v8p3 - 0.5) * 2;
	swt = 0.01 * exp2(v8p4 * 7);
	lr = 0.5 * exp2(v8p5 * 6.3);
	lph8 = fract(lph8 + lr / sr);
	lf = lph8 < 0.5 ? 1 : -1;
	f = 30.868 * exp2(v8p1 * 6 + swp * 4 * (1 - exp(-t / swt)) + lf * v8p6 * v8p6 * 1.5 + v8p7 * v8p7 * noise() * 0.8);
	sph8 = fract(sph8 + min(f, 0.45 * sr) / sr);
	tri = 4 * abs(sph8 - 0.5) - 1;
	sqr = sph8 < 0.5 ? 0.8 : -0.8;
	s = tri + (sqr - tri) * v8p0;
	dec = 0.03 * exp2(v8p2 * 6.2);
	x = (s + v8p7 * noise() * 0.6) * exp(-t / dec) * (1 - exp(-t / 0.0007));
	if (t > dec * 9 + 0.02) {
		av8 = 0;
	}
	x = x * (0.3 + 0.7 * vel8) * v8lvl * 0.2817;
	
	ax = abs(x);
	if (ax > 0.6) {
		x = (0.6 + 0.4 * tanh((ax - 0.6) / 0.4)) * (x < 0 ? -1 : 1);
	}
	cu = v8cut;
	if (cu < 0.48) {
		lf = 18000 * exp2(-(0.48 - cu) * 14);
		lpc8 = lpc8 + (x - lpc8) * (1 - exp(-6.283185307179586 * lf / sr));
		x = lpc8;
	} else if (cu > 0.52) {
		hf = 20 * exp2((cu - 0.52) * 16);
		hpc8 = exp(-6.283185307179586 * hf / sr) * (hpc8 + x - hpx8);
		hpx8 = x;
		x = hpc8;
	}
}
gn8 = x;
pa = v8pan;
xl = x * sqrt(1 - pa);
xr = x * sqrt(pa);
dl = dl + xl;
dr = dr + xr;
el = el + xl * v8echo;
er = er + xr * v8echo;
wl = wl + xl * v8void;
wr = wr + xr * v8void;

out1 = dl * mvol;
out2 = dr * mvol;
out3 = el;
out4 = er;
out5 = wl;
out6 = wr;
 return [out1, out2, out3, out4, out5, out6]; }
  function set(name, v) { switch (name) { case 'v0p0': v0p0 = v; break; case 'v0p1': v0p1 = v; break; case 'v0p2': v0p2 = v; break; case 'v0p3': v0p3 = v; break; case 'v0p4': v0p4 = v; break; case 'v0p5': v0p5 = v; break; case 'v0p6': v0p6 = v; break; case 'v0p7': v0p7 = v; break; case 'v0lvl': v0lvl = v; break; case 'v0pan': v0pan = v; break; case 'v0echo': v0echo = v; break; case 'v0void': v0void = v; break; case 'v0cut': v0cut = v; break; case 'v0trig': v0trig = v; break; case 'v1p0': v1p0 = v; break; case 'v1p1': v1p1 = v; break; case 'v1p2': v1p2 = v; break; case 'v1p3': v1p3 = v; break; case 'v1p4': v1p4 = v; break; case 'v1p5': v1p5 = v; break; case 'v1p6': v1p6 = v; break; case 'v1p7': v1p7 = v; break; case 'v1lvl': v1lvl = v; break; case 'v1pan': v1pan = v; break; case 'v1echo': v1echo = v; break; case 'v1void': v1void = v; break; case 'v1cut': v1cut = v; break; case 'v1trig': v1trig = v; break; case 'v2p0': v2p0 = v; break; case 'v2p1': v2p1 = v; break; case 'v2p2': v2p2 = v; break; case 'v2p3': v2p3 = v; break; case 'v2p4': v2p4 = v; break; case 'v2p5': v2p5 = v; break; case 'v2p6': v2p6 = v; break; case 'v2p7': v2p7 = v; break; case 'v2lvl': v2lvl = v; break; case 'v2pan': v2pan = v; break; case 'v2echo': v2echo = v; break; case 'v2void': v2void = v; break; case 'v2cut': v2cut = v; break; case 'v2trig': v2trig = v; break; case 'v3p0': v3p0 = v; break; case 'v3p1': v3p1 = v; break; case 'v3p2': v3p2 = v; break; case 'v3p3': v3p3 = v; break; case 'v3p4': v3p4 = v; break; case 'v3p5': v3p5 = v; break; case 'v3p6': v3p6 = v; break; case 'v3p7': v3p7 = v; break; case 'v3lvl': v3lvl = v; break; case 'v3pan': v3pan = v; break; case 'v3echo': v3echo = v; break; case 'v3void': v3void = v; break; case 'v3cut': v3cut = v; break; case 'v3trig': v3trig = v; break; case 'v4p0': v4p0 = v; break; case 'v4p1': v4p1 = v; break; case 'v4p2': v4p2 = v; break; case 'v4p3': v4p3 = v; break; case 'v4p4': v4p4 = v; break; case 'v4p5': v4p5 = v; break; case 'v4p6': v4p6 = v; break; case 'v4p7': v4p7 = v; break; case 'v4lvl': v4lvl = v; break; case 'v4pan': v4pan = v; break; case 'v4echo': v4echo = v; break; case 'v4void': v4void = v; break; case 'v4cut': v4cut = v; break; case 'v4trig': v4trig = v; break; case 'v5p0': v5p0 = v; break; case 'v5p1': v5p1 = v; break; case 'v5p2': v5p2 = v; break; case 'v5p3': v5p3 = v; break; case 'v5p4': v5p4 = v; break; case 'v5p5': v5p5 = v; break; case 'v5p6': v5p6 = v; break; case 'v5p7': v5p7 = v; break; case 'v5lvl': v5lvl = v; break; case 'v5pan': v5pan = v; break; case 'v5echo': v5echo = v; break; case 'v5void': v5void = v; break; case 'v5cut': v5cut = v; break; case 'v5trig': v5trig = v; break; case 'v6p0': v6p0 = v; break; case 'v6p1': v6p1 = v; break; case 'v6p2': v6p2 = v; break; case 'v6p3': v6p3 = v; break; case 'v6p4': v6p4 = v; break; case 'v6p5': v6p5 = v; break; case 'v6p6': v6p6 = v; break; case 'v6p7': v6p7 = v; break; case 'v6lvl': v6lvl = v; break; case 'v6pan': v6pan = v; break; case 'v6echo': v6echo = v; break; case 'v6void': v6void = v; break; case 'v6cut': v6cut = v; break; case 'v6trig': v6trig = v; break; case 'v7p0': v7p0 = v; break; case 'v7p1': v7p1 = v; break; case 'v7p2': v7p2 = v; break; case 'v7p3': v7p3 = v; break; case 'v7p4': v7p4 = v; break; case 'v7p5': v7p5 = v; break; case 'v7p6': v7p6 = v; break; case 'v7p7': v7p7 = v; break; case 'v7lvl': v7lvl = v; break; case 'v7pan': v7pan = v; break; case 'v7echo': v7echo = v; break; case 'v7void': v7void = v; break; case 'v7cut': v7cut = v; break; case 'v7trig': v7trig = v; break; case 'v8p0': v8p0 = v; break; case 'v8p1': v8p1 = v; break; case 'v8p2': v8p2 = v; break; case 'v8p3': v8p3 = v; break; case 'v8p4': v8p4 = v; break; case 'v8p5': v8p5 = v; break; case 'v8p6': v8p6 = v; break; case 'v8p7': v8p7 = v; break; case 'v8lvl': v8lvl = v; break; case 'v8pan': v8pan = v; break; case 'v8echo': v8echo = v; break; case 'v8void': v8void = v; break; case 'v8cut': v8cut = v; break; case 'v8trig': v8trig = v; break; case 'mvol': mvol = v; break; case 'rumamt': rumamt = v; break; case 'rumdec': rumdec = v; break; case 'rumtone': rumtone = v; break; case 'rumdrive': rumdrive = v; break; case 'rumduck': rumduck = v; break; case 'rumgroove': rumgroove = v; break; case 'rummotion': rummotion = v; break; case 'rumwidth': rumwidth = v; break; case 'ptempo': ptempo = v; break; default: throw new Error("no param " + name); } }
  return { tick: tick, set: set };
}
window.makeBunkerVoices = makeBunkerVoices;
})();
