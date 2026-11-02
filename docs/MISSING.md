# Missing Material

Everything still needed before a stranger could reproduce this joint from the
repository alone. Ordered by how badly it blocks progress.

The original material set was: one firmware `.ino`, one Gerber zip, three
short `.docx` notes, 13 SolidWorks files, and 15 STL exports. That is a good
start — but several load-bearing pieces did not come with it.

---

## P0 — Blocks building anything

### 1. `config.h`

**The firmware does not compile without it.** `openloop.ino` includes it and
it was not in the released set.

[`../firmware/openloop/config.h.example`](../firmware/openloop/config.h.example)
reconstructs all 17 macros from comments inside the `.ino`, but six values
could not be recovered and are placeholders:

| Macro | Placeholder | Needed |
|---|---|---|
| `MOTOR_POLE_PAIRS` | `7` | the real pole-pair count of your 1503 |
| `OPENLOOP_DIRECTION` | `1` | `+1` or `-1`, whichever you ran |
| `VELOCITY_LIMIT` | `20.0f` | rad/s |
| `VOLTAGE_RAMP_MS` | `500` | ms |
| `ARRIVE_TOL_RAD` | `0.05f` | rad |
| `HOLD_VOLTAGE_RATIO` | `0.40f` | fraction |

**Ask:** send the original `config.h`. It is one small file and it unblocks
everything.

### 2. Schematic, or at minimum the two 6-pin header pinouts

The Gerbers give component *positions* but no *identities*. `U1` (26-pin),
`U2` (8-pin), `U3` (3-pin), `U4` (3-pin) are all unknown, and the pinout of
the two 6-pin headers (`A-P-3-G`, `G-A-B-C`) is unknown. Without that, nobody
can populate the board.

**Ask:** export the EasyEDA Pro schematic to PDF and commit it under
`hardware/pcb/source/`. Or photograph the assembled board with the headers
legible.

### 3. Which physical pin is motor phase A / B / C

The firmware drives A=GPIO10, B=GPIO3, C=GPIO2. The board has `PAD1`,
`PAD2`, `PAD3` at 120°. Nothing connects the two. Get this wrong and the motor
cogs or runs backwards — and in open loop there is no feedback to reveal the
error, only a hot motor.

**Ask:** a mapping table `PAD1 → phase ? → GPIO ?`, plus which stator wire
goes to which pad.

---

## P1 — Blocks trusting what's here

### 4. The closed-loop firmware and board

The Gerber zip is named **`Gerber_1代adc闭环`** — "generation 1, **ADC closed
loop**". But the only firmware released is **`开环代码.ino`** — "**open
loop**". So the PCB appears to be designed for closed-loop operation while the
shipped firmware deliberately does not use feedback.

**Ask:** is there a closed-loop firmware? Is there a gen-2 board? The current
repo can only honestly claim to be an open-loop driver on a closed-loop-named
board, which will confuse every reader.

### 5. BOM values

`R1`–`R4` and `C1`–`C9` have no values. `U1`/`U3`/`U4` have no part numbers.
`H1`/`H2` and the 6-pin headers have no pitch. The analog angle sensor has no
part number at all — the firmware implies something like an AS5600 in
analog-out mode or an MT6701, but that is inference.

The MP6540GU requirement *is* documented, which is good. It is the only
component in the whole project that is.

### 6. Kinematic description (the ratio itself is now known)

~~Nothing states the gear ratio.~~ **Resolved from the author's own video:**
the middle intro video is titled *"Foc 单级十倍减速"* — **single stage, 10:1**.
That number is now in both READMEs.

What is still missing is the *explanation*. What can be measured:

- internal ring gear, module 0.6, **27T**
- external ring gear, module 0.6, **30T**
- a flipped external ring gear variant (8.5 mm thick vs 5.75 mm, 28 128
  triangles vs 4 218 — a genuinely different part, not a mirror)
- a metal-print external ring gear variant
- 4 × 6 mm pins at ≈5.5 mm radius, 90° apart (`crank_output`)
- 4 × 31.8 mm rods at ≈11.5 mm radius, 90° apart (`part1_rod`)
- a part the author named `神秘中间传递` — "mystery intermediate transfer"

A 27T internal ring and a 30T external ring differ by 3 teeth and **cannot
mesh with each other directly**, so there must be an intermediate stage. But
the scheme (planetary? cycloidal? wobble/scotch-yoke?) and the resulting ratio
are not recoverable from the files.

**Ask:** one paragraph describing how the stages connect. The ratio (10:1,
single stage) is settled; the *mechanism* is not.

### 7. Print settings

No material, layer height, nozzle, infill, wall count, support or orientation
data for any part. Module 0.6 gear teeth are at the edge of what FDM can do
with a 0.4 mm nozzle, so this is not a detail — it is the difference between a
working gearbox and a box of failed prints.

