// VAULT voices (preview) — generated. MIT license.
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
function makeVaultVoices(SR) {
  "use strict";
  var M = Math;
  var palgo = 0.0;
  var prc = 1.0;
  var pra = 1.0;
  var prb = 2.0;
  var pia = 0.2;
  var pib = 0.1;
  var pfb = 0.1;
  var pbmix = 0.5;
  var pchuff = 0.15;
  var psub = 0.0;
  var pdrift = 0.3;
  var pdetune = 0.2;
  var pftype = 1.0;
  var pcut = 0.35;
  var preso = 0.25;
  var pkey = 0.5;
  var penv = 0.45;
  var pfa = 0.0;
  var pfd = 0.35;
  var pfs = 0.0;
  var pfr = 0.3;
  var ppoly = 0.2;
  var paa = 0.02;
  var pad = 0.4;
  var pas = 0.1;
  var par = 0.3;
  var pvel = 0.5;
  var plrate = 0.25;
  var plshape = 0.0;
  var pl2cut = 0.0;
  var pl2idx = 0.0;
  var pl2pitch = 0.0;
  var pspread = 0.5;
  var pdubeq = 1.0;
  var pvol = 1.0;
  var v0f = 220.0;
  var v0gate = 0.0;
  var v1f = 220.0;
  var v1gate = 0.0;
  var v2f = 220.0;
  var v2gate = 0.0;
  var v3f = 220.0;
  var v3gate = 0.0;
  var v4f = 220.0;
  var v4gate = 0.0;
  var v5f = 220.0;
  var v5gate = 0.0;
  var v0gl = 0.0;
  var v0vel = 0.0;
  var v0st = 0.0;
  var v0fst = 0.0;
  var v0ae = 0.0;
  var v0fe = 0.0;
  var v0vv = 0.0;
  var v0nf = 220.0;
  var v0dr = 0.0;
  var v0pc = 0.0;
  var v0pa = 0.0;
  var v0pb = 0.0;
  var v0ps = 0.0;
  var v0fbv = 0.0;
  var v0s1 = 0.0;
  var v0s2 = 0.0;
  var v0s3 = 0.0;
  var v0s4 = 0.0;
  var v1gl = 0.0;
  var v1vel = 0.0;
  var v1st = 0.0;
  var v1fst = 0.0;
  var v1ae = 0.0;
  var v1fe = 0.0;
  var v1vv = 0.0;
  var v1nf = 220.0;
  var v1dr = 0.0;
  var v1pc = 0.0;
  var v1pa = 0.0;
  var v1pb = 0.0;
  var v1ps = 0.0;
  var v1fbv = 0.0;
  var v1s1 = 0.0;
  var v1s2 = 0.0;
  var v1s3 = 0.0;
  var v1s4 = 0.0;
  var v2gl = 0.0;
  var v2vel = 0.0;
  var v2st = 0.0;
  var v2fst = 0.0;
  var v2ae = 0.0;
  var v2fe = 0.0;
  var v2vv = 0.0;
  var v2nf = 220.0;
  var v2dr = 0.0;
  var v2pc = 0.0;
  var v2pa = 0.0;
  var v2pb = 0.0;
  var v2ps = 0.0;
  var v2fbv = 0.0;
  var v2s1 = 0.0;
  var v2s2 = 0.0;
  var v2s3 = 0.0;
  var v2s4 = 0.0;
  var v3gl = 0.0;
  var v3vel = 0.0;
  var v3st = 0.0;
  var v3fst = 0.0;
  var v3ae = 0.0;
  var v3fe = 0.0;
  var v3vv = 0.0;
  var v3nf = 220.0;
  var v3dr = 0.0;
  var v3pc = 0.0;
  var v3pa = 0.0;
  var v3pb = 0.0;
  var v3ps = 0.0;
  var v3fbv = 0.0;
  var v3s1 = 0.0;
  var v3s2 = 0.0;
  var v3s3 = 0.0;
  var v3s4 = 0.0;
  var v4gl = 0.0;
  var v4vel = 0.0;
  var v4st = 0.0;
  var v4fst = 0.0;
  var v4ae = 0.0;
  var v4fe = 0.0;
  var v4vv = 0.0;
  var v4nf = 220.0;
  var v4dr = 0.0;
  var v4pc = 0.0;
  var v4pa = 0.0;
  var v4pb = 0.0;
  var v4ps = 0.0;
  var v4fbv = 0.0;
  var v4s1 = 0.0;
  var v4s2 = 0.0;
  var v4s3 = 0.0;
  var v4s4 = 0.0;
  var v5gl = 0.0;
  var v5vel = 0.0;
  var v5st = 0.0;
  var v5fst = 0.0;
  var v5ae = 0.0;
  var v5fe = 0.0;
  var v5vv = 0.0;
  var v5nf = 220.0;
  var v5dr = 0.0;
  var v5pc = 0.0;
  var v5pa = 0.0;
  var v5pb = 0.0;
  var v5ps = 0.0;
  var v5fbv = 0.0;
  var v5s1 = 0.0;
  var v5s2 = 0.0;
  var v5s3 = 0.0;
  var v5s4 = 0.0;
  var newest = 0.0;
  var lph = 0.0;
  var lrnd = 0.0;
  var lrt = 0.0;
  var lsm = 0.0;
  var e1 = 0.0;
  var e2 = 0.0;
  var e3 = 0.0;
  var e4 = 0.0;
  var e5 = 0.0;
  var e6 = 0.0;
  var e7 = 0.0;
  var e8 = 0.0;
  var sr, r48, out1 = 0, out2 = 0, out3 = 0, out4 = 0, out5 = 0;
  function tick(in1, in2, in3, in4) { var act, al, bmix, bp, bp1, bp2, chuff, cut, dl, dr, drift, dtn, eqm, f, fbk, fc, fe, fenv, fsus, ft, gg, ghp, gl45, glo, gmid, hl, hp, hp1, hp2, hr, ia, idxa, idxb, ka, kd, kdr, keyt, kfa, kfd, kfr, kq, kr, l2i, lfc, lfp, lp, lp1, lp2, lr, lsh, lv, oa, ob, ol, orr, pn, ql, qr, ra, rb, rc, ri, rq, sus, velamt, vl, x, y;       
         


       
       
       
     
   
        
    
   

      
     
    
      
     
         
      

       
     

      
       

       
      

   
     
      
      
    

   
      















































































































































sr = SR;

al = floor(palgo + 0.5);
ri = floor(prc + 0.5);
rc = ri == 0 ? 0.5 : (ri == 1 ? 1 : (ri == 2 ? 2 : (ri == 3 ? 3 : (ri == 4 ? 4 : (ri == 5 ? 5 : 7)))));
ri = floor(pra + 0.5);
ra = ri == 0 ? 0.5 : (ri == 1 ? 1 : (ri == 2 ? 2 : (ri == 3 ? 3 : (ri == 4 ? 4 : (ri == 5 ? 5 : 7)))));
ri = floor(prb + 0.5);
rb = ri == 0 ? 0.5 : (ri == 1 ? 1 : (ri == 2 ? 2 : (ri == 3 ? 3 : (ri == 4 ? 4 : (ri == 5 ? 5 : 7)))));
idxa = pia * 6;
idxb = pib * 6;
fbk = pfb * 0.6;
bmix = pbmix;
chuff = pchuff * 0.3;
drift = (0.001 + 0.014 * pdrift) * 4;
kdr = 1 - exp(-1 / (0.4 * sr));
dtn = pdetune * 0.01;
ft = floor(pftype + 0.5);
cut = 80 * exp2(pcut * 6.23);
rq = preso * 0.65;
kq = 2 - 1.95 * rq * (ft == 1 ? 1 : 1.25);
kq = max(kq, 0.08);
keyt = pkey;
velamt = pvel;

ka = 1 - exp(-1 / ((0.001 * exp2(paa * 9.97)) * sr));
kd = 1 - exp(-1 / ((0.03 * exp2(pad * 6.64)) * sr / 2.3));
sus = pas;
kr = exp(-1 / ((0.02 * exp2(par * 6.64)) * sr / 4.6));
kfa = 1 - exp(-1 / ((0.001 * exp2(pfa * 9.97)) * sr));
kfd = 1 - exp(-1 / ((0.01 * exp2(pfd * 7.64)) * sr / 2.3));
fsus = pfs;
kfr = exp(-1 / ((0.02 * exp2(pfr * 6.64)) * sr / 4.6));

lr = 0.01 * exp2(plrate * 9.97);
lph = fract(lph + lr / sr);
lsh = floor(plshape + 0.5);
lv = sin(6.283185307179586 * lph);
if (lsh == 1) {
	lv = 1 - 4 * abs(lph - 0.5);
} else if (lsh == 2) {
	lv = lph < 0.5 ? 1 : -1;
} else if (lsh == 3) {
	if (lph < lrt) {
		lrnd = noise();
	}
	lsm = lsm + (lrnd - lsm) * (1 - exp(-1 / (0.25 / lr * sr)));
	lv = lsm;
}
lrt = lph;
lfc = (pl2cut - 0.5) * 4 * lv;
l2i = pl2idx * 3 * (lv * 0.5 + 0.5);
lfp = pl2pitch * 0.0175 * lv;

dl = 0;
dr = 0;
act = 0;
fenv = 0;



if (v0gate != v0gl) {
	v0gl = v0gate;
	v0vel = (v0gate % 256) / 127;
	if (v0vel > 0) {
		v0st = 1;
		v0fst = 1;
		v0vv = v0vel;
		newest = 0;
		v0nf = v0f;
	} else {
		v0st = 3;
		v0fst = 3;
	}
}

if (v0st == 1) {
	v0ae = v0ae + (1.2 - v0ae) * ka;
	if (v0ae >= 1) {
		v0ae = 1;
		v0st = 2;
	}
} else if (v0st == 2) {
	v0ae = v0ae + (sus - v0ae) * kd;
} else if (v0st == 3) {
	v0ae = v0ae * kr;
	if (v0ae < 0.0001) {
		v0ae = 0;
		v0st = 0;
	}
}
if (v0fst == 1) {
	v0fe = v0fe + (1.2 - v0fe) * kfa;
	if (v0fe >= 1) {
		v0fe = 1;
		v0fst = 2;
	}
} else if (v0fst == 2) {
	v0fe = v0fe + (fsus - v0fe) * kfd;
} else if (v0fst == 3) {
	v0fe = v0fe * kfr;
}
if (v0st > 0) {
	
	v0dr = v0dr + (noise() * 0.5 - v0dr) * kdr;
	f = v0f * (1 + v0dr * drift + lfp);
	vl = 1 - velamt + velamt * v0vv;
	fe = v0fe * (1 - velamt * 0.5 + velamt * 0.5 * v0vv);
	ia = (idxa + ppoly * 4 * fe + l2i) * vl;
	v0pc = fract(v0pc + f * rc / sr);
	v0pa = fract(v0pa + f * ra * (1 + dtn) / sr);
	v0pb = fract(v0pb + f * rb * (1 - dtn) / sr);
	v0ps = fract(v0ps + f * 0.5 / sr);
	if (al == 0) {
		ob = sin(6.283185307179586 * (v0pb + v0fbv * fbk));
		v0fbv = ob;
		oa = sin(6.283185307179586 * (v0pa + ob * idxb * 0.159));
		x = sin(6.283185307179586 * (v0pc + oa * ia * 0.159));
	} else if (al == 1) {
		ob = sin(6.283185307179586 * (v0pb + v0fbv * fbk));
		v0fbv = ob;
		oa = sin(6.283185307179586 * v0pa);
		x = sin(6.283185307179586 * (v0pc + (oa * ia + ob * idxb) * 0.159));
	} else if (al == 2) {
		oa = sin(6.283185307179586 * (v0pa + v0fbv * fbk));
		v0fbv = oa;
		ob = sin(6.283185307179586 * v0pb);
		x = sin(6.283185307179586 * (v0pc + oa * ia * 0.159)) * (1 - bmix * 0.5) + ob * bmix * 0.7;
	} else {
		oa = sin(6.283185307179586 * (v0pa + v0fbv * fbk));
		v0fbv = oa;
		ob = sin(6.283185307179586 * (v0pb + oa * idxb * 0.159));
		x = sin(6.283185307179586 * (v0pc + oa * ia * 0.159)) * (1 - bmix * 0.5) + ob * bmix * 0.7;
	}
	x = x + sin(6.283185307179586 * v0ps) * psub * 0.8 + noise() * chuff * (0.4 + v0fe);
	
	fc = cut * exp2(keyt * log2(f / 261.63) + penv * 4 * fe + lfc);
	fc = clamp(fc, 30, sr * 0.42);
	gg = tan(3.141592653589793 * fc / sr);
	if (ft == 1) {
		
		x = tanh(x * 1.3) / 1.3;
		hp1 = (x - (2 * 0.7071) * v0s1 - gg * v0s1 - v0s2) / (1 + 2 * 0.7071 * gg + gg * gg);
		bp1 = gg * hp1 + v0s1;
		lp1 = gg * bp1 + v0s2;
		v0s1 = gg * hp1 + bp1;
		v0s2 = gg * bp1 + lp1;
		hp2 = (lp1 - kq * v0s3 - gg * v0s3 - v0s4) / (1 + kq * gg + gg * gg);
		bp2 = gg * hp2 + v0s3;
		lp2 = gg * bp2 + v0s4;
		v0s3 = gg * hp2 + bp2;
		v0s4 = gg * bp2 + lp2;
		y = lp2 * (1 + rq * 0.6);
	} else {
		
		hp = (x - kq * v0s1 - gg * v0s1 - v0s2) / (1 + kq * gg + gg * gg);
		bp = gg * hp + v0s1;
		lp = gg * bp + v0s2;
		v0s1 = gg * hp + bp;
		v0s2 = gg * bp + lp;
		y = ft == 0 ? lp : (ft == 2 ? bp * (1 + rq) : hp);
		y = tanh(y * 1.2) / 1.2;
	}
	y = y * v0ae * vl;
	act = act + v0ae;
	pn = -1 * pspread;
	dl = dl + y * sqrt(0.5 - pn * 0.5);
	dr = dr + y * sqrt(0.5 + pn * 0.5);
}
if (newest == 0) {
	fenv = v0fe;
}



if (v1gate != v1gl) {
	v1gl = v1gate;
	v1vel = (v1gate % 256) / 127;
	if (v1vel > 0) {
		v1st = 1;
		v1fst = 1;
		v1vv = v1vel;
		newest = 1;
		v1nf = v1f;
	} else {
		v1st = 3;
		v1fst = 3;
	}
}

if (v1st == 1) {
	v1ae = v1ae + (1.2 - v1ae) * ka;
	if (v1ae >= 1) {
		v1ae = 1;
		v1st = 2;
	}
} else if (v1st == 2) {
	v1ae = v1ae + (sus - v1ae) * kd;
} else if (v1st == 3) {
	v1ae = v1ae * kr;
	if (v1ae < 0.0001) {
		v1ae = 0;
		v1st = 0;
	}
}
if (v1fst == 1) {
	v1fe = v1fe + (1.2 - v1fe) * kfa;
	if (v1fe >= 1) {
		v1fe = 1;
		v1fst = 2;
	}
} else if (v1fst == 2) {
	v1fe = v1fe + (fsus - v1fe) * kfd;
} else if (v1fst == 3) {
	v1fe = v1fe * kfr;
}
if (v1st > 0) {
	
	v1dr = v1dr + (noise() * 0.5 - v1dr) * kdr;
	f = v1f * (1 + v1dr * drift + lfp);
	vl = 1 - velamt + velamt * v1vv;
	fe = v1fe * (1 - velamt * 0.5 + velamt * 0.5 * v1vv);
	ia = (idxa + ppoly * 4 * fe + l2i) * vl;
	v1pc = fract(v1pc + f * rc / sr);
	v1pa = fract(v1pa + f * ra * (1 + dtn) / sr);
	v1pb = fract(v1pb + f * rb * (1 - dtn) / sr);
	v1ps = fract(v1ps + f * 0.5 / sr);
	if (al == 0) {
		ob = sin(6.283185307179586 * (v1pb + v1fbv * fbk));
		v1fbv = ob;
		oa = sin(6.283185307179586 * (v1pa + ob * idxb * 0.159));
		x = sin(6.283185307179586 * (v1pc + oa * ia * 0.159));
	} else if (al == 1) {
		ob = sin(6.283185307179586 * (v1pb + v1fbv * fbk));
		v1fbv = ob;
		oa = sin(6.283185307179586 * v1pa);
		x = sin(6.283185307179586 * (v1pc + (oa * ia + ob * idxb) * 0.159));
	} else if (al == 2) {
		oa = sin(6.283185307179586 * (v1pa + v1fbv * fbk));
		v1fbv = oa;
		ob = sin(6.283185307179586 * v1pb);
		x = sin(6.283185307179586 * (v1pc + oa * ia * 0.159)) * (1 - bmix * 0.5) + ob * bmix * 0.7;
	} else {
		oa = sin(6.283185307179586 * (v1pa + v1fbv * fbk));
		v1fbv = oa;
		ob = sin(6.283185307179586 * (v1pb + oa * idxb * 0.159));
		x = sin(6.283185307179586 * (v1pc + oa * ia * 0.159)) * (1 - bmix * 0.5) + ob * bmix * 0.7;
	}
	x = x + sin(6.283185307179586 * v1ps) * psub * 0.8 + noise() * chuff * (0.4 + v1fe);
	
	fc = cut * exp2(keyt * log2(f / 261.63) + penv * 4 * fe + lfc);
	fc = clamp(fc, 30, sr * 0.42);
	gg = tan(3.141592653589793 * fc / sr);
	if (ft == 1) {
		
		x = tanh(x * 1.3) / 1.3;
		hp1 = (x - (2 * 0.7071) * v1s1 - gg * v1s1 - v1s2) / (1 + 2 * 0.7071 * gg + gg * gg);
		bp1 = gg * hp1 + v1s1;
		lp1 = gg * bp1 + v1s2;
		v1s1 = gg * hp1 + bp1;
		v1s2 = gg * bp1 + lp1;
		hp2 = (lp1 - kq * v1s3 - gg * v1s3 - v1s4) / (1 + kq * gg + gg * gg);
		bp2 = gg * hp2 + v1s3;
		lp2 = gg * bp2 + v1s4;
		v1s3 = gg * hp2 + bp2;
		v1s4 = gg * bp2 + lp2;
		y = lp2 * (1 + rq * 0.6);
	} else {
		
		hp = (x - kq * v1s1 - gg * v1s1 - v1s2) / (1 + kq * gg + gg * gg);
		bp = gg * hp + v1s1;
		lp = gg * bp + v1s2;
		v1s1 = gg * hp + bp;
		v1s2 = gg * bp + lp;
		y = ft == 0 ? lp : (ft == 2 ? bp * (1 + rq) : hp);
		y = tanh(y * 1.2) / 1.2;
	}
	y = y * v1ae * vl;
	act = act + v1ae;
	pn = 1 * pspread;
	dl = dl + y * sqrt(0.5 - pn * 0.5);
	dr = dr + y * sqrt(0.5 + pn * 0.5);
}
if (newest == 1) {
	fenv = v1fe;
}



if (v2gate != v2gl) {
	v2gl = v2gate;
	v2vel = (v2gate % 256) / 127;
	if (v2vel > 0) {
		v2st = 1;
		v2fst = 1;
		v2vv = v2vel;
		newest = 2;
		v2nf = v2f;
	} else {
		v2st = 3;
		v2fst = 3;
	}
}

if (v2st == 1) {
	v2ae = v2ae + (1.2 - v2ae) * ka;
	if (v2ae >= 1) {
		v2ae = 1;
		v2st = 2;
	}
} else if (v2st == 2) {
	v2ae = v2ae + (sus - v2ae) * kd;
} else if (v2st == 3) {
	v2ae = v2ae * kr;
	if (v2ae < 0.0001) {
		v2ae = 0;
		v2st = 0;
	}
}
if (v2fst == 1) {
	v2fe = v2fe + (1.2 - v2fe) * kfa;
	if (v2fe >= 1) {
		v2fe = 1;
		v2fst = 2;
	}
} else if (v2fst == 2) {
	v2fe = v2fe + (fsus - v2fe) * kfd;
} else if (v2fst == 3) {
	v2fe = v2fe * kfr;
}
if (v2st > 0) {
	
	v2dr = v2dr + (noise() * 0.5 - v2dr) * kdr;
	f = v2f * (1 + v2dr * drift + lfp);
	vl = 1 - velamt + velamt * v2vv;
	fe = v2fe * (1 - velamt * 0.5 + velamt * 0.5 * v2vv);
	ia = (idxa + ppoly * 4 * fe + l2i) * vl;
	v2pc = fract(v2pc + f * rc / sr);
	v2pa = fract(v2pa + f * ra * (1 + dtn) / sr);
	v2pb = fract(v2pb + f * rb * (1 - dtn) / sr);
	v2ps = fract(v2ps + f * 0.5 / sr);
	if (al == 0) {
		ob = sin(6.283185307179586 * (v2pb + v2fbv * fbk));
		v2fbv = ob;
		oa = sin(6.283185307179586 * (v2pa + ob * idxb * 0.159));
		x = sin(6.283185307179586 * (v2pc + oa * ia * 0.159));
	} else if (al == 1) {
		ob = sin(6.283185307179586 * (v2pb + v2fbv * fbk));
		v2fbv = ob;
		oa = sin(6.283185307179586 * v2pa);
		x = sin(6.283185307179586 * (v2pc + (oa * ia + ob * idxb) * 0.159));
	} else if (al == 2) {
		oa = sin(6.283185307179586 * (v2pa + v2fbv * fbk));
		v2fbv = oa;
		ob = sin(6.283185307179586 * v2pb);
		x = sin(6.283185307179586 * (v2pc + oa * ia * 0.159)) * (1 - bmix * 0.5) + ob * bmix * 0.7;
	} else {
		oa = sin(6.283185307179586 * (v2pa + v2fbv * fbk));
		v2fbv = oa;
		ob = sin(6.283185307179586 * (v2pb + oa * idxb * 0.159));
		x = sin(6.283185307179586 * (v2pc + oa * ia * 0.159)) * (1 - bmix * 0.5) + ob * bmix * 0.7;
	}
	x = x + sin(6.283185307179586 * v2ps) * psub * 0.8 + noise() * chuff * (0.4 + v2fe);
	
	fc = cut * exp2(keyt * log2(f / 261.63) + penv * 4 * fe + lfc);
	fc = clamp(fc, 30, sr * 0.42);
	gg = tan(3.141592653589793 * fc / sr);
	if (ft == 1) {
		
		x = tanh(x * 1.3) / 1.3;
		hp1 = (x - (2 * 0.7071) * v2s1 - gg * v2s1 - v2s2) / (1 + 2 * 0.7071 * gg + gg * gg);
		bp1 = gg * hp1 + v2s1;
		lp1 = gg * bp1 + v2s2;
		v2s1 = gg * hp1 + bp1;
		v2s2 = gg * bp1 + lp1;
		hp2 = (lp1 - kq * v2s3 - gg * v2s3 - v2s4) / (1 + kq * gg + gg * gg);
		bp2 = gg * hp2 + v2s3;
		lp2 = gg * bp2 + v2s4;
		v2s3 = gg * hp2 + bp2;
		v2s4 = gg * bp2 + lp2;
		y = lp2 * (1 + rq * 0.6);
	} else {
		
		hp = (x - kq * v2s1 - gg * v2s1 - v2s2) / (1 + kq * gg + gg * gg);
		bp = gg * hp + v2s1;
		lp = gg * bp + v2s2;
		v2s1 = gg * hp + bp;
		v2s2 = gg * bp + lp;
		y = ft == 0 ? lp : (ft == 2 ? bp * (1 + rq) : hp);
		y = tanh(y * 1.2) / 1.2;
	}
	y = y * v2ae * vl;
	act = act + v2ae;
	pn = -0.5 * pspread;
	dl = dl + y * sqrt(0.5 - pn * 0.5);
	dr = dr + y * sqrt(0.5 + pn * 0.5);
}
if (newest == 2) {
	fenv = v2fe;
}



if (v3gate != v3gl) {
	v3gl = v3gate;
	v3vel = (v3gate % 256) / 127;
	if (v3vel > 0) {
		v3st = 1;
		v3fst = 1;
		v3vv = v3vel;
		newest = 3;
		v3nf = v3f;
	} else {
		v3st = 3;
		v3fst = 3;
	}
}

if (v3st == 1) {
	v3ae = v3ae + (1.2 - v3ae) * ka;
	if (v3ae >= 1) {
		v3ae = 1;
		v3st = 2;
	}
} else if (v3st == 2) {
	v3ae = v3ae + (sus - v3ae) * kd;
} else if (v3st == 3) {
	v3ae = v3ae * kr;
	if (v3ae < 0.0001) {
		v3ae = 0;
		v3st = 0;
	}
}
if (v3fst == 1) {
	v3fe = v3fe + (1.2 - v3fe) * kfa;
	if (v3fe >= 1) {
		v3fe = 1;
		v3fst = 2;
	}
} else if (v3fst == 2) {
	v3fe = v3fe + (fsus - v3fe) * kfd;
} else if (v3fst == 3) {
	v3fe = v3fe * kfr;
}
if (v3st > 0) {
	
	v3dr = v3dr + (noise() * 0.5 - v3dr) * kdr;
	f = v3f * (1 + v3dr * drift + lfp);
	vl = 1 - velamt + velamt * v3vv;
	fe = v3fe * (1 - velamt * 0.5 + velamt * 0.5 * v3vv);
	ia = (idxa + ppoly * 4 * fe + l2i) * vl;
	v3pc = fract(v3pc + f * rc / sr);
	v3pa = fract(v3pa + f * ra * (1 + dtn) / sr);
	v3pb = fract(v3pb + f * rb * (1 - dtn) / sr);
	v3ps = fract(v3ps + f * 0.5 / sr);
	if (al == 0) {
		ob = sin(6.283185307179586 * (v3pb + v3fbv * fbk));
		v3fbv = ob;
		oa = sin(6.283185307179586 * (v3pa + ob * idxb * 0.159));
		x = sin(6.283185307179586 * (v3pc + oa * ia * 0.159));
	} else if (al == 1) {
		ob = sin(6.283185307179586 * (v3pb + v3fbv * fbk));
		v3fbv = ob;
		oa = sin(6.283185307179586 * v3pa);
		x = sin(6.283185307179586 * (v3pc + (oa * ia + ob * idxb) * 0.159));
	} else if (al == 2) {
		oa = sin(6.283185307179586 * (v3pa + v3fbv * fbk));
		v3fbv = oa;
		ob = sin(6.283185307179586 * v3pb);
		x = sin(6.283185307179586 * (v3pc + oa * ia * 0.159)) * (1 - bmix * 0.5) + ob * bmix * 0.7;
	} else {
		oa = sin(6.283185307179586 * (v3pa + v3fbv * fbk));
		v3fbv = oa;
		ob = sin(6.283185307179586 * (v3pb + oa * idxb * 0.159));
		x = sin(6.283185307179586 * (v3pc + oa * ia * 0.159)) * (1 - bmix * 0.5) + ob * bmix * 0.7;
	}
	x = x + sin(6.283185307179586 * v3ps) * psub * 0.8 + noise() * chuff * (0.4 + v3fe);
	
	fc = cut * exp2(keyt * log2(f / 261.63) + penv * 4 * fe + lfc);
	fc = clamp(fc, 30, sr * 0.42);
	gg = tan(3.141592653589793 * fc / sr);
	if (ft == 1) {
		
		x = tanh(x * 1.3) / 1.3;
		hp1 = (x - (2 * 0.7071) * v3s1 - gg * v3s1 - v3s2) / (1 + 2 * 0.7071 * gg + gg * gg);
		bp1 = gg * hp1 + v3s1;
		lp1 = gg * bp1 + v3s2;
		v3s1 = gg * hp1 + bp1;
		v3s2 = gg * bp1 + lp1;
		hp2 = (lp1 - kq * v3s3 - gg * v3s3 - v3s4) / (1 + kq * gg + gg * gg);
		bp2 = gg * hp2 + v3s3;
		lp2 = gg * bp2 + v3s4;
		v3s3 = gg * hp2 + bp2;
		v3s4 = gg * bp2 + lp2;
		y = lp2 * (1 + rq * 0.6);
	} else {
		
		hp = (x - kq * v3s1 - gg * v3s1 - v3s2) / (1 + kq * gg + gg * gg);
		bp = gg * hp + v3s1;
		lp = gg * bp + v3s2;
		v3s1 = gg * hp + bp;
		v3s2 = gg * bp + lp;
		y = ft == 0 ? lp : (ft == 2 ? bp * (1 + rq) : hp);
		y = tanh(y * 1.2) / 1.2;
	}
	y = y * v3ae * vl;
	act = act + v3ae;
	pn = 0.5 * pspread;
	dl = dl + y * sqrt(0.5 - pn * 0.5);
	dr = dr + y * sqrt(0.5 + pn * 0.5);
}
if (newest == 3) {
	fenv = v3fe;
}



if (v4gate != v4gl) {
	v4gl = v4gate;
	v4vel = (v4gate % 256) / 127;
	if (v4vel > 0) {
		v4st = 1;
		v4fst = 1;
		v4vv = v4vel;
		newest = 4;
		v4nf = v4f;
	} else {
		v4st = 3;
		v4fst = 3;
	}
}

if (v4st == 1) {
	v4ae = v4ae + (1.2 - v4ae) * ka;
	if (v4ae >= 1) {
		v4ae = 1;
		v4st = 2;
	}
} else if (v4st == 2) {
	v4ae = v4ae + (sus - v4ae) * kd;
} else if (v4st == 3) {
	v4ae = v4ae * kr;
	if (v4ae < 0.0001) {
		v4ae = 0;
		v4st = 0;
	}
}
if (v4fst == 1) {
	v4fe = v4fe + (1.2 - v4fe) * kfa;
	if (v4fe >= 1) {
		v4fe = 1;
		v4fst = 2;
	}
} else if (v4fst == 2) {
	v4fe = v4fe + (fsus - v4fe) * kfd;
} else if (v4fst == 3) {
	v4fe = v4fe * kfr;
}
if (v4st > 0) {
	
	v4dr = v4dr + (noise() * 0.5 - v4dr) * kdr;
	f = v4f * (1 + v4dr * drift + lfp);
	vl = 1 - velamt + velamt * v4vv;
	fe = v4fe * (1 - velamt * 0.5 + velamt * 0.5 * v4vv);
	ia = (idxa + ppoly * 4 * fe + l2i) * vl;
	v4pc = fract(v4pc + f * rc / sr);
	v4pa = fract(v4pa + f * ra * (1 + dtn) / sr);
	v4pb = fract(v4pb + f * rb * (1 - dtn) / sr);
	v4ps = fract(v4ps + f * 0.5 / sr);
	if (al == 0) {
		ob = sin(6.283185307179586 * (v4pb + v4fbv * fbk));
		v4fbv = ob;
		oa = sin(6.283185307179586 * (v4pa + ob * idxb * 0.159));
		x = sin(6.283185307179586 * (v4pc + oa * ia * 0.159));
	} else if (al == 1) {
		ob = sin(6.283185307179586 * (v4pb + v4fbv * fbk));
		v4fbv = ob;
		oa = sin(6.283185307179586 * v4pa);
		x = sin(6.283185307179586 * (v4pc + (oa * ia + ob * idxb) * 0.159));
	} else if (al == 2) {
		oa = sin(6.283185307179586 * (v4pa + v4fbv * fbk));
		v4fbv = oa;
		ob = sin(6.283185307179586 * v4pb);
		x = sin(6.283185307179586 * (v4pc + oa * ia * 0.159)) * (1 - bmix * 0.5) + ob * bmix * 0.7;
	} else {
		oa = sin(6.283185307179586 * (v4pa + v4fbv * fbk));
		v4fbv = oa;
		ob = sin(6.283185307179586 * (v4pb + oa * idxb * 0.159));
		x = sin(6.283185307179586 * (v4pc + oa * ia * 0.159)) * (1 - bmix * 0.5) + ob * bmix * 0.7;
	}
	x = x + sin(6.283185307179586 * v4ps) * psub * 0.8 + noise() * chuff * (0.4 + v4fe);
	
	fc = cut * exp2(keyt * log2(f / 261.63) + penv * 4 * fe + lfc);
	fc = clamp(fc, 30, sr * 0.42);
	gg = tan(3.141592653589793 * fc / sr);
	if (ft == 1) {
		
		x = tanh(x * 1.3) / 1.3;
		hp1 = (x - (2 * 0.7071) * v4s1 - gg * v4s1 - v4s2) / (1 + 2 * 0.7071 * gg + gg * gg);
		bp1 = gg * hp1 + v4s1;
		lp1 = gg * bp1 + v4s2;
		v4s1 = gg * hp1 + bp1;
		v4s2 = gg * bp1 + lp1;
		hp2 = (lp1 - kq * v4s3 - gg * v4s3 - v4s4) / (1 + kq * gg + gg * gg);
		bp2 = gg * hp2 + v4s3;
		lp2 = gg * bp2 + v4s4;
		v4s3 = gg * hp2 + bp2;
		v4s4 = gg * bp2 + lp2;
		y = lp2 * (1 + rq * 0.6);
	} else {
		
		hp = (x - kq * v4s1 - gg * v4s1 - v4s2) / (1 + kq * gg + gg * gg);
		bp = gg * hp + v4s1;
		lp = gg * bp + v4s2;
		v4s1 = gg * hp + bp;
		v4s2 = gg * bp + lp;
		y = ft == 0 ? lp : (ft == 2 ? bp * (1 + rq) : hp);
		y = tanh(y * 1.2) / 1.2;
	}
	y = y * v4ae * vl;
	act = act + v4ae;
	pn = -0.2 * pspread;
	dl = dl + y * sqrt(0.5 - pn * 0.5);
	dr = dr + y * sqrt(0.5 + pn * 0.5);
}
if (newest == 4) {
	fenv = v4fe;
}



if (v5gate != v5gl) {
	v5gl = v5gate;
	v5vel = (v5gate % 256) / 127;
	if (v5vel > 0) {
		v5st = 1;
		v5fst = 1;
		v5vv = v5vel;
		newest = 5;
		v5nf = v5f;
	} else {
		v5st = 3;
		v5fst = 3;
	}
}

if (v5st == 1) {
	v5ae = v5ae + (1.2 - v5ae) * ka;
	if (v5ae >= 1) {
		v5ae = 1;
		v5st = 2;
	}
} else if (v5st == 2) {
	v5ae = v5ae + (sus - v5ae) * kd;
} else if (v5st == 3) {
	v5ae = v5ae * kr;
	if (v5ae < 0.0001) {
		v5ae = 0;
		v5st = 0;
	}
}
if (v5fst == 1) {
	v5fe = v5fe + (1.2 - v5fe) * kfa;
	if (v5fe >= 1) {
		v5fe = 1;
		v5fst = 2;
	}
} else if (v5fst == 2) {
	v5fe = v5fe + (fsus - v5fe) * kfd;
} else if (v5fst == 3) {
	v5fe = v5fe * kfr;
}
if (v5st > 0) {
	
	v5dr = v5dr + (noise() * 0.5 - v5dr) * kdr;
	f = v5f * (1 + v5dr * drift + lfp);
	vl = 1 - velamt + velamt * v5vv;
	fe = v5fe * (1 - velamt * 0.5 + velamt * 0.5 * v5vv);
	ia = (idxa + ppoly * 4 * fe + l2i) * vl;
	v5pc = fract(v5pc + f * rc / sr);
	v5pa = fract(v5pa + f * ra * (1 + dtn) / sr);
	v5pb = fract(v5pb + f * rb * (1 - dtn) / sr);
	v5ps = fract(v5ps + f * 0.5 / sr);
	if (al == 0) {
		ob = sin(6.283185307179586 * (v5pb + v5fbv * fbk));
		v5fbv = ob;
		oa = sin(6.283185307179586 * (v5pa + ob * idxb * 0.159));
		x = sin(6.283185307179586 * (v5pc + oa * ia * 0.159));
	} else if (al == 1) {
		ob = sin(6.283185307179586 * (v5pb + v5fbv * fbk));
		v5fbv = ob;
		oa = sin(6.283185307179586 * v5pa);
		x = sin(6.283185307179586 * (v5pc + (oa * ia + ob * idxb) * 0.159));
	} else if (al == 2) {
		oa = sin(6.283185307179586 * (v5pa + v5fbv * fbk));
		v5fbv = oa;
		ob = sin(6.283185307179586 * v5pb);
		x = sin(6.283185307179586 * (v5pc + oa * ia * 0.159)) * (1 - bmix * 0.5) + ob * bmix * 0.7;
	} else {
		oa = sin(6.283185307179586 * (v5pa + v5fbv * fbk));
		v5fbv = oa;
		ob = sin(6.283185307179586 * (v5pb + oa * idxb * 0.159));
		x = sin(6.283185307179586 * (v5pc + oa * ia * 0.159)) * (1 - bmix * 0.5) + ob * bmix * 0.7;
	}
	x = x + sin(6.283185307179586 * v5ps) * psub * 0.8 + noise() * chuff * (0.4 + v5fe);
	
	fc = cut * exp2(keyt * log2(f / 261.63) + penv * 4 * fe + lfc);
	fc = clamp(fc, 30, sr * 0.42);
	gg = tan(3.141592653589793 * fc / sr);
	if (ft == 1) {
		
		x = tanh(x * 1.3) / 1.3;
		hp1 = (x - (2 * 0.7071) * v5s1 - gg * v5s1 - v5s2) / (1 + 2 * 0.7071 * gg + gg * gg);
		bp1 = gg * hp1 + v5s1;
		lp1 = gg * bp1 + v5s2;
		v5s1 = gg * hp1 + bp1;
		v5s2 = gg * bp1 + lp1;
		hp2 = (lp1 - kq * v5s3 - gg * v5s3 - v5s4) / (1 + kq * gg + gg * gg);
		bp2 = gg * hp2 + v5s3;
		lp2 = gg * bp2 + v5s4;
		v5s3 = gg * hp2 + bp2;
		v5s4 = gg * bp2 + lp2;
		y = lp2 * (1 + rq * 0.6);
	} else {
		
		hp = (x - kq * v5s1 - gg * v5s1 - v5s2) / (1 + kq * gg + gg * gg);
		bp = gg * hp + v5s1;
		lp = gg * bp + v5s2;
		v5s1 = gg * hp + bp;
		v5s2 = gg * bp + lp;
		y = ft == 0 ? lp : (ft == 2 ? bp * (1 + rq) : hp);
		y = tanh(y * 1.2) / 1.2;
	}
	y = y * v5ae * vl;
	act = act + v5ae;
	pn = 0.2 * pspread;
	dl = dl + y * sqrt(0.5 - pn * 0.5);
	dr = dr + y * sqrt(0.5 + pn * 0.5);
}
if (newest == 5) {
	fenv = v5fe;
}



eqm = pdubeq;
ghp = 1 - exp(-6.283185307179586 * 100 / sr);
e1 = e1 + ghp * (dl - e1);
e2 = e2 + ghp * (dr - e2);
hl = dl - e1;
hr = dr - e2;
gmid = 1 - exp(-6.283185307179586 * 600 / sr);
glo = 1 - exp(-6.283185307179586 * 300 / sr);
e3 = e3 + gmid * (hl - e3);
e4 = e4 + gmid * (hr - e4);
e5 = e5 + glo * (e3 - e5);
e6 = e6 + glo * (e4 - e6);

ql = hl - (e3 - e5) * 0.55 * 1.6;
qr = hr - (e4 - e6) * 0.55 * 1.6;
gl45 = 1 - exp(-6.283185307179586 * 4500 / sr);
e7 = e7 + gl45 * (ql - e7);
e8 = e8 + gl45 * (qr - e8);
ql = e7;
qr = e8;
ol = dl + (ql - dl) * eqm;
orr = dr + (qr - dr) * eqm;
out1 = ol * pvol * 0.20000;
out2 = orr * pvol * 0.20000;
out3 = fenv;
out4 = lv;
out5 = act;
 return [out1, out2, out3, out4, out5]; }
  function set(name, v) { switch (name) { case 'palgo': palgo = v; break; case 'prc': prc = v; break; case 'pra': pra = v; break; case 'prb': prb = v; break; case 'pia': pia = v; break; case 'pib': pib = v; break; case 'pfb': pfb = v; break; case 'pbmix': pbmix = v; break; case 'pchuff': pchuff = v; break; case 'psub': psub = v; break; case 'pdrift': pdrift = v; break; case 'pdetune': pdetune = v; break; case 'pftype': pftype = v; break; case 'pcut': pcut = v; break; case 'preso': preso = v; break; case 'pkey': pkey = v; break; case 'penv': penv = v; break; case 'pfa': pfa = v; break; case 'pfd': pfd = v; break; case 'pfs': pfs = v; break; case 'pfr': pfr = v; break; case 'ppoly': ppoly = v; break; case 'paa': paa = v; break; case 'pad': pad = v; break; case 'pas': pas = v; break; case 'par': par = v; break; case 'pvel': pvel = v; break; case 'plrate': plrate = v; break; case 'plshape': plshape = v; break; case 'pl2cut': pl2cut = v; break; case 'pl2idx': pl2idx = v; break; case 'pl2pitch': pl2pitch = v; break; case 'pspread': pspread = v; break; case 'pdubeq': pdubeq = v; break; case 'pvol': pvol = v; break; case 'v0f': v0f = v; break; case 'v0gate': v0gate = v; break; case 'v1f': v1f = v; break; case 'v1gate': v1gate = v; break; case 'v2f': v2f = v; break; case 'v2gate': v2gate = v; break; case 'v3f': v3f = v; break; case 'v3gate': v3gate = v; break; case 'v4f': v4f = v; break; case 'v4gate': v4gate = v; break; case 'v5f': v5f = v; break; case 'v5gate': v5gate = v; break; default: throw new Error("no param " + name); } }
  return { tick: tick, set: set };
}
window.makeVaultVoices = makeVaultVoices;
})();
