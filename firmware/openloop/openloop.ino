// =====================================================================
//  1503 brushless motor — pure open-loop drive
//  (ESP32-C3 + SimpleFOC + analog-voltage angle sensor)
//
//  English translation of the original `开环代码.ino`. Control logic is
//  byte-for-byte equivalent; only comments and serial strings changed.
//  The untouched original lives in `../openloop-zh/`.
//
//  ★ OPEN LOOP ★ The electrical angle is advanced purely by internal
//    integration inside this program. No feedback is used at all:
//    - No power-on calibration or alignment: no linkSensor(), no
//      initFOC(), no PID.
//    - ★ After power-up the motor STAYS STILL. It does not spin on its
//      own and does not hunt for a zero position. It only moves once a
//      serial command arrives.
//    - ★ The encoder takes NO part in control. It is read only to display
//      a number on the serial port (the ANG / status lines). Unplug the
//      encoder and the motor still follows commands. You only need it
//      back if you switch to closed loop.
//    - Rotation direction is set by the sign of the target plus
//      OPENLOOP_DIRECTION.
//    - Open-loop position is an internal counter, not a real position.
//      After a stall or a lost step the angle is wrong. If it drifts,
//      reboot (internal angle resets to 0) and re-send the angle command.
//
//  Wiring (editable in config.h):
//    Phase A -> INHA = GPIO10 , Phase B -> INHB = GPIO3 , Phase C -> INHC = GPIO2
//    EN = GPIO8     Encoder (analog voltage) ENC_ANGLE = GPIO0 (ADC)
//    (Phase B was originally GPIO7; that pin has a hardware fault, so it
//     was moved to GPIO3. All other wiring is unchanged.)
//
//  Output ceiling: POWER_SUPPLY_VOLTAGE * VOLTAGE_LIMIT_RATIO
//                  = 5 V * 15 % = 0.75 V
//                  (so the duty-cycle ceiling seen by the host is 15 %)
//
//  ★ Three modes, switched by serial command, each with its own prefix:
//    (1) Position  [POS]   a number, or `a 90` -> move the internal angle
//                          to 90 deg (sign gives direction); voltage drops
//                          once the target is reached
//    (2) Velocity  [VEL]   `R2` -> spin at 2 revolutions per second
//    (3) Encoder   [ENC]   `D`  -> motor de-energised, read encoder only
//                          (this is the default mode at power-up)
//    Position and velocity modes both advance open loop (electrical angle
//    integrated internally) with no feedback. Encoder mode only reads the
//    sensor and never drives the motor.
//
//  Serial commands (115200 baud):
//    D / d     -> switch to encoder mode
//    R2 / r2   -> switch to velocity mode, 2 rev/s
//    90 / a 90 -> switch to position mode, move to 90 deg
//    o / q     -> status / parameter query
//
//  Serial output (every line carries a mode prefix so the host can tell
//  the modes apart):
//    [POS] enc=xx.xx deg target=xx.x deg internal=xx.xx deg
//    [VEL] enc=xx.xx deg target=x.xx rev/s
//    [ENC] enc=xx.xx deg
//    One live line every 50 ms; one "status:" summary line every 500 ms.
// =====================================================================

#include <SimpleFOC.h>
#include "config.h"

// ========================= helpers =========================
// Normalise an angle difference into [-pi, pi] so motion takes the
// shortest path. SimpleFOC's own wrapper returns [0, 2pi), which is
// unusable here.
static float wrapAnglePi(float a) {
  float w = fmod(a + PI, _2PI);
  if (w < 0) w += _2PI;
  return w - PI;
}

// ========================= analog-voltage angle sensor =========================
// The encoder outputs an analog voltage with a fixed mapping:
// 0.0 V -> 0 deg, 3.3 V -> 360 deg.
// Read the voltage with the ADC and convert to an angle. 8 samples are
// averaged for noise rejection.
class AnalogAngleSensor : public Sensor {
  public:
    AnalogAngleSensor(uint8_t pin) : _pin(pin) {}

    // Configure the ADC pin (0 ~ 3.3 V range)
    void initADC() {
      analogSetPinAttenuation(_pin, ADC_11db);   // ADC range 0 ~ 3.3 V
      pinMode(_pin, INPUT);
    }

    // Read the current encoder voltage in volts, averaged over 8 samples
    float readVoltage() {
      int mv = 0;
      for (int i = 0; i < 8; i++) mv += analogReadMilliVolts(_pin);  // calibrated voltage, mV
      return (mv / 8) / 1000.0f;                                     // -> V
    }

    void init() override {
      initADC();
      this->Sensor::init();                      // smooth startup (prevents an initial jump)
      Serial.printf("Encoder: analog voltage -> angle (GPIO%d, %.2fV~%.2fV <-> 0~360 deg)\n",
                    _pin, ANGLE_V_MIN, ANGLE_V_MAX);
    }

