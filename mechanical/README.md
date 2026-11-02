# Mechanical

The printed reduction gearbox for the joint.

| Folder | Contents |
|---|---|
| [`cad-solidworks/`](cad-solidworks/) | 13 SolidWorks parts + `gearbox_assembly.SLDASM` — the design source |
| [`stl/print/`](stl/print/) | **9 unique printable geometries** — print from here |
| [`stl/as-exported/`](stl/as-exported/) | all 15 original assembly-instance exports, unmodified |

## Gear train

All gears are **module 0.6**:

- internal ring gear, **27T**
- external ring gear, **30T**
- a *flipped* external ring gear variant — 8.5 mm thick vs 5.75 mm, and 28 128
  triangles vs 4 218. A genuinely different part, not a mirror
- a *metal 3D print* variant of the external ring gear (CAD only, no STL)

Plus a motor mount, an intermediate driver the author named
`神秘中间传递` ("mystery intermediate transfer"), a shaft rocker arm, a
4-pin crank output stage, and two universal output adapters (CAD only).

## Reduction ratio: unknown

**Nothing in the released material states the ratio**, and it cannot be
derived from the files. The 27T internal ring and 30T external ring differ by
3 teeth and **cannot mesh with each other directly**, so there is an
intermediate stage — but its function and the resulting ratio are not
recoverable from geometry alone.

See [`../docs/MISSING.md`](../docs/MISSING.md) item 6.

## Two "printed" parts are actually steel rod

`crank_output` and `part1_rod` shipped in the folder named
*"用于打印STL格式"* ("STL format, for printing"), but neither is a printed
part:

| Part | Measured | Qty | Reality |
|---|---|---|---|
| `crank_output` | 6.0 × 1.4 × 1.4 mm | 4 | **1.5 mm steel rod**, cut to 6 mm |
| `part1_rod` | 31.8 × 1.4 × 1.4 mm | 4 | **1.5 mm steel rod**, cut to length |

The reasoning: a 1.5 mm cylinder modelled with 8 facets has a bounding box of
2 × 0.75 × cos(22.5°) = **1.386 mm**, which is the 1.4 mm measured here. Both
parts appear exactly 4× in the assembly, 90° apart. And the author's own
consumables list specifies **1.5 mm rod**.

So: buy 1.5 mm rod, cut **4 × 6 mm** and **4 × ~32 mm**.

## Name mapping

Files were renamed from Chinese to English. Original names are recorded here
so the author can check the translations are faithful.

### CAD

| Original | English |
|---|---|
| `装配体3.SLDASM` | `gearbox_assembly.SLDASM` |
| `1503无刷电机 - 副本.SLDPRT` | `motor_1503_reference.SLDPRT` |
| `电机座.SLDPRT` | `motor_mount.SLDPRT` |
| `内齿圈0.6M 27T.SLDPRT` | `internal_ring_gear_m0.6_27T.SLDPRT` |
| `外齿圈0.6M 30T.SLDPRT` | `external_ring_gear_m0.6_30T.SLDPRT` |
| `外齿圈反面0.6M 30T.SLDPRT` | `external_ring_gear_flipped_m0.6_30T.SLDPRT` |
| `金属3D打印外齿圈.SLDPRT` | `external_ring_gear_metal_print_m0.6_30T.SLDPRT` |
| `神秘中间传递.SLDPRT` | `mystery_intermediate_driver.SLDPRT` |
| `类曲柄输出.SLDPRT` | `crank_output.SLDPRT` |
| `轴摇柄.SLDPRT` | `shaft_rocker_arm.SLDPRT` |
| `综合转接器.SLDPRT` | `universal_adapter.SLDPRT` |
| `综合转接器延展.SLDPRT` | `universal_adapter_extended.SLDPRT` |
| `零件1.SLDPRT` | `part1_rod.SLDPRT` |

### STL

The original STL exports were named `装配体3 - <part>-<instance>.STL`. The
`装配体3 - ` prefix (the assembly name) was dropped and `<instance>` became
`_inst<N>`.

| Original | English |
|---|---|
| `装配体3 - 1503无刷电机 - 副本-1.STL` | `motor_1503_reference.stl` |
| `装配体3 - 电机座-1.STL` | `motor_mount.stl` |
| `装配体3 - 内齿圈0.6M 27T-1.STL` | `internal_ring_gear_m0.6_27T.stl` |
| `装配体3 - 外齿圈0.6M 30T-1.STL` | `external_ring_gear_m0.6_30T.stl` |
| `装配体3 - 外齿圈反面0.6M 30T-1.STL` | `external_ring_gear_flipped_m0.6_30T.stl` |
| `装配体3 - 神秘中间传递-1.STL` | `mystery_intermediate_driver.stl` |
| `装配体3 - 轴摇柄-1.STL` | `shaft_rocker_arm.stl` |
| `装配体3 - 类曲柄输出-{1,2,3,4}.STL` | `crank_output_inst{1..4}.stl` |
| `装配体3 - 零件1-{2,3,4,5}.STL` | `part1_rod_inst{2,3,4,5}.stl` |

Note `零件1` has instances **2–5 but no instance 1**. Either instance 1 is
suppressed in the assembly, or its export failed. **TODO(author): confirm
whether there should be a fifth rod.**

> **TODO(author):** several of these translations are literal and could be
> better. `神秘中间传递` → "mystery intermediate driver" keeps your own
> naming; `类曲柄输出` → "crank output" and `零件1` → "part1_rod" are
> guesses at function from measured geometry. Please correct any that are
> wrong — especially `零件1`, which is the least descriptive name in the set.

## Three CAD parts have no STL

| CAD file | Why it matters |
|---|---|
| `universal_adapter.SLDPRT` | looks like the output interface — without it, nothing can be mounted to the joint |
| `universal_adapter_extended.SLDPRT` | longer variant |
| `external_ring_gear_metal_print_m0.6_30T.SLDPRT` | the durable version of a wear part |

See [`../docs/MISSING.md`](../docs/MISSING.md) item 10.
