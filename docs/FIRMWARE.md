# Firmware Guide

**Target:** ESP32-C3 · **Library:** SimpleFOC 2.3.x · **Mode:** pure open loop

The firmware lives in two places:

- [`firmware/openloop/`](../firmware/openloop/) — English translation. This is
  the primary version.
- [`firmware/openloop-zh/`](../firmware/openloop-zh/) — the author's original
  Chinese source, byte-for-byte unmodified, kept for reference.

The translation was verified mechanically: with comments and string literals
stripped, both files reduce to the **same 713 code tokens**. Control logic is
identical; only human-readable text changed.

---

## It will not compile yet

`openloop.ino` ends with `#include "config.h"` and **`config.h` was not
released**. Copy the reconstruction and fill in the blanks:

```bash
cd firmware/openloop
cp config.h.example config.h
# now edit every line tagged [GUESS]
```

### The 17 macros, and which ones are known

| Macro | Value | Confidence |
|---|---|---|
| `PIN_INHA` | `10` | derived from source comment |
| `PIN_INHB` | `3` | derived (moved off faulty GPIO7) |
| `PIN_INHC` | `2` | derived |
| `PIN_EN` | `8` | derived |
| `PIN_ENC_ANGLE` | `0` | derived |
| `POWER_SUPPLY_VOLTAGE` | `5.0f` | derived |
| `VOLTAGE_LIMIT_RATIO` | `0.15f` | derived |
| `VOLTAGE_LIMIT` | `bus × ratio` = 0.75 V | derived |
| `ANGLE_V_MIN` | `0.0f` | derived |
| `ANGLE_V_MAX` | `3.3f` | derived |
| `ANGLE_MAP_INVERT` | `0` | derived |
| `MOTOR_POLE_PAIRS` | `7` | **GUESS** — confirm |
| `OPENLOOP_DIRECTION` | `1` | **GUESS** — confirm |
| `VELOCITY_LIMIT` | `20.0f` rad/s | **GUESS** — confirm |
| `VOLTAGE_RAMP_MS` | `500` | **GUESS** — confirm |
| `ARRIVE_TOL_RAD` | `0.05f` rad | **GUESS** — confirm |
| `HOLD_VOLTAGE_RATIO` | `0.40f` | **GUESS** — confirm |

`MOTOR_POLE_PAIRS` matters most. A wrong value means the electrical angle is
wrong, and the motor will cog, stall, or run backwards. Count the magnets on
the rotor bell — pole pairs = magnets ÷ 2. A 1503 gimbal motor is very often
12 slots / 14 poles = **7 pole pairs**, but verify yours.

---

## Known inconsistencies in the original source

These are recorded so nobody wastes an evening on them. **They are in the
author's original file, not introduced by the translation.**

### 1. Three different voltage ceilings

| Where in `开环代码.ino` | Claimed value |
|---|---|
| Header comment, line ~18 | `5V × 15% = 0.75V` |
| `setup()`, where `driver.voltage_limit` is assigned | `= 母线 × 15%` |
| `loop()` banner comment | `输出电压限 10%` |
| `loop()`, velocity-mode comment | `速度开环: 满压(10%)` |
| Removed-current-sense note | `VOLTAGE_LIMIT(0.5V)` |

The `#define` in `config.h` is the only thing that actually takes effect, and
that file is missing. `config.h.example` uses **15 %** because that is the
value stated at the point where `VOLTAGE_LIMIT` is assigned to the driver.
**TODO(author): confirm which one you actually ran.**

### 2. Stale pin comment in `setup()`

The comment above the driver setup says `A=GPIO10 B=GPIO7 C=GPIO2 EN=GPIO8`,
but the file header explicitly documents that **B was moved from GPIO7 to
GPIO3** because GPIO7 is faulty on this board. The code uses `PIN_INHB`, so
behaviour follows `config.h` — only the comment is stale. The English
translation corrects the comment and notes the change.

### 3. `VELOCITY_LIMIT` comment vs. reality

The comment describes it as "the rate limit for internal angle advance in
open-loop angle mode". In SimpleFOC, `velocity_limit` in
`angle_openloop` mode caps how fast the *target* is approached; the actual
advance rate in `velocity_openloop` mode is set by `target_velocity`, not by
`velocity_limit`. Worth re-checking that the limit does what you expect.

---

## Behaviour you should expect

### Power-up

The motor **does not move**. `setup()` ends with:

```cpp
ctrl_mode = MODE_READ;   // encoder-only mode
uq_now = 0.0f;           // output voltage zero
motor.disable();         // driver de-energised
```

No alignment routine runs, no zero is hunted for, nothing is calibrated.
The board prints its parameters and waits.

### Why the encoder is not in the loop

This is intentional and stated repeatedly in the source. The firmware never
calls `motor.linkSensor(&sensor)` and never calls `motor.initFOC()`. The
sensor object exists only so `encDeg()` can print a number.

Consequence: **unplug the encoder and the motor still runs.** Plug it back
in and nothing changes either. If you want real closed loop, that work has
not been released yet (see [MISSING.md](MISSING.md)).

### The sensor's interface is PWM, read on the ADC, not I2C

The magnetic angle sensor's closed-loop angle output is a **PWM signal**:
duty cycle proportional to angle. The firmware reads it with the **ADC** on
GPIO0 (`PIN_ENC_ANGLE`). **There is no I2C transaction anywhere in this
firmware.** Do not wire SDA/SCL, and do not go looking for an I2C address —
you will find nothing, and the angle will never arrive.

This deserves emphasis because it is the single most common mistake people
make with magnetic angle sensors. Their datasheets lead with I2C (and SPI,
and ABI), so a builder wires SDA/SCL and then wonders why the firmware reads
a constant. On this board the sensor is configured for **PWM output**, and
the read path is a single ADC pin.

