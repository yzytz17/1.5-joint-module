// =====================================================================
//  1503 无刷电机 纯开环驱动 (ESP32-C3 + SimpleFOC + 模拟电压编码器)
//
//  ★ 开环 ★ 电角度完全由程序内部积分推进, 不使用任何反馈:
//    - 不做任何上电校正/对齐: 不 linkSensor、不执行 initFOC()、无 PID
//    - ★ 上电后电机【保持静止】, 不会自己转、不会自己找零位, 等串口命令才动
//    - ★ 编码器【不参与实际控制】: 只用于串口读数/显示(ANG/状态行),
//         拔掉编码器电机照样按命令转动 (要改闭环才需要接回来)
//    - 转动方向由目标正负 + OPENLOOP_DIRECTION 决定
//    - 开环的位置是“内部计数值”不是真实位置: 堵转/丢步后角度会失准,
//      觉得不准就重启(内部角度归 0)后重新发角度命令
//
//  接线 (config.h 可改):
//    A 相 → INHA = GPIO10 , B 相 → INHB = GPIO3 , C 相 → INHC = GPIO2
//    EN = GPIO8     编码器(模拟电压) ENC_ANGLE = GPIO0(ADC)
//    (B 相原为 GPIO7, 该脚硬件有问题 → 换成 GPIO3, 其余接线不变)
//
//  输出上限: POWER_SUPPLY_VOLTAGE × VOLTAGE_LIMIT_RATIO = 5V × 15% = 0.75V
//             (上位机看到的占空比上限就是 15%)
//
//  ★ 三种模式 (串口命令切换) 与反馈前缀:
//    ① 位置环   [位置环]   数字 或 a 90 → 内部角度转到 90° (含方向), 到位降压
//    ② 速度环   [速度环]   R2        → 以 2 圈/秒 旋转
//    ③ 磁编码器 [磁编码器] D         → 电机断电, 只读编码器角度 (上电默认此模式)
//    位置环/速度环当前均为开环推进(电角度内部积分), 不带反馈;
//    磁编码器模式只读编码器, 不驱动电机。
//
//  串口命令 (115200):
//    D / d     → 切磁编码器模式
//    R2 / r2   → 切速度环, 每秒 2 圈
//    90 / a 90 → 切位置环, 转到 90°
//    o / q     → 状态 / 参数查询
//
//  串口反馈 (每行都带模式前缀, 便于上位机区分):
//    [位置环]   编码器=xx.xx° 目标=xx.x° 内部=xx.xx°
//    [速度环]   编码器=xx.xx° 目标=x.xx圈/秒
//    [磁编码器] 编码器=xx.xx°
//    每 50ms 一行实时反馈; 每 500ms 一行 "状态:" 汇总
// =====================================================================

#include <SimpleFOC.h>
#include "config.h"

// ========================= 工具函数 =========================
// 把角度差归一化到 [-π, π], 实现"最短路径" (SimpleFOC 自带的是 [0,2π), 不能用)
static float wrapAnglePi(float a) {
  float w = fmod(a + PI, _2PI);
  if (w < 0) w += _2PI;
  return w - PI;
}

// ========================= 模拟电压编码器传感器 =========================
// 编码器输出模拟电压, 固定映射: 0.0V → 0°, 3.3V → 360°
// 用 ADC 读取电压 → 换算角度, 8 次采样平均抗噪
class AnalogAngleSensor : public Sensor {
  public:
    AnalogAngleSensor(uint8_t pin) : _pin(pin) {}

    // 配置 ADC 引脚 (量程 0~3.3V)
    void initADC() {
      analogSetPinAttenuation(_pin, ADC_11db);   // ADC 量程 0~3.3V
      pinMode(_pin, INPUT);
    }

    // 读取当前编码器电压 (V), 8 次平均抗噪
    float readVoltage() {
      int mv = 0;
      for (int i = 0; i < 8; i++) mv += analogReadMilliVolts(_pin);  // 校准电压(mV)
      return (mv / 8) / 1000.0f;                                     // → V
    }

    void init() override {
      initADC();
      this->Sensor::init();                      // 平滑启动 (防初始跳变)
      Serial.printf("编码器: 模拟电压→角度 (GPIO%d, %.2fV~%.2fV ↔ 0~360°)\n",
                    _pin, ANGLE_V_MIN, ANGLE_V_MAX);
    }

