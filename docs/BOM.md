# Bill of Materials

> **This BOM is incomplete.** The released material contained a short
> consumables list (`需要的其他耗材.docx`) plus a Gerber set and a firmware
> file — no formal BOM, no schematic, and no part numbers for the
> electronics. Everything below marked **TODO** has to come from you.
>
> What *is* here was recovered from three sources: the author's consumables
> note, the flying-probe placement data inside the Gerber set, and direct
> measurement of the released STL files.

---

## 1. Electronics

| # | Item | Qty | Reference | Status |
|---|---|---|---|---|
| 1 | Custom 4-layer driver PCB, ≈17.6 × 16.4 mm | 1 | — | Gerbers supplied ✓ |
| 2 | **MP6540GU** three-phase gate driver w/ integrated FETs | 1 | `U2`? | **Author: must buy exactly the `GU` package variant.** Which ref-des is unconfirmed. |
| 3 | ESP32-C3 (module or bare chip) | 1 | `U1` (26-pin, top) | **TODO** — exact module variant |
| 4 | 3-pin part | 1 | `U3` (top) | **TODO** — unknown |
| 5 | 3-pin part | 1 | `U4` (bottom) | **TODO** — unknown |
| 6 | Resistor | 4 | `R1`–`R4` | **TODO** — values unknown |
| 7 | Capacitor | 9 | `C1`–`C9` | **TODO** — values unknown |
| 8 | LED | 1 | `LED1` | **TODO** — colour / part |
| 9 | Connector, 3-pin | 2 | `H1`, `H2` | **TODO** — pitch and part |
| 10 | Header, 6-pin | 2 | `A-P-3-G`, `G-A-B-C` | **TODO** — pitch, and pinout (A/B/C/EN/encoder) |
| 11 | Magnetic angle sensor, **PWM output mode**, read on the MCU's ADC (not I2C) | 1 | off-board | **TODO** — part number and how PWM output mode is selected. AS5600/MT6701-class parts offer PWM among their output modes |
| 12 | Solder paste + stencil | — | — | Both top and bottom paste layers exist; `U2`/`U4` are bottom-side. This is a two-sided reflow job |

**Board cost:** the author's upstream LCEDA project is titled *"10元 1503 最小
FOC"* → **10 CNY ≈ $1.40** for the bare 4-layer board at prototype pricing.
That is the "$1.5" in this project's name.

---

## 2. Motor

| Item | Qty | Notes | Status |
|---|---|---|---|
| **1503** brushless gimbal motor | 1 | Measured from the reference STL: **18.1 mm Ø × 11.2 mm** thick | **TODO** — KV rating, winding resistance, inductance, supplier link |
| Pole pairs | — | Firmware needs `MOTOR_POLE_PAIRS`; guessed as 7 (12-slot / 14-pole). **Count the rotor magnets to confirm** | **TODO** |

`mechanical/stl/print/motor_1503_reference.stl` is a **CAD reference model of
the motor, not a printable part.** Do not print it.

---

## 3. Bearings and rod

Recovered from `需要的其他耗材.docx` ("Other consumables required"):

| Item | Spec as written | Standard designation | Qty | Status |
|---|---|---|---|---|
| Bearing | 内 7 mm，外 11 mm，厚 3 mm → ID 7 × OD 11 × W 3 mm | **MR117 / 6701** class | **TODO** | size confirmed, qty unknown |
| Bearing | 内 5 mm，外 8 mm，厚 2.5 mm → ID 5 × OD 8 × W 2.5 mm | **MR85** class | **TODO** | size confirmed, qty unknown |
| Steel rod | 1.5 mm 粗 → **1.5 mm diameter** | music wire / silver steel | 2 lengths | confirmed |

### The rod is used for two different parts

Measured from the STL, both are 1.4 mm across — a 1.5 mm cylinder modelled
with 8 facets has a 1.386 mm bounding box, which is what we see:

| Printed name | Cut length | Qty | Assembly placement |
|---|---|---|---|
| `crank_output` | **6.0 mm** | 4 | 90° apart, ≈5.5 mm from the axis |
| `part1_rod` | **31.8 mm** | 4 | 90° apart, ≈11.5 mm from the axis |

So buy 1.5 mm rod and cut **4 × 6 mm** and **4 × ~32 mm**.

> **TODO(author):** confirm the 31.8 mm rods are cut to 31.8 mm, or whether
> that is just the stock length drawn in CAD and they get trimmed on assembly.
> At 31.8 mm they span almost the whole 41 mm assembly stack, which is unusual.

### Original purchase links (China only)

The author's note pointed at Pinduoduo mobile links. These require the
Pinduoduo app and a Chinese phone number, and mobile commerce links of this
kind rot quickly:

