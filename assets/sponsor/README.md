# Support this project / 支持这个项目

## English

**Open source earns nothing.** Everything in this repository is free and will
stay free — every Gerber layer, every SolidWorks part, every line of firmware.

But it cost real time. The board went through revisions. The gearbox was
printed, measured, and printed again. The firmware was debugged pin by pin —
including finding out the hard way that GPIO7 on this board is damaged and
moving phase B to GPIO3.

If this project saved you time or money, consider buying me the board it is
named after:

> **$1.50 — the price of one bare four-layer PCB.**

A tip of ¥10–20 (roughly $1.50–$3) is more than enough. There is no tier, no
reward and no obligation. It just tells me the work landed somewhere.

### How

**From mainland China** — scan either code:

| Alipay / 支付宝 | WeChat / 微信 |
|---|---|
| `alipay-qr.png` *(not yet uploaded)* | `wechat-qr.png` *(not yet uploaded)* |

**Internationally** — whichever of these is enabled:

- Ko-fi: *(TODO — not yet set up)*
- Buy Me a Coffee: *(TODO — not yet set up)*
- 爱发电 / Afdian: *(TODO — not yet set up)*

> **Note on GitHub Sponsors:** the native Sponsor button is deliberately not
> enabled. Anyone can *give* through GitHub Sponsors, but receiving requires
> residence in a supported region — GitHub's list includes Hong Kong SAR and
> Macau SAR but not mainland China, and payouts run through Stripe. Enabling it
> would produce a button that takes money it cannot deliver.

### What tips actually go towards

Concretely: replacement motors after one is cooked by an open-loop stall,
filament and resin, PCB revisions, bearing and steel-rod stock, and the
occasional meter or logic analyser. This project's whole premise is that the
bill of materials should be trivial — keeping it trivial is what the money
buys.

### Other ways to help that cost nothing

Honestly, these matter more:

- **Build one and tell me what broke.** Failure reports are the fastest route
  to a better design.
- **Send the missing files.** [`../../docs/MISSING.md`](../../docs/MISSING.md)
  lists exactly what is still absent — `config.h`, the schematic, the reduction
  ratio, print settings, photos.
- **Star the repo**, or share it with someone building a cheap arm or quadruped.
- **Open an issue** with a question. Answering questions in public builds a
  record that helps the next person.
- **Send a pull request** — a translated doc, a normalised STL, a STEP export,
  a photo. All welcome.

---

## 中文

**开源没有收益。** 这个仓库里的所有东西都是免费的，也会一直免费——每一层
Gerber、每一个 SolidWorks 零件、每一行固件。

但它花了实打实的时间。板子改过版；减速箱打出来、量过、再打；固件一个引脚
一个引脚地调过——包括用代价换来"这块板的 GPIO7 是坏的"这个结论，然后把
B 相挪到 GPIO3。

如果这个项目帮你省了时间或钱，欢迎请我一块它自己命名的板子：

> **1.5 美元 —— 一块四层裸板的打样价。**

打赏 10 块 20 块（约 $1.5–$3）就够了。没有档位、没有回报、也没有义务，
只是让我知道这东西真的有人用上。

### 怎么打赏

**国内** —— 扫码：

| 支付宝 | 微信 |
|---|---|
| `alipay-qr.png` *(尚未上传)* | `wechat-qr.png` *(尚未上传)* |

**海外** —— 以下任一（暂未开通）：

- Ko-fi：*（待设置）*
- Buy Me a Coffee：*（待设置）*
- 爱发电：*（待设置）*

> **关于 GitHub Sponsors：** 原生赞助按钮是故意没开的。任何人都可以通过
> GitHub Sponsors 赞助，但**收款要求居住在受支持的地区**——官方列表里有
> 香港特别行政区和澳门特别行政区，**没有中国大陆**，且打款走 Stripe。
> 开了这个按钮，别人点了钱却到不了你手上。

### 打赏的钱会花在哪

具体说：被开环堵转烧掉的电机（补货）、耗材和树脂、改板打样、轴承和
1.5 mm 钢棒、偶尔一支万用表或逻辑分析仪。这个项目的前提就是"物料成本
应该低到可以忽略"——钱就是用来把这个前提维持住的。

### 不花钱但更有用的支持方式

说真的，这些更重要：

- **做一个出来，然后告诉我哪里坏了。** 失效报告是改进设计最快的路径。
- **把缺的文件补上。** [`../../docs/MISSING.md`](../../docs/MISSING.md)
  里列清楚了还缺什么——`config.h`、原理图、减速比、打印参数、照片。
- **给仓库点个 Star**，或者转给正在做廉价机械臂/四足的人。
- **开 Issue 提问。** 公开回答能留下记录，帮到下一个人。
- **提 PR** —— 翻译文档、归一化 STL、导出 STEP、发张照片，都欢迎。

---

## Repo maintainer notes

*This section is instructions, not part of the public page — delete it before
publishing if you prefer.*

To make the QR codes appear:

1. Export your Alipay collection code as `alipay-qr.png` and your WeChat
   collection code as `wechat-qr.png`.
2. Drop both into this folder (`assets/sponsor/`).
3. Replace the two placeholder cells in the tables above with real image
   links:

   ```markdown
   | Alipay / 支付宝 | WeChat / 微信 |
   |---|---|
   | ![Alipay](alipay-qr.png) | ![WeChat](wechat-qr.png) |
   ```

4. Commit and push. GitHub renders relative image paths in Markdown, so no
   external image host is needed.

Practical notes:

- Keep each QR under ~200 KB. They render at table-cell size, and large PNGs
  slow the page for no benefit.
- Prefer a **fixed-amount** collection code (e.g. ¥10) over an open one if you
  want to reinforce the "$1.5" framing.
- **Do not commit a QR that encodes a personal account number in plain text**
  if you are uncomfortable with it being mirrored. Alipay/WeChat collection
  codes are designed to be public, but be aware forks keep them forever.
- If you set up Ko-fi or Buy Me a Coffee later, uncomment the matching line in
  [`.github/FUNDING.yml`](../../.github/FUNDING.yml) — the Sponsor button
  appears automatically once that file has a valid key.
