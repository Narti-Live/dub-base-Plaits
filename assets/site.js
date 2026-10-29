// Hero visual: chord stabs as faded horizontal smears with dub-delay echoes,
// on the off-beats of a 120 BPM 16th grid. Greyscale only.
(function () {
  var cv = document.getElementById('stabs');
  if (!cv || !cv.getContext) return;
  var ctx = cv.getContext('2d');
  var reduce = window.matchMedia && window.matchMedia('(prefers-reduced-motion: reduce)').matches;
  var W = 0, H = 0, scale = 0.5;           // render at half resolution: softer, cheaper

  // a soft smear sprite (no ctx.filter needed, works in every browser)
  var sprite = document.createElement('canvas');
  sprite.width = 512; sprite.height = 64;
  (function () {
    var s = sprite.getContext('2d');
    for (var i = 0; i < 24; i++) {
      var t = i / 23, h = 64 * (1 - t * 0.85), y = (64 - h) / 2;
      var g = s.createLinearGradient(0, 0, 512, 0);
      g.addColorStop(0, 'rgba(200,214,232,0)');
      g.addColorStop(0.12 + 0.1 * Math.random(), 'rgba(200,214,232,0.05)');
      g.addColorStop(0.55, 'rgba(200,214,232,0.06)');
      g.addColorStop(1, 'rgba(200,214,232,0)');
      s.fillStyle = g;
      s.fillRect(0, y, 512, h);
    }
    // streaks, like a worn print
    for (var k = 0; k < 40; k++) {
      s.fillStyle = 'rgba(4,7,12,' + (0.05 + Math.random() * 0.12) + ')';
      s.fillRect(Math.random() * 512, Math.random() * 64, 30 + Math.random() * 200, 1 + Math.random() * 2);
    }
  })();

  function resize() {
    var r = cv.getBoundingClientRect();
    W = Math.max(1, Math.round(r.width * scale));
    H = Math.max(1, Math.round(r.height * scale));
    cv.width = W; cv.height = H;
  }
  window.addEventListener('resize', resize);
  resize();

  var stabs = [];
  var stepMs = 60000 / 120 / 4, next = 0, step = 0;
  var pattern = [0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0];
  var chordY = 0.42;

  function stab(t, accent) {
    if (step % 64 === 0) chordY = 0.25 + Math.random() * 0.4;   // new chord every 4 bars
    var y = (chordY + (Math.random() - 0.5) * 0.06) * H;
    var x = W * (0.05 + Math.random() * 0.35), w = W * (0.35 + Math.random() * 0.35);
    var h = H * (0.025 + Math.random() * 0.05) * (accent ? 1.4 : 1);
    for (var e = 0; e < 6; e++) {                                   // the echoes
      stabs.push({ t: t + e * stepMs * 3, x: x + e * W * 0.045, y: y + e * H * 0.012, w: w * (1 + e * 0.12),
                   h: h * (1 + e * 0.25), a: (accent ? 0.95 : 0.7) * Math.pow(0.55, e), life: 2600 + e * 500 });
    }
  }

  function draw(now) {
    ctx.globalCompositeOperation = 'source-over';
    ctx.fillStyle = 'rgba(4,7,12,0.10)';                          // slow fade: trails
    ctx.fillRect(0, 0, W, H);
    var g = ctx.createRadialGradient(W * 0.7, H * 0.35, 0, W * 0.7, H * 0.35, Math.max(W, H) * 0.7);
    g.addColorStop(0, 'rgba(22,38,66,0.06)'); g.addColorStop(1, 'rgba(4,7,12,0)');
    ctx.fillStyle = g; ctx.fillRect(0, 0, W, H);
    ctx.globalCompositeOperation = 'source-over';
    for (var i = stabs.length - 1; i >= 0; i--) {
      var s = stabs[i], age = now - s.t;
      if (age < 0) continue;
      if (age > s.life) { stabs.splice(i, 1); continue; }
      var env = age < 40 ? age / 40 : Math.exp(-(age - 40) / (s.life * 0.32));
      ctx.globalAlpha = s.a * env * 0.16;
      ctx.drawImage(sprite, s.x + age * 0.012 * W / 100, s.y - s.h / 2, s.w, s.h);
    }
    ctx.globalAlpha = 1;
  }

  function frame(now) {
    if (!next) next = now;
    while (now >= next) {
      var p = step % 16;
      if (pattern[p] || (Math.random() < 0.08 && p % 2 === 0)) stab(next, p === 2 || p === 10);
      step++; next += stepMs;
    }
    draw(now);
    requestAnimationFrame(frame);
  }
  if (reduce) {
    var t0 = 1000; for (var s = 0; s < 6; s++) stab(t0 - s * 400, s === 0);
    for (var f = 0; f < 40; f++) draw(t0 + f * 20);
  } else {
    requestAnimationFrame(frame);
  }
})();