How a PWM signal becomes an angle with only an ADC:

1. `AnalogAngleSensor::readVoltage()` takes **8 ADC samples** and averages
   them (`analogReadMilliVolts`, 8×, divided by 8).
2. Sampling a PWM signal asynchronously and averaging converges toward its
   **duty cycle** — the average of many instantaneous captures of a 0/Vhigh
   square wave is `duty × Vhigh`.
3. That averaged value is then treated as a voltage and mapped linearly,
   `ANGLE_V_MIN … ANGLE_V_MAX` ↔ 0–360°.

So the 8-sample average is doing double duty: noise rejection *and* PWM
demodulation. It **is** the low-pass filter; there is no separate filtering
stage in software.

Practical consequences:

- **Resolution is limited by the averaging.** 8 samples of a PWM signal is a
  coarse duty estimate. If you need finer angle resolution, either increase
  the sample count, measure the PWM period/duty properly with a timer and
  input capture, or switch the sensor to a different output mode — all of
  which are firmware changes, not wiring changes.
- **Do not assume an RC low-pass on the board.** Whether the PCB filters the
  PWM into a true analog voltage before the ADC pin is not documented.
  **TODO(author): confirm** — it changes how many ADC samples are needed.
- **Scope it before debugging anything else.** If the angle reads a constant
  or jumps wildly, put a scope on GPIO0 first. You should see a square wave
  whose duty tracks the shaft. If you see a flat rail, the sensor is in the
  wrong output mode or not powered.

### Open-loop position is a fiction

`motor.shaft_angle` is an integrated counter, not a measurement. If the
motor stalls or loses steps, the counter keeps going and the reported
position silently drifts from reality. The only fix is a reboot, which
zeroes it.

### Soft start

Output voltage slews toward its target over `VOLTAGE_RAMP_MS` rather than
stepping. Set that macro to `0` to disable the ramp entirely.

### Arrival behaviour in position mode

When `|target − internal| < ARRIVE_TOL_RAD`, output drops to
`VOLTAGE_LIMIT × HOLD_VOLTAGE_RATIO`. This keeps a stationary motor from
cooking itself at full open-loop voltage. Both values are guesses right now.

---

## Serial protocol

> **⚠ The two firmware versions speak different protocols.**
>
> The original (`openloop-zh/开环代码.ino`) prefixes its output with Chinese
> mode names: `[位置环]`, `[速度环]`, `[磁编码器]`. The English version
> (`openloop/openloop.ino`) uses `[POS]`, `[VEL]`, `[ENC]`.
>
> These prefixes are a **protocol**, not decoration — a host program that
> demultiplexes on them will not understand the other language. If you have
> existing host software (上位机) parsing the Chinese prefixes, flash
> `openloop-zh/`, not the English version. If you are writing new host
> software, pick one and stick to it.
>
> Commands are identical in both versions (`D`, `R2`, `a 90`, `o`, `q`, bare
> numbers). Only the output prefixes differ.

115200 baud. Commands are line-terminated (`\n` or `\r`).

### Commands

| Send | Effect |
|---|---|
| `D` or `d` | Encoder mode — de-energise motor, read sensor only |
| `R2` or `r2` | Velocity mode — 2 rev/s (any number works, negative reverses) |
| `90` | Position mode — move internal angle to 90° |
| `a 90` | Position mode — same as above |
| `-45` | Position mode — negative angle accepted |
| `o` | Print one-off status |
| `q` | Print parameters and the live pin map |

Parsing detail: `a 90` is split as command char + `buf.substring(2)`, so
**the space after `a` is required**. `R2` has no space — the number is read
with `buf.substring(1)`. The two forms are inconsistent; that is how the
original works.

### Output

Every line is prefixed with the active mode so a host program can demux:

```
[POS] enc=12.34 deg target=90.0 deg internal=88.10 deg
[VEL] enc=200.11 deg target=2.00 rev/s
[ENC] enc=12.34 deg
```

Cadence: one live line every **50 ms** (20 Hz), one `status:` summary line
every **500 ms** (2 Hz). The status lines add `enabled`, output voltage, and
internal angle.

`q` also reprints the pin map. Useful for confirming you did not fry a pin —
particularly GPIO7.

---

## Flashing

1. Install the **Arduino IDE** and the **ESP32-C3 board package**.
2. Install **SimpleFOC 2.3.x** via Library Manager.
3. Copy `config.h.example` → `config.h`, correct the six `[GUESS]` values.
4. Open `openloop.ino`, select your ESP32-C3 board and the right port.
5. Flash. GPIO0 is used for the encoder ADC — on most C3 boards GPIO0 is also
   a strapping/boot pin, so **hold BOOT appropriately if the flash fails**.
6. Open the serial monitor at 115200. You should see the banner, the
   parameter line, the wiring line, then `[ENC]` lines. Send `a 90` to move.

> **TODO(author):** confirm the exact ESP32-C3 module variant, the Arduino
> ESP32 core version, and the SimpleFOC version you built against. Also
> confirm whether GPIO0 being a strapping pin caused you any flashing trouble.

---

## Safety

- Output is voltage-limited to 0.75 V (15 % of a 5 V bus). **Current sensing
  was deliberately removed** (it was on GPIO1), so that voltage ceiling is the
  *only* protection. Do not raise `VOLTAGE_LIMIT_RATIO` without understanding
  that there is no current trip underneath it.
- The motor will get warm in open loop, especially while holding position.
  `HOLD_VOLTAGE_RATIO` exists to limit this, but it is currently a guess.
- Open-loop drive can stall silently. Do not use this for anything that can
  pinch, lift, or injure.