    // Voltage -> angle (0 ~ 2pi rad), 0 ~ 3.3 V <-> 0 ~ 360 deg.
    // With ANGLE_MAP_INVERT=1 the mapping is reversed: 3.3 V -> 0 deg,
    // 0 V -> 360 deg (use when the motor runs the wrong way).
    float getSensorAngle() override {
      float v = readVoltage();
      float frac = (v - ANGLE_V_MIN) / (ANGLE_V_MAX - ANGLE_V_MIN);   // 0 ~ 1
      if (frac < 0.0f) frac = 0.0f;
      if (frac > 1.0f) frac = 1.0f;
  #if ANGLE_MAP_INVERT
      frac = 1.0f - frac;   // inverted mapping
  #endif
      return frac * _2PI;
    }
    float getAngle() override { return getSensorAngle(); }

  private:
    uint8_t _pin;
};

// ========================= hardware objects =========================
// ★ Encoder: connected to the ADC purely to read a value for display.
//   NEVER wire it into the control loop.
//   (Do not call motor.linkSensor(&sensor), and do not call motor.initFOC().)
//   The three motor phases are driven open loop straight from the driver,
//   independent of the encoder.
AnalogAngleSensor sensor(PIN_ENC_ANGLE);
BLDCDriver3PWM driver(PIN_INHA, PIN_INHB, PIN_INHC, PIN_EN);
BLDCMotor motor(MOTOR_POLE_PAIRS);

// ========================= global state (pure open loop) =========================
float target_angle_deg = 0.0f;   // target angle in degrees, set by the user, display only
float target_angle = 0.0f;       // open-loop internal target angle, rad, already multiplied by OPENLOOP_DIRECTION
float target_velocity = 0.0f;    // open-loop target velocity, rad/s, used by R mode

// Mode: 0 = angle open loop, 1 = velocity open loop (R), 2 = read only (D)
enum CtrlMode { MODE_ANGLE, MODE_VELOCITY, MODE_READ };
CtrlMode ctrl_mode = MODE_ANGLE;

// Soft start: the output voltage actually applied to the motor right now,
// in volts. Ramps from 0 toward the target over VOLTAGE_RAMP_MS.
float uq_now = 0.0f;

// Mode names, used as the serial-output prefix:
//   (1) POS = MODE_ANGLE     (a number, or `a 90`)
//   (2) VEL = MODE_VELOCITY  (`R2`)
//   (3) ENC = MODE_READ      (`D`: motor de-energised, encoder read only)
//   Note: POS and VEL are both open-loop implementations (the electrical
//   angle is advanced by internal integration), with no feedback.
static const char* modeName() {
  if (ctrl_mode == MODE_VELOCITY) return "VEL";
  if (ctrl_mode == MODE_READ)     return "ENC";
  return "POS";
}

// Encoder / internal angle in degrees, for printing
static float encDeg()   { return sensor.getMechanicalAngle() * 180.0f / PI; }
static float innerDeg() { return motor.shaft_angle * 180.0f / PI; }

// Set the open-loop angle target. Input: user angle in degrees.
// Open loop has no absolute position, so wrap the current internal angle
// into [-pi, pi] and map the target onto the nearest revolution. This both
// guarantees shortest-path motion and stops the internal angle growing
// without bound, which would degrade floating-point precision.
static void applyAngleTarget(float deg) {
  ctrl_mode = MODE_ANGLE;
  target_angle_deg = deg;
  float tgt = deg * PI / 180.0f * OPENLOOP_DIRECTION;      // user degrees -> rad (sign included)
  motor.shaft_angle = wrapAnglePi(motor.shaft_angle);      // wrap the internal angle
  target_angle = motor.shaft_angle + wrapAnglePi(tgt - motor.shaft_angle);
  motor.controller = MotionControlType::angle_openloop;    // open-loop angle
  motor.enable();
  Serial.printf("[POS] target %.1f deg\n", deg);
}

