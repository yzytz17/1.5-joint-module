# PCB Ordering Notes

Translation of the author's note that shipped inside the Gerber zip.

## Original (Chinese)

> 如何进行PCB下单
>
> 请查看：
> https://prodocs.lceda.cn/cn/pcb/order-order-pcb/index.html

## Translation

> **How to order this PCB**
>
> Please see:
> <https://prodocs.lceda.cn/cn/pcb/order-order-pcb/index.html>

That is the official LCEDA (嘉立创EDA / EasyEDA Pro) documentation page on
placing a PCB order. It is written for the Chinese JLCPCB workflow.

---

## Ordering outside China

The linked guide assumes a JLCPCB account and the Chinese LCEDA client. If you
are ordering elsewhere, this is what to specify:

| Parameter | Value |
|---|---|
| File to upload | `../gerber_gen1_adc_closedloop_2026-10-21.zip` |
| Layer count | **4** |
| Outline size | ≈ 17.59 × 16.38 mm |
| Minimum drill | 0.305 mm (12 mil) |
| Smallest trace/space | not documented — **TODO(author)** |
| Copper weight | not documented — **TODO(author)** |
| Surface finish | not documented (ENIG recommended for the fine-pitch bottom-side parts) |
| Solder mask / silkscreen | both sides present |
| Stencil | **order one** — top and bottom paste layers both exist |
| Impedance control | not documented — **TODO(author)** |

Any fab that accepts a Gerber zip will work: JLCPCB, PCBWAY, Eurocircuits,
OSH Park, PCBTrain.

At JLCPCB prototype pricing, a small 4-layer board is roughly **10 CNY ≈
$1.40** — which is where this project's name comes from. The author's upstream
project is titled *"10元 1503 最小 FOC"*.

---

## Provenance note

This note was originally a plain `.txt` inside the Gerber zip with a filename
stored in a Chinese codepage. On non-CJK systems it extracts as mojibake. The
content is reproduced above; the mangled original was not carried into the
repository. See [`../README.md`](../README.md#filename-note).