    // 电压 → 角度 (0 ~ 2π rad), 0~3.3V ↔ 0~360°
    // ANGLE_MAP_INVERT=1 时反向: 3.3V→0°, 0V→360° (电机往反方向跑时用)
    float getSensorAngle() override {
      float v = readVoltage();
      float frac = (v - ANGLE_V_MIN) / (ANGLE_V_MAX - ANGLE_V_MIN);   // 0~1
      if (frac < 0.0f) frac = 0.0f;
      if (frac > 1.0f) frac = 1.0f;
  #if ANGLE_MAP_INVERT
      frac = 1.0f - frac;   // 反向映射
  #endif
      return frac * _2PI;
    }
    float getAngle() override { return getSensorAngle(); }

  private:
    uint8_t _pin;
};

// ========================= 硬件对象 =========================
// ★ 编码器: 只接在 ADC 上读值显示, 绝不要接进控制回路
//   (不要写 motor.linkSensor(&sensor), 也不要调 motor.initFOC())
//   电机三相由 driver 直接开环驱动, 与编码器无关
AnalogAngleSensor sensor(PIN_ENC_ANGLE);
BLDCDriver3PWM driver(PIN_INHA, PIN_INHB, PIN_INHC, PIN_EN);
BLDCMotor motor(MOTOR_POLE_PAIRS);

// ========================= 全局状态 (纯开环) =========================
float target_angle_deg = 0.0f;   // 目标角度 (度, 用户设定, 仅显示用)
float target_angle = 0.0f;       // 开环内部目标角度 (rad, 已乘 OPENLOOP_DIRECTION)
float target_velocity = 0.0f;    // 开环目标速度 (rad/s), R 模式用

// 模式: 0=角度开环, 1=速度开环(R), 2=读取(D) —— 三模式并列
enum CtrlMode { MODE_ANGLE, MODE_VELOCITY, MODE_READ };
CtrlMode ctrl_mode = MODE_ANGLE;

// 软启动: 当前实际加到电机的输出电压 (V), 从 0 按 VOLTAGE_RAMP_MS 斜坡爬到目标值
float uq_now = 0.0f;

// 模式名 (串口反馈前缀用): 位置环 / 速度环 / 磁编码器
//   ① 位置环   = MODE_ANGLE    (数字 或 a 90)
//   ② 速度环   = MODE_VELOCITY (R2)
//   ③ 磁编码器 = MODE_READ     (D: 电机断电, 只读编码器)
//   注: 位置/速度环当前均为开环实现(电角度内部积分推进), 不带反馈
static const char* modeName() {
  if (ctrl_mode == MODE_VELOCITY) return "速度环";
  if (ctrl_mode == MODE_READ)     return "磁编码器";
  return "位置环";
}

// 编码器/内部角度 (度), 方便打印
static float encDeg()   { return sensor.getMechanicalAngle() * 180.0f / PI; }
static float innerDeg() { return motor.shaft_angle * 180.0f / PI; }

// 设置开环角度目标 (输入: 用户角度, 度)
// 开环没有绝对位置, 把当前内部角度卷绕到 [-π,π] 并把目标映射到最近一圈,
// 既保证“按最短路径走”, 又避免内部角度值无限增长导致浮点精度下降
static void applyAngleTarget(float deg) {
  ctrl_mode = MODE_ANGLE;
  target_angle_deg = deg;
  float tgt = deg * PI / 180.0f * OPENLOOP_DIRECTION;      // 用户角度 → rad (含方向)
  motor.shaft_angle = wrapAnglePi(motor.shaft_angle);      // 内部角度卷绕
  target_angle = motor.shaft_angle + wrapAnglePi(tgt - motor.shaft_angle);
  motor.controller = MotionControlType::angle_openloop;    // 开环角度
  motor.enable();
  Serial.printf("[位置环] 目标 %.1f°\n", deg);
}

