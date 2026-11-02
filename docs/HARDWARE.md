# Hardware Guide

The driver board for the joint. Four layers, roughly the size of a coin,
designed to sit inside the bell of a 1503 brushless gimbal motor.

---

## Board facts

All of the numbers below were read directly out of the released Gerbers and
the flying-probe test file — not from documentation, because none was
supplied.

| Property | Value | Source |
|---|---|---|
| CAD tool | EasyEDA Pro **v4.1.60** | Gerber header |
| Gerber generated | **2026-10-21 22:08:49** | Gerber header |
| Copper layers | **4** (top, bottom, inner1, inner2) | `Gerber_InnerLayer1.G1`, `Gerber_InnerLayer2.G2` present |
| Board outline | ≈ **17.59 × 16.38 mm** | 31-vertex outline path in `Gerber_BoardOutlineLayer.GKO` |
| Outline shape | rounded-corner, not a plain rectangle | mixed vertex radii (8.0–11.9 mm) |
| PTH drills | **14 × 0.305 mm** + **3 × 2.0 mm** | `Drill_PTH_Through.DRL` |
| Via drills | 0.305 mm, separate file | `Drill_PTH_Through_Via.DRL` |
| Placements | **28** | `FlyingProbeTesting.json` |
| Units in test file | mil | `lengthUnit: "mil"` |

### Phase pads are arranged radially

`PAD1`, `PAD2`, `PAD3` sit at:

| Pad | X (mil) | Y (mil) | Radius | Angle |
|---|---|---|---|---|
| `PAD1` | 0 | 236.22 | 236.2 mil = **6.00 mm** | 90° |
| `PAD2` | −203.52 | −117.50 | 235.0 mil = 5.97 mm | 210° |
| `PAD3` | 203.52 | −117.50 | 235.0 mil = 5.97 mm | 330° |

Exactly **120° apart on a ~6 mm radius circle**. That is the signature of a
board meant to live inside the motor: the three phase wires fan out radially
to the stator terminals instead of being routed off one edge. It also means
the board is rotationally symmetric in its most important respect, so
orientation during assembly matters.

> **TODO(author):** confirm the board is intended to mount inside the motor
> bell, and how it is retained (the `motor_mount` printed part is 25.26 mm Ø
> × 8 mm thick, which is larger than the board — is the board pressed into it,
> screwed, or glued?).

---

## Component placement map

Recovered from `FlyingProbeTesting.json`. Layer `T` = top, `B` = bottom.

| Ref | Pins | Layer | X (mil) | Y (mil) | Rot | Likely role |
|---|---|---|---|---|---|---|
| `U1` | 26 | T | 1.63 | 0.64 | 270° | main controller — **TODO(author)**: ESP32-C3 module? |
| `U2` | 8 | B | 0 | 0 | 270° | **TODO(author)**: MP6540GU gate driver? |
| `U3` | 3 | T | −160 | 235 | 0° | **TODO(author)** |
| `U4` | 3 | B | 11.44 | −206.44 | 0° | **TODO(author)** |
| `H1` | 3 | T | −243.37 | 55.64 | 90° | connector — **TODO(author)** |
| `H2` | 3 | T | 260 | 0 | 90° | connector — **TODO(author)** |
| `LED1` | 2 | T | | | | status LED |
| `R1`–`R4` | 2 each | T | | | | **TODO(author)**: values |
| `C1`–`C9` | 2 each | T | | | | **TODO(author)**: values |
| `PAD1`–`PAD3` | — | T | see above | | | motor phases A/B/C |
| `A-P-3-G` | 6 | | | | | 6-pin header — **TODO(author)** |
| `G-A-B-C` | 6 | | | | | 6-pin header — **TODO(author)** |

The two 6-pin headers are almost certainly **motor phases + encoder**, but
nothing in the released material says which pin is which. The firmware
expects phase A/B/C on GPIO10/GPIO3/GPIO2, enable on GPIO8, and an analog
angle signal on GPIO0.

> The pinout of these headers is the single biggest blocker to reproducing
> this board. **TODO(author): please supply the schematic, or at minimum a
> labelled photo of the two 6-pin headers.**

---

## Ordering the PCB

The author's note, translated:

> **How to order this PCB**
> See the official LCEDA guide:
> <https://prodocs.lceda.cn/cn/pcb/order-order-pcb/index.html>

Practical route for anyone, in or out of China:

1. Take `hardware/pcb/gerber_gen1_adc_closedloop_2026-10-21.zip` — this is
   the original manufacturing zip, unmodified.