// Title: slow wave (animated turbulence) and an occasional glitch.
(function () {
  var reduce = window.matchMedia && window.matchMedia('(prefers-reduced-motion: reduce)').matches;
  var t = document.querySelector('.title'), n = document.getElementById('waveNoise');
  if (!t || reduce) return;
  var last = 0;
  function wave(now) {
    if (now - last > 66) {                     // ~15 fps is plenty for a slow wave
      last = now;
      var s = now / 1000;
      var fx = 0.0025 + 0.0010 * Math.sin(s * 0.09), fy = 0.008 + 0.004 * Math.sin(s * 0.06 + 1.3);
      if (n) n.setAttribute('baseFrequency', fx.toFixed(5) + ' ' + fy.toFixed(5));
    }
    requestAnimationFrame(wave);
  }
  requestAnimationFrame(wave);
  function glitch() {
    t.classList.add('glitch');
    setTimeout(function () { t.classList.remove('glitch'); }, 110 + Math.random() * 160);
    if (Math.random() < 0.4) setTimeout(function () {
      t.classList.add('glitch'); setTimeout(function () { t.classList.remove('glitch'); }, 70);
    }, 260);
    setTimeout(glitch, 5000 + Math.random() * 9000);
  }
  setTimeout(glitch, 3000);
})();