// ========================= 串口反馈 (★ 每行都带模式前缀) =========================
//   位置环  : [位置环] 编码器=xx.xx° 目标=xx.x° 内部=xx.xx°
//   速度环  : [速度环] 编码器=xx.xx° 目标=x.xx圈/秒
//   磁编码器: [磁编码器] 编码器=xx.xx°
//   每 50ms 一行实时反馈, 每 500ms 一行完整状态
void printFeedback() {
  static unsigned long last_fast = 0;
  static unsigned long last_slow = 0;
  unsigned long now = millis();

  // ---- 20Hz 实时反馈 ----
  if (now - last_fast > 50) {
    last_fast = now;
    switch (ctrl_mode) {
      case MODE_VELOCITY:
        Serial.printf("[速度环] 编码器=%.2f° 目标=%.2f圈/秒\n", encDeg(), target_velocity / _2PI);
        break;
      case MODE_READ:
        Serial.printf("[磁编码器] 编码器=%.2f°\n", encDeg());
        break;
      default:
        Serial.printf("[位置环] 编码器=%.2f° 目标=%.1f° 内部=%.2f°\n",
                      encDeg(), target_angle_deg, innerDeg());
        break;
    }
  }

  // ---- 2Hz 完整状态 ----
  if (now - last_slow > 500) {
    last_slow = now;
    switch (ctrl_mode) {
      case MODE_VELOCITY:
        Serial.printf("[速度环] 状态: 使能=%d 输出=%.2fV 目标=%.2f圈/秒 内部=%.2f°\n",
                      motor.enabled, uq_now, target_velocity / _2PI, innerDeg());
        break;
      case MODE_READ:
        Serial.printf("[磁编码器] 状态: 电机断电 使能=%d 输出=%.2fV\n", motor.enabled, uq_now);
        break;
      default:
        Serial.printf("[位置环] 状态: 使能=%d 输出=%.2fV 目标=%.1f° 内部=%.2f°\n",
                      motor.enabled, uq_now, target_angle_deg, innerDeg());
        break;
    }
  }
}

// ========================= 总电流读取 (三相合并) —— 已删除 =========================
//  用户要求移除电流采样(GPIO1), 驱动靠 VOLTAGE_LIMIT(0.5V) 电压上限保护

// ========================= 串口命令解析 =========================
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

        if (cmd == 'R' || cmd == 'r') {        // R2 → 速度环: 每秒 2 圈 (开环推进)
          float rps = buf.substring(1).toFloat();
          ctrl_mode = MODE_VELOCITY;
          target_velocity = rps * _2PI * OPENLOOP_DIRECTION;   // 圈/秒 → rad/s (含方向)
          motor.controller = MotionControlType::velocity_openloop;  // 速度开环
          motor.enable();
          Serial.printf("[速度环] 已切入: 目标 %.2f 圈/秒 (%.2f rad/s)\n",
                        target_velocity / _2PI, target_velocity);
        }
        else if (cmd == 'D' || cmd == 'd') {   // D → 磁编码器模式: 只读编码器, 电机断电
          ctrl_mode = MODE_READ;
          uq_now = 0.0f;                       // 输出电压归零
          motor.disable();                     // 断电, 不驱动
          Serial.printf("[磁编码器] 已切入: 电机断电, 只读编码器角度\n");
        }
        else if (cmd == 'a') {                 // a 90 → 位置环目标角度 (度)
          applyAngleTarget(val);
        }
        else if (cmd == 'o') {
          Serial.printf("[%s] 状态: 编码器=%.1f°(仅显示) 内部角度=%.1f° 使能=%d 输出=%.2fV\n",
            modeName(), encDeg(), innerDeg(), motor.enabled, uq_now);
        }
        else if (cmd == 'q') {
          Serial.printf("[%s] 参数: 开环 极对数=%d 母线=%.1fV 输出上限=%.2fV(%.0f%%) 软启动=%dms 方向=%d 编码器=仅显示不参与控制\n",
            modeName(), motor.pole_pairs, POWER_SUPPLY_VOLTAGE,
            VOLTAGE_LIMIT, VOLTAGE_LIMIT_RATIO * 100.0f, VOLTAGE_RAMP_MS,
            OPENLOOP_DIRECTION);
          // 接线信息 (带模式前缀), 方便随时核对引脚有没有烧对
          Serial.printf("[%s] 接线: A(INHA)=GPIO%d B(INHB)=GPIO%d C(INHC)=GPIO%d EN=GPIO%d 编码器=GPIO%d\n",
            modeName(), PIN_INHA, PIN_INHB, PIN_INHC, PIN_EN, PIN_ENC_ANGLE);
        }
        else if (isdigit(cmd) || cmd == '-' || cmd == '.') {
          // 直接输入数字 = 位置环目标角度 (度), 如输入 90 → 内部角度转到 90°
          applyAngleTarget(buf.toFloat());
        }
      }
      buf = "";
    } else {
      buf += c;
    }
  }
}

