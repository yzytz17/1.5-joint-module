# STL — print from `print/`

| Folder | Files | Use |
|---|---|---|
| `print/` | 9 | **Print from here.** De-duplicated: one file per unique geometry |
| `as-exported/` | 15 | The original assembly-instance exports, unmodified. Kept for fidelity |

## Why two folders

The originals were exported out of the SolidWorks assembly, so:

1. **Repeated parts appear multiple times.** `crank_output` was exported 4×
   and `part1_rod` 4×. Each instance has an identical triangle count and
   identical dimensions — same part, four placements in the assembly:

   | Part | Instances | Triangles each | Dimensions each |
   |---|---|---|---|
   | `crank_output` | 4 (`inst1`–`inst4`) | 152 | 6.0 × 1.4 × 1.4 mm |
   | `part1_rod` | 4 (`inst2`–`inst5`) | 150 | 31.8 × 1.4 × 1.4 mm |

   `print/` keeps one of each. Print one, make four.

2. **Every file carries its placement transform.** Nothing sits at the
   origin — e.g. `part1_rod` starts at X=5.3, Y=21.5, Z=6.7 in the assembly
   frame. Most slicers auto-centre, so this is usually harmless, but if your
   slicer does not, expect parts to land off the bed.

> The originals are untouched in `as-exported/`. **TODO(author):** want the
> `print/` copies normalised to the origin and re-oriented flat for printing?
> It is a pure translation, fully reversible, and the originals stay as they
> are. See [`../../docs/MISSING.md`](../../docs/MISSING.md) item 13.

## `print/` contents

Measured bounding boxes and triangle counts, read from the binary STL headers:

| File | Size (mm) | Triangles | Qty | Print? |
|---|---|---|---|---|
| `motor_mount.stl` | 25.26 Ø × 8.0 | 2 750 | **TODO** | yes |
| `external_ring_gear_m0.6_30T.stl` | 24.65 Ø × 5.75 | 4 218 | **TODO** | yes |
| `external_ring_gear_flipped_m0.6_30T.stl` | 24.66 Ø × 8.5 | 28 128 | **TODO** | yes — see note |
| `internal_ring_gear_m0.6_27T.stl` | 17.39 Ø × 4.0 | 2 664 | **TODO** | yes |
| `mystery_intermediate_driver.stl` | 16.0 Ø × 10.5 | 1 470 | **TODO** | yes |
| `shaft_rocker_arm.stl` | 15.0 Ø × 9.5 | 2 078 | **TODO** | yes |
| `motor_1503_reference.stl` | 18.1 Ø × 11.2 | 3 102 | — | **NO** — CAD reference model of the motor |
| `crank_output.stl` | 6.0 × 1.4 × 1.4 | 152 | 4 | **NO** — 1.5 mm steel rod, cut to 6 mm |
| `part1_rod.stl` | 31.8 × 1.4 × 1.4 | 150 | 4 | **NO** — 1.5 mm steel rod, cut to ~32 mm |

So of the 9 files, **6 are printable**, 1 is a reference model, and 2 are
steel rod.

### Why the rods are not printed

Both measure 1.4 mm across. A 1.5 mm cylinder modelled with 8 facets has a
bounding box of 2 × 0.75 × cos(22.5°) = **1.386 mm** — that is the 1.4 mm seen
here. Both appear exactly 4× at 90° spacing. And the author's consumables note
lists **1.5 mm rod** as a required material. Printed 1.4 mm pins would snap;
steel rod will not.

### `external_ring_gear_flipped` is not a mirror

Same outer diameter as the normal version (24.66 vs 24.65 mm) but **8.5 mm
thick instead of 5.75 mm**, and **28 128 triangles instead of 4 218** — nearly
7× the mesh density. It is a different part. **TODO(author): when is each one
used?**

## No print settings were supplied

Nothing in the released material says what material, layer height, nozzle,
infill, wall count, supports or orientation to use.

This matters more than usual here: **module 0.6 gear teeth** on a ~17–25 mm
ring are at or below the resolution limit of FDM with a 0.4 mm nozzle. A 0.6
module tooth is roughly 1.3 mm at the pitch line — printable, but only with
care.

**TODO(author):** please supply, per part:

- material (PLA / PETG / ABS / nylon / resin?)
- printer and nozzle
- layer height
- wall/perimeter count and infill
- supports yes/no
- print orientation — flat or on edge? Gears print very differently the two ways
- any post-processing (annealing, filing the teeth, pressing in bearings)

Also: is the external ring gear meant to be printed in **metal**? There is a
`external_ring_gear_metal_print_m0.6_30T.SLDPRT` in CAD with no STL export.

See [`../../docs/BOM.md`](../../docs/BOM.md) §4 and
[`../../docs/MISSING.md`](../../docs/MISSING.md) items 7, 10, 12, 13.