// ========================= serial output (★ every line carries a mode prefix) =========================
//   POS : [POS] enc=xx.xx deg target=xx.x deg internal=xx.xx deg
//   VEL : [VEL] enc=xx.xx deg target=x.xx rev/s
//   ENC : [ENC] enc=xx.xx deg
//   One live line every 50 ms, one full status line every 500 ms.
void printFeedback() {
  static unsigned long last_fast = 0;
  static unsigned long last_slow = 0;
  unsigned long now = millis();

  // ---- 20 Hz live feedback ----
  if (now - last_fast > 50) {
    last_fast = now;
    switch (ctrl_mode) {
      case MODE_VELOCITY:
        Serial.printf("[VEL] enc=%.2f deg target=%.2f rev/s\n", encDeg(), target_velocity / _2PI);
        break;
      case MODE_READ:
        Serial.printf("[ENC] enc=%.2f deg\n", encDeg());
        break;
      default:
        Serial.printf("[POS] enc=%.2f deg target=%.1f deg internal=%.2f deg\n",
                      encDeg(), target_angle_deg, innerDeg());
        break;
    }
  }

  // ---- 2 Hz full status ----
  if (now - last_slow > 500) {
    last_slow = now;
    switch (ctrl_mode) {
      case MODE_VELOCITY:
        Serial.printf("[VEL] status: enabled=%d output=%.2fV target=%.2f rev/s internal=%.2f deg\n",
                      motor.enabled, uq_now, target_velocity / _2PI, innerDeg());
        break;
      case MODE_READ:
        Serial.printf("[ENC] status: motor de-energised enabled=%d output=%.2fV\n", motor.enabled, uq_now);
        break;
      default:
        Serial.printf("[POS] status: enabled=%d output=%.2fV target=%.1f deg internal=%.2f deg\n",
                      motor.enabled, uq_now, target_angle_deg, innerDeg());
        break;
    }
  }
}

// ========================= total current sense (three phases summed) — REMOVED =========================
//  Current sampling on GPIO1 was removed at the author's request. The
//  driver relies on the VOLTAGE_LIMIT ceiling for protection instead.

// ========================= serial command parsing =========================
void handleSerial() {
  static String buf = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      buf.trim();
      if (buf.length() > 0) {
        char cmd = buf[0];
        String arg = buf.length() > 2 ? buf.substring(2) : "";
        float val = arg.toFloat();

        if (cmd == 'R' || cmd == 'r') {        // R2 -> velocity mode: 2 rev/s (advanced open loop)
          float rps = buf.substring(1).toFloat();
          ctrl_mode = MODE_VELOCITY;
          target_velocity = rps * _2PI * OPENLOOP_DIRECTION;   // rev/s -> rad/s (sign included)
          motor.controller = MotionControlType::velocity_openloop;  // open-loop velocity
          motor.enable();
          Serial.printf("[VEL] engaged: target %.2f rev/s (%.2f rad/s)\n",
                        target_velocity / _2PI, target_velocity);
        }
        else if (cmd == 'D' || cmd == 'd') {   // D -> encoder mode: read only, motor de-energised
          ctrl_mode = MODE_READ;
          uq_now = 0.0f;                       // output voltage to zero
          motor.disable();                     // de-energise, do not drive
          Serial.printf("[ENC] engaged: motor de-energised, reading encoder angle only\n");
        }
        else if (cmd == 'a') {                 // a 90 -> position-mode target angle in degrees
          applyAngleTarget(val);
        }
        else if (cmd == 'o') {
          Serial.printf("[%s] status: enc=%.1f deg (display only) internal=%.1f deg enabled=%d output=%.2fV\n",
            modeName(), encDeg(), innerDeg(), motor.enabled, uq_now);
        }
        else if (cmd == 'q') {
          Serial.printf("[%s] params: open loop | pole pairs=%d bus=%.1fV ceiling=%.2fV(%.0f%%) soft start=%dms direction=%d encoder=display only, not in control loop\n",
            modeName(), motor.pole_pairs, POWER_SUPPLY_VOLTAGE,
            VOLTAGE_LIMIT, VOLTAGE_LIMIT_RATIO * 100.0f, VOLTAGE_RAMP_MS,
            OPENLOOP_DIRECTION);
          // Wiring info, prefixed with the mode, so you can always
          // double-check which pin is which.
          Serial.printf("[%s] wiring: A(INHA)=GPIO%d B(INHB)=GPIO%d C(INHC)=GPIO%d EN=GPIO%d encoder=GPIO%d\n",
            modeName(), PIN_INHA, PIN_INHB, PIN_INHC, PIN_EN, PIN_ENC_ANGLE);
        }
        else if (isdigit(cmd) || cmd == '-' || cmd == '.') {
          // A bare number = position-mode target angle in degrees.
          // e.g. typing 90 moves the internal angle to 90 deg.
          applyAngleTarget(buf.toFloat());
        }
      }
      buf = "";
    } else {
      buf += c;
    }
  }
}