2. Upload it to JLCPCB, PCBWAY, or any fab that accepts Gerber zips.
3. Set **4 layers**. The stackup is not documented — ask the fab to use their
   standard 4-layer stackup, or supply the author's.
4. The outline is ≈17.6 × 16.4 mm. At JLCPCB prototype pricing a small
   4-layer board runs about **10 CNY ≈ $1.40**, which is where the project's
   name comes from.
5. Specify **0.305 mm (12 mil)** minimum drill — that is what the files use.

If you prefer to work from the extracted layers instead of the zip, they are
in [`hardware/pcb/gerber/`](../hardware/pcb/gerber/):

| File | Layer |
|---|---|
| `Gerber_TopLayer.GTL` | top copper |
| `Gerber_BottomLayer.GBL` | bottom copper |
| `Gerber_InnerLayer1.G1` | inner 1 |
| `Gerber_InnerLayer2.G2` | inner 2 |
| `Gerber_TopSilkscreenLayer.GTO` | top silkscreen |
| `Gerber_BottomSilkscreenLayer.GBO` | bottom silkscreen |
| `Gerber_TopSolderMaskLayer.GTS` | top solder mask |
| `Gerber_BottomSolderMaskLayer.GBS` | bottom solder mask |
| `Gerber_TopPasteMaskLayer.GTP` | top paste (for stencil / SMT) |
| `Gerber_BottomPasteMaskLayer.GBP` | bottom paste |
| `Gerber_BoardOutlineLayer.GKO` | board outline |
| `Gerber_DrillDrawingLayer.GDD` | drill drawing |
| `Gerber_DocumentLayer.GDL` | document |
| `Drill_PTH_Through.DRL` | plated through-holes |
| `Drill_PTH_Through_Via.DRL` | plated vias |
| `FlyingProbeTesting.json` | flying-probe test data |

Note that **both top and bottom paste layers exist**, and `U2` and `U4` are on
the bottom. This board is a two-sided reflow job — plan for solder paste and
a hot plate or oven on the bottom side, not just hand soldering.

> **TODO(author):** is there a stencil file or a pick-and-place / CPL file?
> The flying-probe JSON gives component centroids, which most fabs will not
> accept as a placement file. A proper `pick_and_place.csv` would make this
> board orderable as a full SMT assembly.

---

## Critical component note

From the author's `注意事项.docx` ("Notes"), translated verbatim:

> **The MCU/driver must be purchased as MP6540GU.**

MP6540 is Monolithic Power Systems' three-phase BLDC gate driver with
integrated power FETs. The `GU` suffix denotes a specific package variant, and
the author's emphasis ("一定", i.e. *must / definitely*) suggests other
MP6540 package markings are **not** drop-in compatible with this footprint.

> **TODO(author):** confirm which of `U1`–`U4` is the MP6540GU, and supply the
> full part number as ordered (e.g. `MP6540GU-Z` or similar suffix), plus the
> supplier. Also confirm whether `U1` really is the ESP32-C3, and if so which
> module.

---

## Upstream project

The editable design is hosted on the LCEDA open-source hardware platform:

**[10元 1503 最小 FOC — OSHWHub](https://oshwhub.com/team_dvojhors/project_ceehsgzu)**

That page should contain the schematic and the full BOM. **It is not
mirrored into this repository.**

> **TODO(author):** export the EasyEDA Pro project (schematic + PCB + BOM) and
> commit it under `hardware/pcb/source/`. Relying on an external link means
> this repo cannot outlive that account, and the OSHWHub URL carries a long
> tracking token that may expire.

---

## What is missing on the hardware side

1. **Schematic** — no PDF, no EasyEDA export
2. **BOM with values** — `R1`–`R4` and `C1`–`C9` values unknown; `U1`–`U4`
   part numbers unknown
3. **Header pinouts** — which pin of the two 6-pin headers is A/B/C/EN/encoder
4. **Layer stackup** — 4-layer, but the stackup is unspecified
5. **Pick-and-place / stencil files** for SMT assembly
6. **Encoder part number** — the firmware reads an analog voltage 0–3.3 V
   mapped to 0–360°, which is characteristic of a Hall-effect angle sensor
   (AS5600 in analog-out mode, MT6701, or similar), but nothing names it
7. **The closed-loop board revision** — the Gerber is named *"gen 1 ADC closed
   loop"*, implying there is a closed-loop design and firmware, neither of
   which was released

See [MISSING.md](MISSING.md).
