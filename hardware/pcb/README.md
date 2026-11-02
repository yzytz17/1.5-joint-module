# PCB

The driver board: **4 layers, ≈17.6 × 16.4 mm**, shaped to sit inside the bell
of a 1503 brushless gimbal motor.

| Item | Path |
|---|---|
| Manufacturing zip — upload this to your fab | [`gerber_gen1_adc_closedloop_2026-10-21.zip`](gerber_gen1_adc_closedloop_2026-10-21.zip) |
| Extracted layer files, for inspection | [`gerber/`](gerber/) |
| Author's ordering note, translated | [`gerber/ORDERING_NOTES.md`](gerber/ORDERING_NOTES.md) |
| Full board analysis | [`../../docs/HARDWARE.md`](../../docs/HARDWARE.md) |

## Key facts

| | |
|---|---|
| Layers | **4** (top, bottom, inner1, inner2) |
| Outline | ≈ **17.59 × 16.38 mm**, rounded corners |
| Tool | EasyEDA Pro **v4.1.60** |
| Generated | **2026-10-21 22:08:49** |
| Min drill | **0.305 mm** (12 mil), 14 PTH + separate via file |
| Large holes | 3 × **2.0 mm** |
| Placements | **28** |
| Two-sided SMT | **Yes** — top *and* bottom paste layers; `U2` and `U4` are bottom-side |

## The detail that defines this board

The three motor-phase pads are placed **120° apart on a 6.0 mm radius circle**:

| Pad | Angle | Radius |
|---|---|---|
| `PAD1` | 90° | 6.00 mm |
| `PAD2` | 210° | 5.97 mm |
| `PAD3` | 330° | 5.97 mm |

That is the signature of a board designed to live *inside* the motor, with the
phase wires fanning out radially to the stator rather than routing off an edge.

## Critical component

> **The gate driver must be purchased as `MP6540GU`.**

This is the only component requirement the author documented, and it was
emphatic about it ("一定" — *must / definitely*). Other MP6540 package
markings are implied to be incompatible with this footprint.

## What is not here

- **No schematic.** The Gerbers give component positions, not identities.
  `U1` (26-pin), `U2` (8-pin), `U3` (3-pin), `U4` (3-pin) are all unknown.
- **No BOM values.** `R1`–`R4`, `C1`–`C9`, `H1`, `H2`, `LED1` — nothing.
- **No header pinouts.** Two 6-pin headers (`A-P-3-G`, `G-A-B-C`) with no
  documented mapping to A/B/C/EN/encoder.
- **No layer stackup** specification for the 4-layer build.
- **No pick-and-place / CPL file.** `gerber/FlyingProbeTesting.json` has
  component centroids, but most fabs will not accept it as a placement file.
- **No editable EasyEDA Pro project export.**

The upstream editable design is on the LCEDA platform:
**[10元 1503 最小 FOC — OSHWHub](https://oshwhub.com/team_dvojhors/project_ceehsgzu)**
It is *not* mirrored here, so this repository cannot currently outlive that
account.

See [`../../docs/MISSING.md`](../../docs/MISSING.md) items 2, 3, 5.

## Filename note

The original zip contains one text file whose name is stored in a Chinese
codepage. Extracting the zip on a non-CJK system produces mojibake — something
like `PCBõ©ïÕìòÕ+à޻+.txt`. The bytes could not be cleanly recovered, so the
file was **not** copied into `gerber/`. Its content was short and has been
translated into [`gerber/ORDERING_NOTES.md`](gerber/ORDERING_NOTES.md).

If you extract the original zip yourself and see a garbled filename, that is
expected and harmless — the 16 Gerber/drill/test files all have ASCII names.