// ========================= 初始化 =========================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n===== 1503 纯开环驱动 启动 =====");
  Serial.printf("极对数 %d | 母线 %.1fV | 输出上限 %.2fV (%.0f%%) | 软启动 %dms\n",
                MOTOR_POLE_PAIRS, POWER_SUPPLY_VOLTAGE, VOLTAGE_LIMIT,
                VOLTAGE_LIMIT_RATIO * 100.0f, VOLTAGE_RAMP_MS);
  Serial.printf("接线: A=GPIO%d B=GPIO%d C=GPIO%d EN=GPIO%d | 编码器读数 GPIO%d(仅显示, 不参与控制)\n",
                PIN_INHA, PIN_INHB, PIN_INHC, PIN_EN, PIN_ENC_ANGLE);

  // 编码器 (模拟电压): 配置 ADC 后直接初始化, 固定 0~3.3V ↔ 0~360° 映射
  // ★ 仅用于读取/显示: 不 linkSensor、不参与换相与控制
  sensor.initADC();
  sensor.init();

  // 驱动 (按实际接线: A=GPIO10 B=GPIO7 C=GPIO2 EN=GPIO8)
  driver.voltage_power_supply = POWER_SUPPLY_VOLTAGE;
  driver.voltage_limit = VOLTAGE_LIMIT;     // = 母线 × 15%
  driver.pwm_frequency = 20000;
  driver.init();
  motor.linkDriver(&driver);

  // ★ 纯开环: 不 linkSensor、不调 initFOC()
  //   电角度 = _electricalAngle(内部 shaft_angle, 极对数), 完全由程序自己推进
  motor.controller = MotionControlType::angle_openloop;
  motor.torque_controller = TorqueControlType::voltage;
  motor.voltage_limit = VOLTAGE_LIMIT;      // 开环输出电压上限 (Loop 里会按软启动斜坡改)
  motor.velocity_limit = VELOCITY_LIMIT;    // 开环角度模式下内部角度的推进速度上限
  // ★ 不做任何上电校正/对齐 (不调 initFOC), 只初始化硬件
  motor.init();

  // ★ 上电保持静止: 驱动器断电待命, 内部角度归 0, 不自动发任何目标
  //   要动才发串口命令: 数字/a 角度、R 转速、D 回到断电读取
  ctrl_mode = MODE_READ;
  uq_now = 0.0f;
  target_angle_deg = 0.0f;
  target_angle = 0.0f;
  motor.disable();

  Serial.printf("纯开环就绪 (上电静止, 等串口命令)! 数字/a=角度 R=转速 D=断电读取\n");
}

// ========================= 主循环 (纯开环, 输出电压限 10%) =========================
void loop() {
  // ---- 本拍时间 (s), 供软启动斜坡用 ----
  static unsigned long last_ms = 0;
  unsigned long now = millis();
  float dt = (now - last_ms) * 0.001f;
  last_ms = now;
  if (dt <= 0.0f || dt > 0.5f) dt = 0.001f;

  // ---- 1) 本拍期望输出电压 ----
  float u_target = 0.0f;
  if (ctrl_mode == MODE_VELOCITY) {
    u_target = VOLTAGE_LIMIT;                       // 速度开环: 满压(10%)
  } else if (ctrl_mode == MODE_ANGLE) {
    // 角度开环: 到位降压保持, 运动中满压
    if (fabs(wrapAnglePi(target_angle - motor.shaft_angle)) < ARRIVE_TOL_RAD) {
      u_target = VOLTAGE_LIMIT * HOLD_VOLTAGE_RATIO;   // 到位: 降压
    } else {
      u_target = VOLTAGE_LIMIT;                        // 运动: 满压
    }
  } else {
    u_target = 0.0f;                                // 读取(D)模式: 断电
  }

  // ---- 2) 软启动: 输出电压朝目标值斜坡变化 ----
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
  motor.voltage_limit = uq_now;                     // 本拍开环输出电压上限

  // ---- 3) 开环推进 (电角度由内部积分得到, 不用反馈) ----
  if (ctrl_mode == MODE_VELOCITY) {
    motor.loopFOC();
    motor.move(target_velocity);      // velocity_openloop: 内部角度按目标速度推进
  } else if (ctrl_mode == MODE_ANGLE) {
    motor.loopFOC();
    motor.move(target_angle);         // angle_openloop: 内部角度朝目标推进
  }
  // 读取模式: 已在命令里 disable() 且不调 loopFOC/move, PWM 保持 0

  // ---- 4) 编码器只读 (★ 仅用于显示, 不参与任何控制) ----
  sensor.update();

  handleSerial();

  // ---- 串口反馈 (每 50ms 实时 / 每 500ms 状态, 均带模式前缀) ----
  printFeedback();
}
