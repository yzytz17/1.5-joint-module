# Assembly Guide

> **This guide is a skeleton.** The released material contained no assembly
> instructions, no exploded view, no photos and no video. The sequence below
> is inferred from part geometry and the firmware's expectations — it is a
> place to hang the real instructions, not a substitute for them.
>
> Every step marked **TODO(author)** needs your input.

---

## Overview

```
   [motor_mount]──[1503 motor]──[gear train]──[output]
        │              │              │            │
     25.3 Ø         18.1 Ø        m0.6 gears   4 × 6 mm pins
      8 mm           11.2 mm      + 4 × 32 mm      on a ~5.5 mm
                                  rods on an        radius
                                  ~11.5 mm circle
```

The whole stack measures about **41 mm along the shaft axis**, from the front
of the motor mount (X ≈ 0.3 mm) to the end of the intermediate driver
(X ≈ 41 mm), all in the assembly's own coordinate frame.

Stack order, measured from the STL bounding boxes along the shaft axis:

| Position along axis | Part | Thickness |
|---|---|---|
| 0.3 → 8.3 mm | `motor_mount` | 8.0 mm |
| 8.3 → 19.5 mm | `motor_1503_reference` (the motor) | 11.2 mm |
| 19.5 → 29.0 mm | `shaft_rocker_arm` | 9.5 mm |
| 23.0 → 28.8 mm | `external_ring_gear_m0.6_30T` | 5.75 mm |
| 26.5 → 30.5 mm | `internal_ring_gear_m0.6_27T` | 4.0 mm |
| 27.0 → 33.0 mm | `crank_output` pins (×4) | 6.0 mm long |
| 30.5 → 41.0 mm | `mystery_intermediate_driver` | 10.5 mm |
| 5.3 → 37.1 mm | `part1_rod` (×4) | 31.8 mm long — spans nearly the whole stack |

Note that `external_ring_gear` and `internal_ring_gear` **overlap in the same
axial band** (23–30.5 mm), which is consistent with them meshing side by side
rather than stacked.

> **TODO(author):** confirm this stack order, and explain the role of each
> stage. In particular: the 27T internal ring and the 30T external ring differ
> by 3 teeth and cannot mesh directly, so what does the
> `mystery_intermediate_driver` actually do, and what is the **reduction
> ratio**?

---

## Step 1 — Order and populate the PCB

1. Upload `hardware/pcb/gerber_gen1_adc_closedloop_2026-10-21.zip` to your
   fab. **4 layers**, 0.305 mm min drill, outline ≈17.6 × 16.4 mm.
2. Order a **stencil**. Both top and bottom paste layers are present, and
   `U2` + `U4` are bottom-side — this is a two-sided reflow job.
3. Source the **MP6540GU** specifically. The author's note is emphatic that
   other MP6540 package variants will not work.
4. Reflow bottom side first, then top.

> **TODO(author):** supply the BOM values for `R1`–`R4` and `C1`–`C9`, the
> part numbers for `U1`/`U3`/`U4`, and the pinout of the two 6-pin headers.
> Without the schematic, populating this board requires reverse-engineering
> it — see [HARDWARE.md](HARDWARE.md).

---

## Step 2 — Print the gearbox

Files are in [`mechanical/stl/print/`](../mechanical/stl/print/).

Do **not** print these three — they are not printed parts:

- `motor_1503_reference.stl` — CAD model of the motor
- `crank_output.stl` — this is a **1.5 mm steel rod cut to 6 mm**
- `part1_rod.stl` — this is a **1.5 mm steel rod cut to ~31.8 mm**

> **TODO(author):** material (PLA / PETG / ABS / resin?), layer height,
> nozzle, infill, wall count, supports, and print orientation for each part.
> Module 0.6 teeth are at or below FDM resolution limits with a 0.4 mm
> nozzle, so this matters a lot. Did you print the gears flat or on edge?
> Did you anneal them?

> **TODO(author):** is the external ring gear printed in plastic or metal?
> There is a metal-print CAD variant with no STL export.

### Cut the rod

From 1.5 mm steel rod (music wire / silver steel):

- **4 × 6.0 mm** — the crank pins
- **4 × 31.8 mm** — the long rods

Deburr both ends. The pins are a press/sliding fit in printed holes, so a
bur will stop them seating.

---

## Step 3 — Press the bearings

| Bearing | Size | Qty | Location |
|---|---|---|---|
| MR117 / 6701 | 7 × 11 × 3 mm | **TODO** | **TODO** |
| MR85 | 5 × 8 × 2.5 mm | **TODO** | **TODO** |

> **TODO(author):** quantities and which part each bearing seats in.

---

## Step 4 — Mount the motor

The `motor_mount` is 25.26 mm Ø × 8 mm thick and the motor measures 18.1 mm Ø
× 11.2 mm. The mount is thicker in diameter than the motor but shorter than it
is long, so it is presumably a flange or cup the motor registers into.

> **TODO(author):** how is the motor retained — press fit, screws, adhesive?
> Are there mounting holes? Which face of the motor sits against the mount?