// Flasher: plays the firmware WAV untouched and shows progress + a level guide.
(function () {
  var a = document.getElementById('flashAudio');
  if (!a) return;
  var play = document.getElementById('flashPlay'), stop = document.getElementById('flashStop');
  var fill = document.getElementById('flashFill'), time = document.getElementById('flashTime');
  var status = document.getElementById('flashStatus');
  var leds = document.querySelectorAll('.meter .m');       // red, yellow, green, green (top to bottom)
  var timer = null;
  function fmt(x) { x = Math.max(0, Math.floor(x)); return Math.floor(x / 60) + ':' + ('0' + x % 60).slice(-2); }
  function blink() {                                        // show the target: both greens, yellow flickers
    leds[2].classList.add('on'); leds[3].classList.add('on');
    leds[1].classList.toggle('on', Math.random() < 0.15);
    leds[0].classList.remove('on');
  }
  function clearLeds() { for (var i = 0; i < leds.length; i++) leds[i].classList.remove('on'); }
  var radios = document.querySelectorAll('input[name=fw]');
  function onPick(e) {
    if (!a.paused) return;
    var r = e.target; fill.style.width = '0'; time.textContent = '0:00';
    if (r.dataset.beta) {
      play.disabled = true; status.textContent = 'Decrypting the beta firmware…';
      r._get().then(function (url) {
        if (r.checked) { a.src = url; play.disabled = false; status.textContent = 'Ready: ' + r.dataset.label + ' (beta).'; }
      }, function () { status.textContent = 'Could not decrypt the beta firmware.'; });
    } else { a.src = r.value; play.disabled = false; status.textContent = 'Ready: ' + (r.dataset.label || 'C minor, release v1.1') + '.'; }
  }
  for (var k = 0; k < radios.length; k++) radios[k].addEventListener('change', onPick);
  window.dubAddFirmware = function (f, get) {
    var box = document.getElementById('variants');
    var l = document.createElement('label');
    l.innerHTML = '<input type="radio" name="fw"><b></b><span></span>';
    var inp = l.querySelector('input'); inp.value = 'beta:' + f.file; inp.dataset.beta = '1'; inp._get = get;
    inp.dataset.label = f.label.replace('Firmware · ', '');
    l.querySelector('b').textContent = f.short || (f.variant === 'gmin' ? 'G minor β' : 'G minor+ β');
    l.querySelector('span').textContent = f.sub || (f.variant === 'gmin' ? 'DEEP dub chords' : 'all chords');
    box.appendChild(l); box.classList.remove('one');
    inp.addEventListener('change', onPick);
    radios = document.querySelectorAll('input[name=fw]');
  };
  play.addEventListener('click', function () {
    for (var k = 0; k < radios.length; k++) { radios[k].disabled = true; if (radios[k].checked && !radios[k].dataset.beta && a.getAttribute('src') !== radios[k].value) a.src = radios[k].value; }
    a.currentTime = 0;
    status.textContent = 'Loading the firmware…';
    var p = a.play();
    if (p && p.catch) p.catch(function () { status.textContent = 'The browser blocked playback. Press Play again.'; });
  });
  stop.addEventListener('click', function () {
    a.pause(); a.currentTime = 0; fill.style.width = '0'; time.textContent = '0:00';
    play.disabled = false; stop.disabled = true; clearLeds(); clearInterval(timer);
    for (var k = 0; k < radios.length; k++) radios[k].disabled = false;
    status.textContent = 'Stopped. On the module: press LEFT, then play again from the start.';
  });
  a.addEventListener('playing', function () {
    play.disabled = true; stop.disabled = false;
    status.textContent = 'Playing — don’t touch the computer. Check the module’s meter: both green LEDs lit.';
    clearInterval(timer); timer = setInterval(blink, 120);
  });
  a.addEventListener('timeupdate', function () {
    var d = a.duration || 156;
    fill.style.width = (100 * a.currentTime / d).toFixed(2) + '%';
    time.textContent = fmt(a.currentTime);
  });
  a.addEventListener('ended', function () {
    play.disabled = false; stop.disabled = true; clearInterval(timer); clearLeds();
    for (var k = 0; k < radios.length; k++) radios[k].disabled = false;
    status.textContent = 'Done. The module should restart into DUB STAB PLAITS. All LEDs blinking red instead: press LEFT, nudge FREQUENCY, play again.';
  });
  a.addEventListener('error', function () {
    status.textContent = 'Could not load the file. Download the .wav instead and play it with QuickTime or VLC.';
  });
})();