- Bearings: `https://mobile.yangkeduo.com/goods.html?ps=XM0X17KyeA`
- 1.5 mm rod: `https://mobile.yangkeduo.com/goods.html?ps=DSV019TH5z`

They are recorded here for provenance. **They should not be treated as a
sourcing route for an international audience** — the standard designations
above (MR117, MR85, 1.5 mm rod) are what a non-Chinese builder should search
for instead.

> **TODO(author):** how many of each bearing, and where do they go? The
> assembly has a motor mount, an intermediate driver, and a rocker arm — all
> plausible bearing seats.

---

## 4. Printed parts

All gears are **module 0.6**. Sizes below are STL bounding boxes in mm.

| File in `mechanical/stl/print/` | Size | Triangles | Qty | Material | Status |
|---|---|---|---|---|---|
| `motor_mount` | 25.26 Ø × 8.0 | 2 750 | **TODO** | **TODO** | |
| `external_ring_gear_m0.6_30T` | 24.65 Ø × 5.75 | 4 218 | **TODO** | **TODO** | |
| `external_ring_gear_flipped_m0.6_30T` | 24.66 Ø × 8.5 | 28 128 | **TODO** | **TODO** | dense mesh — see note below |
| `internal_ring_gear_m0.6_27T` | 17.39 Ø × 4.0 | 2 664 | **TODO** | **TODO** | |
| `mystery_intermediate_driver` | 16.0 Ø × 10.5 | 1 470 | **TODO** | **TODO** | author's own name |
| `shaft_rocker_arm` | 15.0 Ø × 9.5 | 2 078 | **TODO** | **TODO** | |
| ~~`crank_output`~~ | 6.0 × 1.4 × 1.4 | 152 | — | **not printed** — 1.5 mm rod | |
| ~~`part1_rod`~~ | 31.8 × 1.4 × 1.4 | 150 | — | **not printed** — 1.5 mm rod | |
| ~~`motor_1503_reference`~~ | 18.1 Ø × 11.2 | 3 102 | — | **not printed** — CAD reference | |

Notes:

- **`external_ring_gear_flipped` is a different part**, not a mirror of
  `external_ring_gear`. Same outer diameter (24.66 vs 24.65 mm) but 8.5 mm
  thick instead of 5.75 mm, and 28 128 triangles instead of 4 218 — nearly
  7× the mesh density. **TODO(author): explain when to print which.**
- A **metal 3D print** version of the external ring gear exists in CAD
  (`external_ring_gear_metal_print_m0.6_30T.SLDPRT`) but **has no STL
  export**. **TODO(author): export it, and say which process — DMLS/SLM
  steel, or resin?**
- Module 0.6 teeth at this size are near the resolution limit of FDM. With a
  0.4 mm nozzle the tooth profile will be rough. **TODO(author): what printer,
  nozzle, material, layer height, and orientation did you actually use?**

### CAD parts with no STL

Three SolidWorks parts have no corresponding STL export:

| CAD file | Why it matters |
|---|---|
| `universal_adapter.SLDPRT` | appears to be the output interface — needed to mount anything to the joint |
| `universal_adapter_extended.SLDPRT` | longer variant |
| `external_ring_gear_metal_print_m0.6_30T.SLDPRT` | the durable version of a wear part |

---

## 5. Cost summary

> **TODO(author):** this is the table that justifies the project's name. It
> needs your real numbers.

| Item | Est. cost (USD) | Source | Confirmed? |
|---|---|---|---|
| Bare 4-layer PCB | ~1.40 | JLCPCB | author-stated (10 CNY) |
| MP6540GU | ? | | ✗ |
| ESP32-C3 | ? | | ✗ |
| Passives + connectors + LED | ? | | ✗ |
| 1503 motor | ? | | ✗ |
| Analog angle sensor | ? | | ✗ |
| Bearings (MR117 × ?, MR85 × ?) | ? | | ✗ |
| 1.5 mm steel rod | ? | | ✗ |
| Filament / resin | ? | | ✗ |
| **Total per joint** | **?** | | ✗ |

---

## 6. Tools and consumables not in the author's list

Not mentioned anywhere in the released material, but you will need them:

- Solder paste, stencil (or a careful hand + fine tip), hot plate or reflow
  oven for the bottom-side components
- USB-C cable and an ESP32-C3 flashing setup (Arduino IDE + ESP32 board
  package + SimpleFOC 2.3.x)
- 3D printer capable of ~0.2 mm features, or a resin printer
- Access to metal 3D printing **if** the metal ring gear is required
- Rod cutter / flush cutter and a way to deburr the 1.5 mm pins
- Calipers, for verifying the printed gears before assembly
- A 5 V supply. **TODO(author): what is the actual current draw?** With no
  current sensing on the board, this matters for supply sizing.
