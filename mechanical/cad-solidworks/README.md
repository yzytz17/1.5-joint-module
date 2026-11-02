# SolidWorks CAD

13 parts plus the top-level assembly. This is the design source; the STL
exports live in [`../stl/`](../stl/).

> **These are native SolidWorks files** (`.SLDPRT` / `.SLDASM`). They require
> SolidWorks to open. The author's own CAD walkthrough video shows
> **SOLIDWORKS Design Premium 2025 SP3.0** in the title bar, so that is the
> version these were saved with — and SolidWorks will not open a file saved by
> a *newer* version, so if yours is older than 2025 SP3.0 you are out of luck.
>
> **TODO(author):** which SolidWorks release are these saved as? Consider also
> exporting a neutral format (`STEP` AP214) so the CAD is usable without a
> SolidWorks licence. STEP files are also what most machining and metal-print
> services want.

## Files

| File | Original name | Notes |
|---|---|---|
| `gearbox_assembly.SLDASM` | 装配体3 | top-level assembly, 15 instances across 9 unique parts |
| `motor_1503_reference.SLDPRT` | 1503无刷电机 - 副本 | reference model of the motor, **not a part to make** |
| `motor_mount.SLDPRT` | 电机座 | 25.26 Ø × 8.0 mm |
| `internal_ring_gear_m0.6_27T.SLDPRT` | 内齿圈0.6M 27T | module 0.6, 27 teeth, internal |
| `external_ring_gear_m0.6_30T.SLDPRT` | 外齿圈0.6M 30T | module 0.6, 30 teeth, external |
| `external_ring_gear_flipped_m0.6_30T.SLDPRT` | 外齿圈反面0.6M 30T | **different part**, not a mirror: 8.5 mm thick vs 5.75 mm |
| `external_ring_gear_metal_print_m0.6_30T.SLDPRT` | 金属3D打印外齿圈 | metal-3D-print variant — **no STL export** |
| `mystery_intermediate_driver.SLDPRT` | 神秘中间传递 | author's own name for it — **function undocumented** |
| `crank_output.SLDPRT` | 类曲柄输出 | 4-pin crank output stage; the pins are 1.5 mm steel rod |
| `shaft_rocker_arm.SLDPRT` | 轴摇柄 | |
| `universal_adapter.SLDPRT` | 综合转接器 | output interface — **no STL export** |
| `universal_adapter_extended.SLDPRT` | 综合转接器延展 | longer variant — **no STL export** |
| `part1_rod.SLDPRT` | 零件1 | 1.5 mm steel rod, 4× at 31.8 mm |

## File sizes

`external_ring_gear_flipped_m0.6_30T.SLDPRT` is 2.96 MB and
`external_ring_gear_m0.6_30T.SLDPRT` is 1.50 MB — together over half the
repository's ~11 MB. That is normal for SolidWorks part files with patterned
gear teeth (the feature history and sketch geometry are retained). The whole
set is well under GitHub's 100 MB per-file limit, so **Git LFS is not needed**.

## Three parts have no STL

`universal_adapter`, `universal_adapter_extended`, and
`external_ring_gear_metal_print_m0.6_30T` exist only as CAD. The first is
almost certainly the output interface — the thing you would bolt a robot limb
to — so its absence is significant.

See [`../../docs/MISSING.md`](../../docs/MISSING.md) item 10.

## What is not documented

- **SolidWorks version** — resolved: **2025 SP3.0**, read off the title bar in
  the author's CAD walkthrough video
- **No STEP / IGES export** — blocks anyone without a SolidWorks licence
- **No drawings** — no dimensioned 2D views, so nothing can be machined
  conventionally instead of printed
- **No kinematic description** — see
  [`../README.md`](../README.md#reduction-ratio-unknown)
- **The assembly is named `装配体3`** ("assembly 3"), implying assemblies 1
  and 2 existed. **TODO(author): are earlier revisions worth publishing?**