// Beta testing: files are AES-256-GCM encrypted. The key comes from the
// password through PBKDF2-SHA256 with 2,000,000 rounds (deliberately slow),
// is derived once per unlock and kept only in memory, never stored.
// After 2 wrong passwords the form locks for 2 minutes (doubling each time).
(function () {
  var form = document.getElementById('betaLock');
  if (!form || !window.crypto || !crypto.subtle) return;
  var pwEl = document.getElementById('betaPw'), go = document.getElementById('betaGo'), msg = document.getElementById('betaMsg');
  var wrap = document.getElementById('betaWrap'), open = document.getElementById('betaOpen');
  var list = document.getElementById('betaList'), note = document.getElementById('betaNote');
  var key = null, cache = {}, busy = false, timer = null;
  var LS = 'dubBetaGuard';
  function guard() { try { return JSON.parse(localStorage.getItem(LS)) || { fails: 0, locks: 0, until: 0 }; } catch (e) { return { fails: 0, locks: 0, until: 0 }; } }
  function save(g) { try { localStorage.setItem(LS, JSON.stringify(g)); } catch (e) {} }
  function header(b) {
    if (b[0] !== 68 || b[1] !== 66 || b[2] !== 50) throw new Error('format');      // "DB2"
    var it = b[3] | (b[4] << 8) | (b[5] << 16) | (b[6] << 24);
    return { iter: it >>> 0, salt: b.slice(7, 23), iv: b.slice(23, 35), data: b.slice(35) };
  }
  async function deriveKey(password, h) {
    var base = await crypto.subtle.importKey('raw', new TextEncoder().encode(password), 'PBKDF2', false, ['deriveKey']);
    return crypto.subtle.deriveKey({ name: 'PBKDF2', salt: h.salt, iterations: h.iter, hash: 'SHA-256' },
      base, { name: 'AES-GCM', length: 256 }, false, ['decrypt']);
  }
  async function decrypt(buf) {
    var h = header(new Uint8Array(buf));
    return crypto.subtle.decrypt({ name: 'AES-GCM', iv: h.iv }, key, h.data);
  }
  async function blobUrl(f) {
    if (cache[f.file]) return cache[f.file];
    var r = await fetch('beta/' + f.file);
    var plain = await decrypt(await r.arrayBuffer());
    var type = f.kind === 'wav' ? 'audio/wav' : (f.kind === 'apk' ? 'application/vnd.android.package-archive' : 'application/octet-stream');
    return (cache[f.file] = URL.createObjectURL(new Blob([plain], { type: type })));
  }
  function mb(n) { return n > 1e6 ? (n / 1e6).toFixed(1) + ' MB' : Math.round(n / 1e3) + ' KB'; }
  function lockedFor() { return Math.max(0, guard().until - Date.now()); }
  function tick() {
    var ms = lockedFor();
    if (ms > 0) {
      var s = Math.ceil(ms / 1000);
      pwEl.disabled = go.disabled = true;
      msg.textContent = 'Too many attempts — wait ' + Math.floor(s / 60) + ':' + ('0' + s % 60).slice(-2);
      clearTimeout(timer); timer = setTimeout(tick, 500);
    } else {
      pwEl.disabled = go.disabled = false;
      if (/Too many/.test(msg.textContent)) msg.textContent = '';
    }
  }
  function show(man) {
    wrap.classList.add('unlocked');
    form.hidden = true; open.hidden = false;
    note.textContent = man.note || '';
    man.files.forEach(function (f) {
      var a = document.createElement('a');
      a.href = '#'; a.innerHTML = '<b></b><span></span><code></code>';
      a.querySelector('b').textContent = f.name;
      a.querySelector('span').textContent = f.label + ' · ' + mb(f.size);
      a.querySelector('code').textContent = 'sha256 ' + f.sha256;
      a.addEventListener('click', async function (e) {
        e.preventDefault();
        var sp = a.querySelector('span'), old = sp.textContent;
        sp.textContent = 'Decrypting…';
        try {
          var url = await blobUrl(f), d = document.createElement('a');
          d.href = url; d.download = f.name; document.body.appendChild(d); d.click(); d.remove();
          sp.textContent = old;
        } catch (err) { sp.textContent = 'Could not decrypt — reload and try again.'; }
      });
      list.appendChild(a);
      if (f.kind === 'wav' && window.dubAddFirmware) window.dubAddFirmware(f, function () { return blobUrl(f); });
    });
  }
  form.addEventListener('submit', async function (e) {
    e.preventDefault();
    if (busy || lockedFor() > 0) { tick(); return; }
    var p = pwEl.value;
    if (!p) return;
    busy = true; go.disabled = true; msg.textContent = 'Checking…';
    try {
      var buf = await (await fetch('beta/m.bin')).arrayBuffer();
      key = await deriveKey(p, header(new Uint8Array(buf)));
      var man = JSON.parse(new TextDecoder().decode(await decrypt(buf)));
      pwEl.value = ''; msg.textContent = '';
      save({ fails: 0, locks: 0, until: 0 });
      show(man);
    } catch (err) {
      key = null;
      var g = guard(); g.fails++;
      if (g.fails >= 2) { g.until = Date.now() + 120000 * Math.pow(2, Math.min(g.locks, 6)); g.locks++; g.fails = 0; }
      save(g);
      pwEl.value = '';
      msg.textContent = g.until > Date.now() ? '' : 'Wrong password — 1 try left before a 2 minute wait';
      form.classList.remove('shake'); void form.offsetWidth; form.classList.add('shake');
      tick();
    }
    busy = false; if (lockedFor() === 0) go.disabled = false;
  });
  tick();
})();
