from dubemu import *
def run(label, blocks=2000, period=500, **kw):
    d = DubFw('../build_dub/plaits.elf', '../build_dub/plaits.bin', True)
    d.patch(kw.pop('h', 0.5), kw.pop('t', 0.5), kw.pop('m', 0.5))
    kw.setdefault('density', 16); d.set(**kw)
    cs = []
    for b in range(blocks):
        d.mods(trigger=1.0 if (b % period) < 8 else 0.0, trigger_patched=True)
        cs.append(d.block(count=True)[2])
    body = sorted(cs[5:])
    print("%-36s avg %5d  p99 %5d  max %5d (first block %d)" % (label, sum(body) // len(body), body[int(len(body) * .99)], body[-1], cs[0]))
for m in range(8):
    run('model %d, envelope 0.8' % (m + 1), chord_model=m, envelope=0.8)
run('model 5, 1/32T, envelope max', chord_model=4, envelope=1.0, speed=7)
