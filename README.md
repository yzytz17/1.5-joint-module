# $1.5 Joint Module

**English** | [中文](README.zh-CN.md)

> **Language policy.** English is the public face of this repository. Chinese
> is the author's working language and is kept in full at
> [`README.zh-CN.md`](README.zh-CN.md) and in
> [`firmware/openloop-zh/`](firmware/openloop-zh/).
>
> Where a load-bearing statement exists in both languages, **the Chinese
> original is authoritative** — English is a translation and can lose emphasis
> or nuance. Part numbers, pin numbers and measured dimensions are
> language-neutral and identical in both.

> An ultra-low-cost robotic joint module: a ~$1.5 four-layer FOC driver board
> that fits inside a 1503 brushless gimbal motor, driving a fully 3D-printed
> high-ratio precision gearbox. **Cheap actuation, precise motion.**

A tiny robotic joint built from three cheap things: a **1503 brushless gimbal
motor**, a **~$1.5 four-layer FOC driver board** that fits inside the motor
bell, and a **fully 3D-printed reduction gearbox**.

The point is the cost floor. A four-layer driver board at prototype pricing is
about **$1.5**, and the entire gear train prints on a machine that already
sits on your desk. That combination is what makes inexpensive actuation
practical at scale — a six-degree-of-freedom arm, a quadruped, a pan-tilt
head — without giving up reduction ratio or positioning resolution.

Everything here is open: PCB Gerbers, drive firmware, SolidWorks CAD and
print-ready STL.

> **Status: work in progress.** This repository was assembled from the
> author's original material set and reorganised into English. Several
> items are still missing — every gap is marked `TODO(author)` and also
> listed in [MISSING.md](docs/MISSING.md). Please don't treat placeholder
> values as verified.

---

## What is in the box

| Layer | Contents | Path |
|---|---|---|
| Electrical | 4-layer driver PCB Gerbers + manufacturing zip | [`hardware/pcb/`](hardware/pcb/) |
| Firmware | ESP32-C3 + SimpleFOC open-loop drive (English + original Chinese) | [`firmware/`](firmware/) |
| Mechanical | SolidWorks parts and assembly | [`mechanical/cad-solidworks/`](mechanical/cad-solidworks/) |
| Mechanical | Print-ready STL, de-duplicated and measured | [`mechanical/stl/`](mechanical/stl/) |
| Docs | Firmware guide, hardware guide, BOM, assembly | [`docs/`](docs/) |

---

## How it works

### Electronics

The driver is a **circular-cornered 4-layer board, ≈17.6 × 16.4 mm**, small
enough to sit inside the 1503 motor's bell. Its three motor-phase pads are
placed **120° apart on a 6.0 mm radius circle**, so the phase wires fan out
radially to the stator instead of routing off the side of the board.

Board facts read directly out of the released Gerbers:

- Designed in **EasyEDA Pro v4.1.60**, Gerbers generated **2026-10-21**
- **4 copper layers** (top, bottom, 2 inner)
- Outline ≈ **17.59 × 16.38 mm**
- Drills: **14 × 0.305 mm** plated through-holes + **3 × 2.0 mm**, plus a
  separate 0.305 mm via drill file
- **28 placements** from the flying-probe data: `U1` (26-pin, top), `U2`
  (8-pin, bottom), `U3` (3-pin, top), `U4` (3-pin, bottom), `H1`/`H2`
  (3-pin connectors), two 6-pin headers, `LED1`, `R1`–`R4`, `C1`–`C9`,
  `PAD1`–`PAD3` (motor phases)

