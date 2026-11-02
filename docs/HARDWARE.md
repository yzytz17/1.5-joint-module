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
| `U1` | 26 | T | 1.63 | 0.64 | 270° | **MP6540GU** — 26 pins matches its QFN-26 package exactly |
| `U2` | 8 | B | 0 | 0 | 270° | likely the magnetic angle sensor (SOIC-8, AS5600/MT6701-class) in **PWM output mode**; bottom side would face the rotor magnet. **TODO(author): confirm** |
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

### The microcontroller is probably not on this board

Once `U1` is identified as the MP6540GU (26 pins = QFN-26), no remaining
placement has enough pins to be an ESP32-C3. A bare ESP32-C3 is QFN-32 or
QFN-48; a module is 20–40+ pins. The largest thing left is `U2` at 8 pins.

So the board is most likely **power stage + angle sensor + passives only**,
with the ESP32-C3 sitting on an off-board development module wired in through
the two 6-pin headers. That reading also explains the price: a ¥10 board is
plausible for a driver + sensor, not for a driver + sensor + MCU.

It also gives a plausible reading of the header names: `G-A-B-C` looks like
**G(nd) – A – B – C**, i.e. ground plus the three phase PWM lines, and
`A-P-3-G` may carry the remaining control and encoder signals.

> **TODO(author):** confirm. If the MCU really is off-board, document which
> header carries what — that mapping is currently the single biggest gap
> between "Gerbers" and "a board someone can populate and run".

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

## The driver chip: MP6540GU, not MP6540HGU

This is the load-bearing purchasing instruction for the entire project, and
the failure mode is silent: **MP6540GU and MP6540HGU share the same QFN-26
(5 × 5 mm) footprint.** A wrongly-bought H part will solder down perfectly,
pass visual inspection, and still not be the part this board was designed
around.

From the author's `注意事项.docx` ("Notes"). Original Chinese, then the
translation — the Chinese is authoritative:

> 原文：**单片机一定采购MP 6540 GU**
>
> Translation: **The MCU/driver must be purchased as MP6540GU.**

### The two parts, side by side

Values from the manufacturers' and distributors' published specifications.

| | **MP6540GU** — this board | MP6540HGU — wrong for this board |
|---|---|---|
| Family | MP6540 / MP6540A | MP6540H / MP6540HA |
| Description | 35 V, 3 A three-phase power stage | 50 V, 5 A three-phase power stage |
| Supply voltage | 5.5 – **35 V** | 5.5 – **50 V** |
| Continuous current | **3 A** | **5 A** |
| Peak current | **10 A** for 1 s | **6 A** |
| RDS(on), HS + LS | 50 mΩ | 45 – 50 mΩ (sources disagree) |
| Package | **QFN-26 (5 × 5 mm)** | **QFN-26 (5 × 5 mm)** — identical |
| LCSC part number | C5676249 (`MP6540GU-Z`) | C5358239 (`MP6540HGU-Z`) |

Both integrate three half-bridges of N-channel power MOSFETs, the pre-driver,
the gate-drive supply and a bidirectional current-sense amplifier. Both use an
internal charge pump for 100 % duty cycle. Electrically they are siblings; the
H part is simply the higher-voltage, higher-current sibling.

### Why the non-H part is the correct one

The author's statement, in the original Chinese (authoritative), then
translated:

> 原文：**选用非H版本原因是因为降低价格，并且H版本提升的电流在FOC项目中没有
> 太高意义，因为这个电机的KV值太高了，高电流会影响整个电机的稳定性。**
>
> Translation: the non-H version was chosen to lower the price, and the extra
> current the H version provides has little meaning in a FOC project like this
> one, because this motor's KV is too high — high current affects the
> stability of the whole motor.

Expanded:

1. **Price.** The non-H part is cheaper, and cost is the entire premise of
   this project.
2. **The extra current buys nothing here.** This firmware is *voltage*-limited
   (`VOLTAGE_LIMIT` = 0.75 V of a 5 V bus, and current sensing was deliberately
   removed). The H part's 5 A continuous rating is headroom this design never
   touches.
3. **The motor's KV is high.** With a high-KV winding, surplus available
   current does not become useful torque — it becomes heat, and it
   destabilises the motor as a whole. In this application the lower-current
   part is the *right* part, not a cost-cutting compromise.

Point 3 is the one worth internalising: for a high-KV gimbal-class motor
driven open loop at a fraction of a volt, current capability is not a feature.
It is a liability you would be paying extra for.

> **TODO(author):** confirm the full ordering code you used (e.g.
> `MP6540GU-Z` vs `-P`) and the supplier, so the BOM is reproducible exactly.

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
6. **Encoder part number and output-mode configuration** — the sensor's angle
   output is **PWM**, read on the ADC (GPIO0); there is no I2C in the firmware.
   That points at a magnetic angle sensor configured for PWM output
   (AS5600/MT6701-class parts offer PWM among their output modes), but nothing
   names the part or says how its output mode was selected (strap pins?
   one-time OTP?). Both matter: the same part in I2C mode would produce
   exactly the "reads a constant" symptom described in
   [FIRMWARE.md](FIRMWARE.md#the-sensors-interface-is-pwm-read-on-the-adc-not-i2c)
7. **The closed-loop board revision** — the Gerber is named *"gen 1 ADC closed
   loop"*, implying there is a closed-loop design and firmware, neither of
   which was released

See [MISSING.md](MISSING.md).