**Ask:** settings per part, plus which printer. And whether the external ring
gear is printed in plastic or metal.

### 8. Still photos (video is now covered)

~~`assets/` is empty.~~ **Partially resolved.** Three intro videos are now
linked from both READMEs via the author's Baidu NetDisk share, with poster
frames committed under `assets/video/`. The third video shows the assembled
unit in hand, which covers the "does it exist" question.

What is still absent is any *still* image of the internals:

**Ask:**
- the bare PCB, top and bottom — the single most useful photo for anyone
  trying to populate the board, since there is no schematic
- the PCB installed inside the motor bell, showing the radial phase pads
- the printed gear train laid out before assembly
- a labelled photo of the two 6-pin headers

### 9. Cost breakdown

The project is named "$1.5" after the author's *"10元 1503 最小 FOC"* board
(10 CNY ≈ $1.40). But there is no table showing what the *joint* costs.

**Ask:** confirm whether "$1.5" means the bare PCB or something broader, and
supply prices for the motor, MP6540GU, ESP32-C3, sensor, bearings, rod and
filament.

---

## P2 — Nice to have

### 10. STL exports for three CAD-only parts

| CAD file | Why it matters |
|---|---|
| `universal_adapter.SLDPRT` | looks like the output interface — without it you cannot mount anything to the joint |
| `universal_adapter_extended.SLDPRT` | longer variant |
| `external_ring_gear_metal_print_m0.6_30T.SLDPRT` | the durable version of a wear part |

### 11. Bearing quantities and locations

Sizes are documented (7 × 11 × 3 mm and 5 × 8 × 2.5 mm) but not how many, or
which part each one seats in.

### 12. Confirm the 1.5 mm rod parts

`crank_output` (4 × 6 mm) and `part1_rod` (4 × 31.8 mm) are in the "for
printing" folder but measure 1.4 mm across — 1.5 mm rod modelled with 8
facets. They are almost certainly cut steel rod, not printed. The consumables
note listing "1.5 mm rod" supports this.

**Ask:** confirm, and confirm whether 31.8 mm is a cut length or just the
stock length drawn in CAD. At 31.8 mm these rods span nearly the whole 41 mm
assembly stack.

### 13. STL normalisation

The STL exports came out of the assembly, so each carries its placement
transform and does not sit at the origin. Slicers usually auto-centre, but
`mechanical/stl/print/` could be regenerated at the origin with a flat
orientation chosen per part, which would print more predictably.

**Ask:** want this done? Say so and it will be — it is a mechanical
transform, fully reversible, and the originals are preserved in
`mechanical/stl/as-exported/`.

### 14. Toolchain versions

Arduino ESP32 core version, SimpleFOC version (2.3.x inferred from the API
surface), and the exact ESP32-C3 module variant.

### 15. Performance numbers

Torque, no-load speed, stall current, backlash, and duty cycle. Nobody can
decide whether to use this joint in a robot arm without them.

### 16. The GPIO7 story

The source says phase B was moved off GPIO7 because "that pin has a hardware
problem". What happened? Over-current, reversed polarity, a solder bridge? If
it is a repeatable failure mode, it belongs in the assembly guide as a
warning.

### 17. Resolve the voltage-ceiling contradiction

The original `.ino` states the output ceiling three different ways: **15 %**
(0.75 V), **10 %**, and **0.5 V**. Since current sensing was deliberately
removed, this ceiling is the *only* protection the motor has. It should be one
number, stated once.

### 18. Purchase links that will rot

The consumables note points at two Pinduoduo mobile links. They need the PDD
app and a Chinese phone number, and links of that kind expire. The standard
bearing designations (MR117 / 6701, MR85) are documented in
[BOM.md](BOM.md) as the international-friendly substitute, but the rod and
motor still have no durable sourcing info.

### 19. License and attribution

The repo currently ships **MIT**, chosen as a safe default and **not
confirmed**. For hardware, CERN-OHL-S or CERN-OHL-W would cover the PCB and
mechanical design explicitly rather than treating everything as software.

Also needed: the name or handle to credit, and the copyright line for
`LICENSE`. The repo is staged under the git identity
`yzytz17 <lsb6699@qq.com>`.

---

## Quick checklist

If you only send five things, send these:

- [ ] **`config.h`** (the real one)
- [ ] **schematic PDF**, or a labelled photo of the two 6-pin headers
- [ ] **PAD1/2/3 → motor phase → GPIO mapping**
- [ ] **one paragraph on how the gear stages connect** (the 10:1 ratio is
      already known from your video; the mechanism is not)
- [ ] **still photos** of the bare PCB and the gear train (video is covered)

Those five turn this from "an archive of files" into "a project someone can
actually build".
