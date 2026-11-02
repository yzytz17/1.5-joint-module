# Firmware

ESP32-C3 + SimpleFOC, driving a 1503 brushless gimbal motor in **pure open
loop**.

| Folder | Contents |
|---|---|
| [`openloop/`](openloop/) | English firmware. `openloop.ino` + `config.h.example`. **This is the primary version** |
| [`openloop-zh/`](openloop-zh/) | the author's original Chinese source, byte-for-byte unmodified |

## It will not compile as-is

`openloop.ino` does `#include "config.h"`, and **`config.h` was not part of
the released material**. Do this first:

```bash
cd openloop
cp config.h.example config.h
```

Then fix every line tagged `[GUESS]`. Six values are unknown:

`MOTOR_POLE_PAIRS` · `OPENLOOP_DIRECTION` · `VELOCITY_LIMIT` ·
`VOLTAGE_RAMP_MS` · `ARRIVE_TOL_RAD` · `HOLD_VOLTAGE_RATIO`

Full table, plus the reasoning behind each recovered value, is in
[`../docs/FIRMWARE.md`](../docs/FIRMWARE.md).

## Translation provenance

`openloop/openloop.ino` is a translation of `openloop-zh/开环代码.ino`.
Comments and serial strings were translated; **control logic was not touched.**

This was verified mechanically rather than by eye: both files were stripped of
comments and string-literal contents, tokenised, and compared. Each reduces to
the same **713 code tokens**, in the same order.

Two intentional comment corrections were made in the English version, both
documented inline:

1. The stale `setup()` comment claiming `B=GPIO7` now reads `B=GPIO3`, with a
   note that GPIO7 is faulty on this board.
2. The `loop()` banner no longer asserts a specific percentage, because the
   original states the output ceiling three different ways (15 %, 10 %, 0.5 V).
   The English version refers to `VOLTAGE_LIMIT_RATIO` symbolically instead.

## Not released

The Gerber zip is named **`Gerber_1代adc闭环`** — "generation 1, **ADC closed
loop**" — but only **open-loop** firmware was supplied. If a closed-loop
version exists, it belongs here as `closedloop/`.

See [`../docs/MISSING.md`](../docs/MISSING.md) item 4.

---

## License and third-party attribution

This firmware is part of the repository and is licensed under
**CERN-OHL-W-2.0** (see [`../LICENSE`](../LICENSE)).

It builds on **SimpleFOC** ([arduino-FOC](https://github.com/simplefoc/Arduino-FOC)):

> SimpleFOC — Copyright (c) the SimpleFOC contributors.
> Licensed under the **MIT licence**.
> <https://github.com/simplefoc/Arduino-FOC>

MIT is permissive, so incorporating it into a CERN-OHL-W project is
straightforward: the only obligation is to retain its copyright notice, which
is done here and in the header of `openloop/openloop.ino`.

Note that SimpleFOC is MIT, **not** GPL — an earlier draft of this repository
worried about GPL-3.0 "contamination" of the firmware. That worry was wrong
and has been removed; it was based on an unverified assumption about
SimpleFOC's licence, which the repository's own `LICENSE` file settles as MIT.

The Arduino core for ESP32 and the ESP32-C3 SDK remain under their own
licences (Apache-2.0 / LGPL-2.1 respectively for their components).
