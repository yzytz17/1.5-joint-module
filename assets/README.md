# Assets

Poster frames for the three intro videos, plus a place for still photos.

## What is here

| File | What it shows |
|---|---|
| `video/01-bench-demo-poster.jpg` | the board driving the motor on the bench, with the author's title overlay: *"开源 / 十元 foc（无刷电机闭环）/ 谁都用得起"* |
| `video/02-cad-walkthrough-10x-reduction-poster.jpg` | the gearbox in SolidWorks — the title bar in this recording reads *Design Premium 2025 SP3.0*, while the author's current toolchain is 2026 — overlay: *"Foc 单级十倍减速 / 可 3D 打印，适配 1503 foc"* |
| `video/03-assembled-joint-in-hand-poster.jpg` | the finished joint held in hand, overlay: *"全网最便宜 / 十元 foc 关节带减速器 / 2 元减速 / 开源"* |

The posters are the frames GitHub shows in the README table. They also happen
to be the only still images of the project in the repository.

## The videos themselves

The `.mp4` files are **not** in the repository — ~100 MB of video would be
downloaded by everyone who clones, forever. They live on the author's Baidu
NetDisk share instead:

> <https://pan.baidu.com/s/1f1OeAkVNAqZQWN0Y5FbujQ?pwd=ge5x> — 提取码 **ge5x**

The READMEs link there. If you want a copy that cannot rot, mirror them
somewhere stable (a GitHub Release on this repo is the obvious choice: 2 GB
per file, no history bloat, permanent URLs) and update the two README links.

## Still photos still wanted

Video covers "it exists and it turns". It does not cover the internals, which
is what a builder actually needs. Please add, if you have them:

| Suggested filename | What to shoot | Why it matters |
|---|---|---|
| `pcb-top.jpg` | bare or populated PCB, top side, macro | there is **no schematic** — this is the only reference a builder has |
| `pcb-bottom.jpg` | bottom side | `U2` and `U4` live here |
| `pcb-in-motor.jpg` | board installed in the motor bell | shows the radial phase pads in situ |
| `headers-labelled.jpg` | the two 6-pin headers with a pen/label | their pinout is the biggest open question |
| `gear-train.jpg` | printed gears and rod laid out before assembly | print orientation and fit reference |
| `exploded.png` | exploded view exported from `gearbox_assembly.SLDASM` | free, and you already have the CAD |

GitHub renders `.png`, `.jpg` and `.gif` inline in the README, so a photo
dropped in here and linked from the README just works.

See [`../docs/MISSING.md`](../docs/MISSING.md) item 8.
