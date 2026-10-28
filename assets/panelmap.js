/* DUB STAB PLAITS: the annotated panel, shared by the website and the manual.
   Numbered markers on the panel picture + a matching list. Hover, focus or tap
   either one and both light up. Coordinates are pixels in assets/panel.jpg (720 x 1240). */
(function () {
  var W = 720, H = 1240;
  // id, x, y, radius, badge dx, dy, group, control, function, detail
  var C = [
    ["bl", 308, 128, 36, -30, -32, "top", "Left button", "Prev model · NEW", "<b>Tap</b>: previous chord model. <b>Hold 1 s</b>: a NEW pattern (the LEDs pulse green). <b>Hold both buttons 2 s</b>: CV TUNE."],
    ["br", 416, 128, 36, 30, -32, "top", "Right button", "Next model · chords / notes", "<b>Tap</b>: next chord model. <b>Hold 1 s</b>: chords ↔ single notes for this model. On ACID: mono line ↔ acid chords."],
    ["leds", 360, 415, 26, 0, 245, "top", "8 LEDs", "Model + values", "The playing model lights <b>yellow</b> (chords) or <b>red</b> (single notes) and blinks on every step. Move a control and its value shows for a second. The red labels on the right are the SPEED values."],
    ["freq", 142, 262, 62, -46, -46, "top", "FREQUENCY", "Envelope", "Left: short dark pluck. Noon: dub stab. Right: long swell with a big filter sweep. <b>Hold LEFT + turn</b>: octave 1–5. <b>Hold RIGHT + turn</b>: resonance."],
    ["harm", 578, 262, 62, 46, -46, "top", "HARMONICS", "Model sound 1", "Changes with the model: detune, ratio, WARP, brightness, cutoff… See the chord models."],
    ["timb", 142, 545, 45, 36, -36, "top", "TIMBRE", "Model sound 2", "Changes with the model: pulse width, fold, FM index, feedback, accent…"],
    ["morph", 578, 545, 45, 36, -36, "top", "MORPH", "Model sound 3", "Changes with the model: noise chuff, feedback, SPACE, resonance…"],
    ["dens", 131, 825, 34, 30, -30, "seq", "TIMBRE small knob", "DENSITY", "How many of the 16 steps play. Left: a few off-beat stabs. Right: every step."],
    ["len", 361, 812, 34, 30, -30, "seq", "FM small knob", "LENGTH", "Pattern length, 1 to 16 steps."],
    ["speed", 590, 825, 34, 30, -30, "seq", "MORPH small knob", "SPEED", "Steps per clock: 1/2 · 1/4 · 1/8 · 1/8T · <b>1/16</b> (centre) · 1/16T · 1/32 · 1/32T."],
    ["jmodel", 85, 998, 34, 28, -28, "jacks", "MODEL jack", "Model CV", "Selects the chord model."],
    ["jtimb", 222, 998, 34, 28, -28, "jacks", "TIMBRE jack", "Timbre CV", "±5 V moves TIMBRE ±15 % around the knob."],
    ["jfm", 361, 998, 34, 28, -28, "jacks", "FM jack", "Not used", ""],
    ["jmorph", 498, 998, 34, 28, -28, "jacks", "MORPH jack", "Morph CV", "±5 V moves MORPH ±15 % around the knob."],
    ["jharm", 634, 998, 34, 28, -28, "jacks", "HARMO jack", "Harmonics CV", "±5 V moves HARMONICS ±15 % around the knob."],
    ["jtrig", 85, 1152, 34, 28, -28, "jacks", "TRIG jack", "Clock in", "16th-note clock. Start the clock and the pattern plays from step 1; stop it and it goes back to step 1. Nothing patched: 120 BPM."],
    ["jlevel", 222, 1152, 34, 28, -28, "jacks", "LEVEL jack", "VCA", "CV for the output level."],
    ["jvoct", 361, 1152, 34, 28, -28, "jacks", "V/OCT jack", "Transpose", "Moves the chord progression by scale degrees, always snapped to the key."],
    ["jout", 498, 1152, 34, 28, -28, "jacks", "OUT", "Chords", "The audio, dry: patch your delay and reverb after it."],
    ["jaux", 634, 1152, 34, 28, -28, "jacks", "AUX", "Root CV out", "The chord root as 1 V/oct (C3 = 0 V) for a second synth. Tune it once with CV TUNE."]
  ];
  var GROUPS = { top: "Buttons, LEDs, big knobs", seq: "Sequencer (small knobs)", jacks: "Jacks" };

  function el(tag, cls, html) { var e = document.createElement(tag); if (cls) e.className = cls; if (html != null) e.innerHTML = html; return e; }

  function mount(opt) {
    var fig = opt.figure, list = opt.list, base = opt.base || "";
    var wrap = el("div", "pm-wrap");
    var img = el("img"); img.src = base + "assets/panel.jpg?v=15"; img.alt = "The Plaits panel running DUB STAB PLAITS, with numbered controls";
    img.width = W; img.height = H; img.loading = "lazy";
    wrap.appendChild(img);
    var marks = {}, items = {};
    function on(id, v) {
      if (marks[id]) marks[id].classList.toggle("on", v);
      if (items[id]) items[id].classList.toggle("on", v);
    }
    C.forEach(function (c, i) {
      var n = i + 1;
      var ring = el("span", "pm-ring");
      ring.style.left = (c[1] / W * 100) + "%"; ring.style.top = (c[2] / H * 100) + "%";
      ring.style.width = (c[3] * 2 / W * 100) + "%"; ring.style.aspectRatio = "1";
      if (c[0] === "leds") { ring.style.height = (470 / H * 100) + "%"; ring.style.width = (64 / W * 100) + "%"; ring.style.aspectRatio = "auto"; ring.style.borderRadius = "40px"; }
      var b = el("button", "pm-badge", String(n));
      b.type = "button"; b.setAttribute("aria-label", n + ": " + c[7] + " — " + c[8]);
      b.style.left = ((c[1] + c[4]) / W * 100) + "%"; b.style.top = ((c[2] + c[5]) / H * 100) + "%";
      var tip = el("span", "pm-tip", c[8]); b.appendChild(tip);
      if (c[1] > W * 0.62) tip.classList.add("l");
      var m = el("span", "pm-mark"); m.appendChild(ring); m.appendChild(b);
      wrap.appendChild(m); marks[c[0]] = m;
      b.addEventListener("mouseenter", function () { on(c[0], true); });
      b.addEventListener("mouseleave", function () { on(c[0], false); });
      b.addEventListener("focus", function () { on(c[0], true); });
      b.addEventListener("blur", function () { on(c[0], false); });
      b.addEventListener("click", function () {
        var it = items[c[0]]; if (!it) return;
        if (opt.scrollList !== false) it.scrollIntoView({ block: "nearest", behavior: "smooth" });
        it.classList.add("flash"); setTimeout(function () { it.classList.remove("flash"); }, 900);
      });
    });
    fig.insertBefore(wrap, fig.firstChild);

    if (list) {
      var g = null, ol = null;
      C.forEach(function (c, i) {
        if (c[6] !== g) {
          g = c[6];
          list.appendChild(el("h3", "pm-group", GROUPS[g]));
          ol = el("ol", "pm-list"); ol.start = i + 1; list.appendChild(ol);
        }
        var li = el("li", "pm-item" + (c[8] === "Not used" ? " off" : ""));
        li.innerHTML = '<span class="pm-n">' + (i + 1) + '</span><span class="pm-t"><span class="pm-ctl">' + c[7] +
          '</span><span class="pm-fn">' + c[8] + '</span>' + (c[9] && !opt.compact ? '<span class="pm-p">' + c[9] + '</span>' : "") + '</span>';
        li.tabIndex = 0;
        li.addEventListener("mouseenter", function () { on(c[0], true); });
        li.addEventListener("mouseleave", function () { on(c[0], false); });
        li.addEventListener("focus", function () { on(c[0], true); });
        li.addEventListener("blur", function () { on(c[0], false); });
        ol.appendChild(li); items[c[0]] = li;
      });
    }
  }
  window.PanelMap = { mount: mount, controls: C };
})();