> ## ⚠️ The driver chip must be **MP6540GU** — *not* MP6540**H**GU
>
> This is the single most important purchasing instruction in the whole
> project, and it is easy to get wrong, because **the two parts share the same
> QFN-26 (5 × 5 mm) footprint**. A wrongly-bought MP6540HGU will still solder
> down perfectly and still not be what this board was designed around.
>
> | | **MP6540GU** (this board) | MP6540HGU (wrong for this board) |
> |---|---|---|
> | Family | MP6540 / MP6540A | MP6540H / MP6540HA |
> | Supply voltage | 5.5 – **35 V** | 5.5 – **50 V** |
> | Continuous current | **3 A** | **5 A** |
> | Peak current | **10 A** (1 s) | **6 A** |
> | Package | QFN-26 (5 × 5) | QFN-26 (5 × 5) — identical |
>
> **Why the non-H part, in the author's own reasoning:**
>
> 1. **It is cheaper**, and cost is the entire point of this project.
> 2. The H version's extra current headroom **earns nothing in a FOC drive
>    like this one** — the firmware is voltage-limited, so the extra amperes
>    are never used.
> 3. **This motor's KV is high.** With a high-KV winding, excess available
>    current does not produce more useful torque; it produces heat and
>    destabilises the whole motor. The lower-current part is the *correct*
>    part, not a compromise.
>
> Full detail: [`docs/HARDWARE.md`](docs/HARDWARE.md#the-driver-chip-mp6540gu-not-mp6540hgu).

The full editable project lives on the LCEDA/OSHWHub platform:
[10元 1503 最小 FOC — OSHWHub](https://oshwhub.com/team_dvojhors/project_ceehsgzu)

### Firmware

**ESP32-C3 + SimpleFOC**, running **pure open loop**. This is deliberate
and it is the most important thing to understand before you flash it:

- The electrical angle is advanced by **internal integration only**. There
  is no sensor in the control loop, no `initFOC()`, no alignment routine,
  and no PID.
- **The motor stays still at power-up.** It does not spin, and it does not
  hunt for a zero position. Nothing moves until you send a serial command.
- The analog encoder is **display-only**. Unplug it and the motor still
  follows commands. It is only read so you can see a number on the serial
  monitor.
- Because position is an internal counter, **a stall or lost step makes the
  angle wrong**. Reboot to zero it, then re-send your target.

Pin map (ESP32-C3):

| Signal | GPIO | Note |
|---|---|---|
| Phase A (`INHA`) | 10 | |
| Phase B (`INHB`) | **3** | originally GPIO7 — that pin is faulty on this board, so it moved |
| Phase C (`INHC`) | 2 | |
| Enable (`EN`) | 8 | |
| Encoder (ADC) | 0 | analog voltage, 0–3.3 V ↔ 0–360° |

Output is capped at `POWER_SUPPLY_VOLTAGE × VOLTAGE_LIMIT_RATIO` =
**5 V × 15 % = 0.75 V**, which also protects the motor now that current
sensing has been removed.

Three serial modes at **115200 baud**:

| Command | Mode | Prefix | Behaviour |
|---|---|---|---|
| `90` or `a 90` | Position | `[POS]` | move internal angle to 90°, drop voltage on arrival |
| `R2` | Velocity | `[VEL]` | spin at 2 rev/s |
| `D` | Encoder | `[ENC]` | motor de-energised, read encoder only (default at boot) |
| `o` / `q` | — | current mode | status / parameter query |

Full details in [`docs/FIRMWARE.md`](docs/FIRMWARE.md).

### Gearbox

The reduction is printed. Gears are **module 0.6**: a **27-tooth internal
ring** and a **30-tooth external ring**, plus a flipped variant of the
external ring and a **metal-3D-print** version of it for durability.

Measured straight from the released STL (bounding boxes, mm):

| Part | Size | Triangles | Notes |
|---|---|---|---|
| `motor_1503_reference` | 18.1 Ø × 11.2 | 3 102 | reference model of the motor, **do not print** |
| `motor_mount` | 25.26 Ø × 8.0 | 2 750 | |
| `external_ring_gear_m0.6_30T` | 24.65 Ø × 5.75 | 4 218 | |
| `external_ring_gear_flipped_m0.6_30T` | 24.66 Ø × 8.5 | 28 128 | much denser mesh |
| `internal_ring_gear_m0.6_27T` | 17.39 Ø × 4.0 | 2 664 | |
| `mystery_intermediate_driver` | 16.0 Ø × 10.5 | 1 470 | author's own name for this part |
| `shaft_rocker_arm` | 15.0 Ø × 9.5 | 2 078 | |

Two parts in the print folder are **not printed parts at all** — they are
steel rod:

| Part | Measured | Qty | Reality |
|---|---|---|---|
| `crank_output` | 6.0 × 1.4 × 1.4 mm | 4 | **1.5 mm steel rod**, cut to 6 mm |
| `part1_rod` | 31.8 × 1.4 × 1.4 mm | 4 | **1.5 mm steel rod**, cut to length |

Evidence: each appears exactly 4× in the assembly, spaced 90° apart, and
each measures 1.4 mm across. A 1.5 mm cylinder modelled with 8 facets has a
bounding box of 1.386 mm — that matches. This also lines up with the
author's consumables list, which specifies **1.5 mm rod**.

> **TODO(author):** confirm the kinematic scheme and the reduction ratio.
> The 27T internal ring and 30T external ring differ by 3 teeth and cannot
> mesh with each other directly, so there is an intermediate stage involved
> (`mystery_intermediate_driver` + the 4 crank pins), but the ratio is not
> stated anywhere in the released material.

---

## Getting started

```
1. Order the PCB      hardware/pcb/gerber_gen1_adc_closedloop_2026-10-21.zip
                      -> upload straight to JLCPCB / PCBWAY. Buy MP6540GU.
2. Print the gearbox  mechanical/stl/print/  (see docs/BOM.md for hardware)
3. Flash firmware     firmware/openloop/  -- copy config.h.example to
                      config.h, fix every [GUESS] value, then flash.
4. Drive it           serial monitor @ 115200, send `D`, `a 90`, or `R2`.
```

Detailed steps: [`docs/ASSEMBLY.md`](docs/ASSEMBLY.md).

### Firmware dependency versions

| Dependency | Version | Status |
|---|---|---|
| Arduino core for ESP32 | 2.x or 3.x | **TODO(author)** — confirm which was used |
| SimpleFOC | 2.3.x | **TODO(author)** — inferred from the API used (`MotionControlType`, `Sensor::init`, `analogReadMilliVolts`) |
| Board target | ESP32-C3 | exact module variant **TODO(author)** |

---

## ⚠️ The firmware ships without its config header

`openloop.ino` does `#include "config.h"`, but **`config.h` was not part of
the released material set**. It will not compile as-is.

[`firmware/openloop/config.h.example`](firmware/openloop/config.h.example)
is a reconstruction: all 17 macros the firmware references, with every
value recovered from comments inside the `.ino`. Each line is tagged
`[DERIVED]` (recovered verbatim) or `[GUESS]` (placeholder you must fix).

Six values are unknown and need the author's real numbers:
`MOTOR_POLE_PAIRS`, `OPENLOOP_DIRECTION`, `VELOCITY_LIMIT`,
`VOLTAGE_RAMP_MS`, `ARRIVE_TOL_RAD`, `HOLD_VOLTAGE_RATIO`.

There is also a **contradiction inside the original source** about the
voltage ceiling — three different values appear in its comments (15 %, 10 %,
and 0.5 V). See
[`docs/FIRMWARE.md` § Known inconsistencies](docs/FIRMWARE.md#known-inconsistencies-in-the-original-source).

---

## Cost

The project is named after the driver board. The author's own LCEDA project
is titled **"10元 1503 最小 FOC"** — 10 CNY ≈ **$1.40** for a bare 4-layer
board at JLCPCB prototype pricing.

> **TODO(author):** the cost table below is a skeleton. Please confirm what
> "$1.5" is meant to cover (the bare PCB, or the PCB populated), and fill in
> the rest.

| Item | Est. cost | Source | Status |
|---|---|---|---|
| Driver PCB, bare, 4-layer | ~$1.40 | JLCPCB | author-stated (10 CNY) |
| MP6540GU gate driver | ? | | **TODO** |
| ESP32-C3 module | ? | | **TODO** |
| Passives, connectors, LED | ? | | **TODO** |
| 1503 brushless gimbal motor | ? | | **TODO** |
| Analog angle sensor | ? | | **TODO** |
| Bearings (MR117 + MR85) | ? | | **TODO** |
| 1.5 mm steel rod | ? | | **TODO** |
| Printed gearbox filament | ? | | **TODO** |
| **Total per joint** | **?** | | **TODO** |

---

## Repository layout

```
.
├── firmware/
│   ├── openloop/            English firmware + reconstructed config.h.example
│   └── openloop-zh/         original Chinese source, unmodified
├── hardware/pcb/
│   ├── gerber/              16 extracted layer files
│   └── *.zip                manufacturing zip, ready to upload
├── mechanical/
│   ├── cad-solidworks/      13 SolidWorks parts + the assembly
│   └── stl/
│       ├── print/           9 unique geometries, print from here
│       └── as-exported/     all 15 original assembly-instance exports
├── docs/
│   ├── FIRMWARE.md  HARDWARE.md  BOM.md  ASSEMBLY.md  MISSING.md
└── assets/                  photos & video — currently empty
```

### About the STL folders

The original exports came out of the SolidWorks assembly, so **each file
carries its placement transform** and does not sit at the origin. Most
slicers auto-centre, so this is usually harmless, but it is worth knowing.

`stl/print/` de-duplicates the repeated instances: `crank_output` and
`part1_rod` each appeared 4× with identical triangle counts and identical
dimensions — same part, four placements. Print one and make four.
`stl/as-exported/` keeps all 15 originals for fidelity.

> **TODO(author):** three CAD parts have **no STL export** — `universal_adapter`,
> `universal_adapter_extended`, and `external_ring_gear_metal_print_m0.6_30T`.
> Please export them if they are meant to be built.

---

## Missing material

See [`docs/MISSING.md`](docs/MISSING.md) for the full, itemised list of what
still needs to be supplied before this project is reproducible by a stranger.
The headline gaps:

1. **`config.h`** — firmware will not compile without it
2. **Closed-loop firmware** — the Gerbers are named *"gen 1 ADC closed
   loop"*, but only open-loop code was released
3. **Schematic / editable PCB source** in-repo — currently only Gerbers plus
   an external OSHWHub link
4. **Real BOM with part numbers** — the board's `U1`–`U4` and all passive
   values are unknown
5. **Still photos** — three intro videos are linked via the author's NetDisk
   share, but there is no still image of the bare PCB or the gear train
6. **Print settings** — material, layer height, infill, supports, and
   whether the ring gear is printed in metal or resin
7. **Kinematic description** — the ratio is known (**10:1, single stage**,
   from the author's own video), but how the stages connect is not

---

## Full archive mirror

GitHub can be slow or unreachable from mainland China, so the author also
hosts the complete material set on Baidu NetDisk:

> **百度网盘 / Baidu NetDisk:**
> <https://pan.baidu.com/s/11fmlISdtuOBgRAKb2E5Nxg?pwd=RRRR>
> Extraction code / 提取码: **RRRR**

This is the same material as this repository, as one zip. Use whichever is
faster for you. If the two ever disagree, **this repository is the source of
truth** — the mirror is a convenience copy and may lag behind.

## Tipping

There is no donation button, no QR code and no sponsorship account, and there
won't be. This project is free and stays free.

If it saved you time or money and you would like to say thanks with money
anyway, the author's preference is simple:

> **Send the author a private message, or leave a comment.**

That is the whole mechanism. A message telling you what you built with it is
worth more than the money would be.

**Ways to help that cost nothing, and matter more:** build one and report what
broke · send the files listed in [`docs/MISSING.md`](docs/MISSING.md) · star
the repo · open an issue · send a PR.

---

## License

**CERN Open Hardware Licence Version 2 – Weakly Reciprocal
(CERN-OHL-W-2.0)** — full text in [`LICENSE`](LICENSE). SPDX: `CERN-OHL-W-2.0`.

In plain terms:

- **You may** study, copy, modify, make and sell products from this design.
- **If you convey the joint design itself** (or a modified version of it), you
  must make the source for *that design* available under the same licence.
- **If you build the joint into a larger product** — a robot arm, a quadruped,
  a pan-tilt head — the rest of your product is *External Material* and may
  stay closed and be sold under your own terms, provided it only interfaces
  with this design through the documented interface (the output adapter is
  exactly that interface).
- Notices, including the copyright line, must be retained.

Weakly reciprocal was chosen over strongly reciprocal (CERN-OHL-S) on purpose:
the goal of this project is that cheap actuation gets *used*, and S would force
anyone embedding the joint to open-source their whole machine. W keeps the
joint itself open and improving while leaving everything around it free.

### Third-party components

- **SimpleFOC** ([arduino-FOC](https://github.com/simplefoc/Arduino-FOC)) is
  **MIT**-licensed. The firmware builds on it; its copyright notice is
  retained in [`firmware/README.md`](firmware/README.md) as MIT requires.
  MIT is permissive, so this causes no licence conflict.
- **ESP32-C3 SDK / Arduino core** components remain under their own licences.

---

## Credits

Original design, PCB, CAD and firmware by the author of the 1503 minimal
FOC project. English documentation, repository organisation and firmware
translation added for this release.

> **TODO(author):** what name or handle should be credited here, and what
> email/username goes in the LICENSE copyright line? The repo is currently
> staged under the git identity `yzytz17 <lsb6699@qq.com>`.

Upstream board project:
[10元 1503 最小 FOC — OSHWHub](https://oshwhub.com/team_dvojhors/project_ceehsgzu)