// ========================= initialisation =========================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n===== 1503 pure open-loop drive, starting =====");
  Serial.printf("pole pairs %d | bus %.1fV | ceiling %.2fV (%.0f%%) | soft start %dms\n",
                MOTOR_POLE_PAIRS, POWER_SUPPLY_VOLTAGE, VOLTAGE_LIMIT,
                VOLTAGE_LIMIT_RATIO * 100.0f, VOLTAGE_RAMP_MS);
  Serial.printf("wiring: A=GPIO%d B=GPIO%d C=GPIO%d EN=GPIO%d | encoder read GPIO%d (display only, not in control loop)\n",
                PIN_INHA, PIN_INHB, PIN_INHC, PIN_EN, PIN_ENC_ANGLE);

  // Encoder (analog voltage): configure the ADC then initialise directly,
  // with a fixed 0 ~ 3.3 V <-> 0 ~ 360 deg mapping.
  // ★ Read/display only: no linkSensor(), no part in commutation or control.
  sensor.initADC();
  sensor.init();

  // Driver, per the real wiring: A=GPIO10 B=GPIO3 C=GPIO2 EN=GPIO8
  // (An older comment in the original source said B=GPIO7; that pin is
  //  faulty on this board and B was moved to GPIO3.)
  driver.voltage_power_supply = POWER_SUPPLY_VOLTAGE;
  driver.voltage_limit = VOLTAGE_LIMIT;     // = bus * 15 %
  driver.pwm_frequency = 20000;
  driver.init();
  motor.linkDriver(&driver);

  // ★ Pure open loop: no linkSensor(), no initFOC().
  //   electrical angle = _electricalAngle(internal shaft_angle, pole pairs),
  //   advanced entirely by this program.
  motor.controller = MotionControlType::angle_openloop;
  motor.torque_controller = TorqueControlType::voltage;
  motor.voltage_limit = VOLTAGE_LIMIT;      // open-loop output ceiling (the soft-start ramp rewrites it every tick)
  motor.velocity_limit = VELOCITY_LIMIT;    // rate limit for internal angle advance in open-loop angle mode
  // ★ No power-on calibration or alignment (initFOC is not called); only
  //   the hardware is initialised.
  motor.init();

  // ★ Power up stationary: driver de-energised and standing by, internal
  //   angle zeroed, no target issued automatically.
  //   To move, send a serial command: number/`a` = angle, `R` = speed,
  //   `D` = back to de-energised read-only.
  ctrl_mode = MODE_READ;
  uq_now = 0.0f;
  target_angle_deg = 0.0f;
  target_angle = 0.0f;
  motor.disable();

  Serial.printf("Pure open loop ready (stationary at power-up, waiting for serial commands). number/a = angle, R = speed, D = de-energised read\n");
}

// ========================= main loop (pure open loop, output voltage capped) =========================
void loop() {
  // ---- elapsed time this tick, in seconds, used by the soft-start ramp ----
  static unsigned long last_ms = 0;
  unsigned long now = millis();
  float dt = (now - last_ms) * 0.001f;
  last_ms = now;
  if (dt <= 0.0f || dt > 0.5f) dt = 0.001f;

  // ---- 1) desired output voltage this tick ----
  float u_target = 0.0f;
  if (ctrl_mode == MODE_VELOCITY) {
    u_target = VOLTAGE_LIMIT;                       // open-loop velocity: full ceiling
  } else if (ctrl_mode == MODE_ANGLE) {
    // Open-loop angle: full ceiling while moving, drop once arrived
    if (fabs(wrapAnglePi(target_angle - motor.shaft_angle)) < ARRIVE_TOL_RAD) {
      u_target = VOLTAGE_LIMIT * HOLD_VOLTAGE_RATIO;   // arrived: reduced voltage
    } else {
      u_target = VOLTAGE_LIMIT;                        // moving: full ceiling
    }
  } else {
    u_target = 0.0f;                                // encoder (D) mode: de-energised
  }

  // ---- 2) soft start: ramp the output voltage toward the target ----
#if VOLTAGE_RAMP_MS > 0
  const float u_step = VOLTAGE_LIMIT * dt * 1000.0f / (float)VOLTAGE_RAMP_MS;
  if (uq_now < u_target) {
    uq_now += u_step;
    if (uq_now > u_target) uq_now = u_target;
  } else if (uq_now > u_target) {
    uq_now -= u_step;
    if (uq_now < u_target) uq_now = u_target;
  }
  if (uq_now < 0.0f) uq_now = 0.0f;
#else
  uq_now = u_target;
#endif
  motor.voltage_limit = uq_now;                     // output ceiling for this tick

  // ---- 3) advance open loop (electrical angle integrated internally, no feedback) ----
  if (ctrl_mode == MODE_VELOCITY) {
    motor.loopFOC();
    motor.move(target_velocity);      // velocity_openloop: internal angle advanced at target speed
  } else if (ctrl_mode == MODE_ANGLE) {
    motor.loopFOC();
    motor.move(target_angle);         // angle_openloop: internal angle advanced toward target
  }
  // Encoder mode: already disable()d by the command handler, and neither
  // loopFOC() nor move() is called, so PWM stays at 0.

  // ---- 4) encoder read only (★ display only, never used for control) ----
  sensor.update();

  handleSerial();

  // ---- serial output (live every 50 ms / status every 500 ms, both prefixed) ----
  printFeedback();
}