---

## Step 5 — Install the PCB inside the motor

The board's three phase pads (`PAD1`, `PAD2`, `PAD3`) are arranged **120°
apart on a 6.0 mm radius circle**, which strongly suggests the board sits
inside the motor bell with the phase wires fanning out radially to the stator
terminals.

> **TODO(author):** confirm the board mounts inside the bell. How is it
> retained? And critically — **which phase pad maps to which stator wire?**
> The firmware drives A=GPIO10, B=GPIO3, C=GPIO2; getting these wrong means
> the motor runs backwards or cogs, and in open loop with no feedback there is
> nothing to catch the mistake.

---

## Step 6 — Assemble the gear train

> **TODO(author):** full step-by-step with photos. Specifically:
> - Which gear is fixed to the housing and which rotates?
> - Where do the 4 × 6 mm crank pins go, and how are they retained?
> - The 4 × 31.8 mm rods span almost the entire 41 mm stack — are they tie
>   rods, output shafts, or axles? They sit 90° apart at ≈11.5 mm radius.
> - What is the output? `shaft_rocker_arm`, `universal_adapter`, or
>   `universal_adapter_extended`?
> - Is there backlash adjustment, or preload?

---

## Step 7 — Flash the firmware

```bash
cd firmware/openloop
cp config.h.example config.h
```

Edit `config.h`. **Six values are guesses and must be corrected:**

| Macro | Guess | Why it matters |
|---|---|---|
| `MOTOR_POLE_PAIRS` | 7 | wrong value = wrong electrical angle = motor cogs or stalls |
| `OPENLOOP_DIRECTION` | 1 | sign of rotation |
| `VELOCITY_LIMIT` | 20 rad/s | open-loop advance rate cap |
| `VOLTAGE_RAMP_MS` | 500 | soft-start duration |
| `ARRIVE_TOL_RAD` | 0.05 | when "arrived" triggers the voltage drop |
| `HOLD_VOLTAGE_RATIO` | 0.40 | holding voltage; too high and the motor cooks |

Then flash with Arduino IDE + ESP32-C3 board package + SimpleFOC 2.3.x.

> **⚠ GPIO0 is a strapping pin** on ESP32-C3 and is used here for the encoder
> ADC. If flashing fails, hold BOOT while resetting.

Full details: [FIRMWARE.md](FIRMWARE.md).

---

## Step 8 — First run

1. Power up. **The motor will not move.** This is correct — the firmware
   boots into encoder-only mode with the driver de-energised.
2. Open a serial monitor at **115200 baud**. You should see the banner, the
   parameter line, the wiring line, then repeating `[ENC] enc=...` lines.
3. Send `q` and check the printed pin map matches your physical wiring:
   `A=GPIO10 B=GPIO3 C=GPIO2 EN=GPIO8 encoder=GPIO0`.
4. Rotate the output by hand and confirm the `[ENC] enc=` value tracks 0–360°.
   If it runs backwards, set `ANGLE_MAP_INVERT` to `1` in `config.h`.
5. Send `a 90`. The motor should move its internal angle to 90° and then drop
   to holding voltage. Watch it — **it may get warm**.
6. Send `R2` for 2 rev/s, and `D` to de-energise again.

### Troubleshooting

| Symptom | Likely cause |
|---|---|
| Motor does not move at all | `EN` (GPIO8) not wired; or you have not sent a command — it is stationary by design |
| Motor vibrates / cogs without turning | Wrong `MOTOR_POLE_PAIRS`; or phase wires A/B/C swapped |
| Motor runs backwards | Set `OPENLOOP_DIRECTION` to `-1` |
| Runs then stalls and position is wrong | Normal for open loop. Reboot to zero the internal counter and re-send the target |
| Encoder reads 0 or 360 and never changes | Analog sensor not powered, or wrong ADC pin; GPIO0 is ADC1_CH0 |
| Motor gets very hot while stationary | `HOLD_VOLTAGE_RATIO` too high, or `ARRIVE_TOL_RAD` too small so it never enters hold |
| Board will not flash | GPIO0 strapping pin — hold BOOT while resetting |
| Nothing on serial | Wrong baud (must be 115200), or USB-CDC not enabled for the C3 board |

> **TODO(author):** add the failure modes you actually hit. The original
> source mentions GPIO7 being hardware-damaged — what caused that, and is it
> a warning others should know about (over-current, reversed polarity, a
> solder bridge)?

---

## Step 9 — Mount something to it

`universal_adapter.SLDPRT` and `universal_adapter_extended.SLDPRT` exist in
CAD but **have no STL export**.

> **TODO(author):** export them, and document the output interface — shaft
> diameter, bolt circle, and how much torque the joint can actually transmit
> before the printed gears strip.

---

## What this guide is missing

- Photos of every stage
- An exploded assembly view
- A video of the joint running
- Torque / speed / backlash measurements
- The reduction ratio
- Print settings
- Bearing quantities and locations
- Phase-wire mapping

See [MISSING.md](MISSING.md) for the consolidated list.
