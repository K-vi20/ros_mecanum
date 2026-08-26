#include <ArduinoHardware.h>
#include "CytronMotorDriver.h"

#include <ros.h>
#include <math.h>

#include <geometry_msgs/Twist.h>
#include <std_msgs/Float32MultiArray.h>
#include <Encoder.h>

// ============================================================
// BATLYBOT — Mecanum Controller + PID Tuning
// Teensy 4.0 / Cytron MDD3A
//
// Subscribe:
//   /cmd_vel      geometry_msgs/Twist
//   /pid_gains    std_msgs/Float32MultiArray [kp, ki, kd]
//
// Publish:
//   /raw_vel      Robot Vx, Vy, Wz
//   /encoder_rpm  FL, FR, RL, RR RPM
// ============================================================

// ---------------- System configuration ----------------------
#define CONTROL_PERIOD_MS   50
#define COMMAND_TIMEOUT_MS  400

// ---------------- Robot physical parameters -------------------
const float WHEEL_RADIUS = 0.045f;  //รัศมีล้อ 
const float LX = 0.2100f;
const float LY = 0.1475f;
const float KINEMATIC_L = LX + LY;

// ---------------- Encoders (FL, FR, RL, RR) --------------------
Encoder Enc_A(2,  3);     // FL
Encoder Enc_B(22, 23);    // FR
Encoder Enc_C(14, 15);    // RL
Encoder Enc_D(21, 20);    // RR

const float PPR_FL = 5841.0f;
const float PPR_FR = 5854.0f;
const float PPR_RL = 5755.0f;
const float PPR_RR = 5815.0f;

// ---------------- Motors (FL, FR, RL, RR) -----------------------
CytronMD motor1(PWM_PWM, 4,  5);   // FL
CytronMD motor2(PWM_PWM, 10, 11);  // FR
CytronMD motor3(PWM_PWM, 8,  9);   // RL
CytronMD motor4(PWM_PWM, 7,  6);   // RR

// ---------------- PID gains (tunable via /pid_gains) ------------
volatile float KP = 70.0f;
volatile float KI = 5.0f;
volatile float KD = 1.0f;

struct PIDController {
  float integral;
  float previousError;
};

PIDController pid1 = {0.0f, 0.0f};  // FL
PIDController pid2 = {0.0f, 0.0f};  // FR
PIDController pid3 = {0.0f, 0.0f};  // RL
PIDController pid4 = {0.0f, 0.0f};  // RR

// ---------------- Velocity state ---------------------------------

// Target wheel speed [rad/s]
float w1 = 0.0f;
float w2 = 0.0f;
float w3 = 0.0f;
float w4 = 0.0f;

// Actual wheel speed [rad/s]
float vel1 = 0.0f;
float vel2 = 0.0f;
float vel3 = 0.0f;
float vel4 = 0.0f;

// Robot velocity
float Vx = 0.0f;
float Vy = 0.0f;
float Wz = 0.0f;

// Encoder RPM
float rpmFL = 0.0f;
float rpmFR = 0.0f;
float rpmRL = 0.0f;
float rpmRR = 0.0f;

unsigned long prev_control_time = 0;
unsigned long prev_command_time = 0;

// ---------------- Function prototypes -----------------------------
void commandCallback(const geometry_msgs::Twist &cmd_msg);
void pidGainsCallback(const std_msgs::Float32MultiArray &msg);
void movebase(float dt);
void encoder();
void encoderRPM();
void stopBase();
float pulse_to_radps(long pulse, float ppr, float dt);
float pulse_to_rpm(long pulse, float ppr, float dt);
int  PID_control(float currentVel, float targetVel, PIDController &pid, float dt);
void resetPID(PIDController &pid);
void resetAllPID();

// ---------------- ROS node, topics --------------------------------
ros::NodeHandle nh;

geometry_msgs::Twist enc;
ros::Publisher Enc_pub("raw_vel", &enc);

std_msgs::Float32MultiArray rpm_msg;
float rpm_data[4];                         // [FL, FR, RL, RR]
ros::Publisher RPM_pub("encoder_rpm", &rpm_msg);

ros::Subscriber<geometry_msgs::Twist> cmd_sub("/cmd_vel", commandCallback);
ros::Subscriber<std_msgs::Float32MultiArray> pid_sub("/pid_gains", pidGainsCallback);

// ============================================================
// SETUP
// ============================================================
void setup() {
  nh.getHardware()->setBaud(57600);
  nh.initNode();

  nh.subscribe(cmd_sub);
  nh.subscribe(pid_sub);
  nh.advertise(Enc_pub);
  nh.advertise(RPM_pub);

  rpm_msg.data_length = 4;
  rpm_msg.data = rpm_data;

  while (!nh.connected()) {
    nh.spinOnce();
  }

  stopBase();

  nh.loginfo("================================");
  nh.loginfo("BATLYBOT CONNECTED");
  nh.loginfo("MECANUM PID CONTROLLER");
  nh.loginfo("Encoder RPM Publisher READY");
  nh.loginfo("================================");

  prev_control_time = millis();
  prev_command_time = millis();
}

// ============================================================
// MAIN LOOP
// ============================================================
void loop() {
  unsigned long now = millis();

  // Control loop @ 1000/CONTROL_PERIOD_MS Hz
  if (now - prev_control_time >= CONTROL_PERIOD_MS) {
    float dt = (now - prev_control_time) / 1000.0f;
    prev_control_time = now;

    movebase(dt);   // 1. read encoders + run PID
    encoder();      // 2. compute + publish robot odometry
    encoderRPM();   // 3. compute + publish wheel RPM
  }

  // Safety stop if no /cmd_vel received recently
  if (now - prev_command_time >= COMMAND_TIMEOUT_MS) {
    stopBase();
  }

  nh.spinOnce();
}

// ============================================================
// /cmd_vel CALLBACK — Mecanum inverse kinematics
// ============================================================
void commandCallback(const geometry_msgs::Twist &cmd_msg) {
  Vx = cmd_msg.linear.x;
  Vy = cmd_msg.linear.y;
  Wz = cmd_msg.angular.z;

  w1 = (Vx - Vy - KINEMATIC_L * Wz) / WHEEL_RADIUS;  // FL
  w2 = (Vx + Vy + KINEMATIC_L * Wz) / WHEEL_RADIUS;  // FR
  w3 = (Vx + Vy - KINEMATIC_L * Wz) / WHEEL_RADIUS;  // RL
  w4 = (Vx - Vy + KINEMATIC_L * Wz) / WHEEL_RADIUS;  // RR

  prev_command_time = millis();
}

// ============================================================
// /pid_gains CALLBACK
// ============================================================
void pidGainsCallback(const std_msgs::Float32MultiArray &msg) {
  if (msg.data_length >= 3) {
    KP = msg.data[0];
    KI = msg.data[1];
    KD = msg.data[2];

    resetAllPID();
    nh.loginfo("PID UPDATED");
  }
}

// ============================================================
// MOTOR CONTROL — read encoders, run PID, drive motors
// ============================================================
void movebase(float dt) {
  long encA = Enc_A.readAndReset();
  long encB = Enc_B.readAndReset();
  long encC = Enc_C.readAndReset();
  long encD = Enc_D.readAndReset();

  vel1 = pulse_to_radps(encA, PPR_FL, dt);
  vel2 = pulse_to_radps(encB, PPR_FR, dt);
  vel3 = pulse_to_radps(encC, PPR_RL, dt);
  vel4 = pulse_to_radps(encD, PPR_RR, dt);

  int pwm1 = PID_control(vel1, w1, pid1, dt);
  int pwm2 = PID_control(vel2, w2, pid2, dt);
  int pwm3 = PID_control(vel3, w3, pid3, dt);
  int pwm4 = PID_control(vel4, w4, pid4, dt);

  motor1.setSpeed(pwm1);
  motor2.setSpeed(pwm2);
  motor3.setSpeed(pwm3);
  motor4.setSpeed(pwm4);
}

// ============================================================
// UNIT CONVERSION
// ============================================================

// pulse -> rad/s
float pulse_to_radps(long pulse, float ppr, float dt) {
  if (dt <= 0.0f) return 0.0f;
  return ((float)pulse * 2.0f * PI) / (ppr * dt);
}

// pulse -> RPM
// pulse -> revolutions -> rev/s -> RPM  =>  RPM = pulse / PPR / dt * 60
float pulse_to_rpm(long pulse, float ppr, float dt) {
  if (dt <= 0.0f) return 0.0f;
  return ((float)pulse * 60.0f) / (ppr * dt);
}

// ============================================================
// PID CONTROLLER
// ============================================================
int PID_control(float currentVel, float targetVel, PIDController &pid, float dt) {
  // Stop condition: zero target -> reset state, no output
  if (fabs(targetVel) < 0.001f) {
    pid.integral = 0.0f;
    pid.previousError = 0.0f;
    return 0;
  }

  float error = targetVel - currentVel;

  float P = KP * error;

  pid.integral += error * dt;
  pid.integral = constrain(pid.integral, -150.0f, 150.0f);  // anti-windup
  float I = KI * pid.integral;

  float D = KD * ((error - pid.previousError) / dt);
  pid.previousError = error;

  float output = P + I + D;
  return (int)constrain(output, -255.0f, 255.0f);
}

void resetPID(PIDController &pid) {
  pid.integral = 0.0f;
  pid.previousError = 0.0f;
}

void resetAllPID() {
  resetPID(pid1);
  resetPID(pid2);
  resetPID(pid3);
  resetPID(pid4);
}

// ============================================================
// ROBOT ODOMETRY — wheel velocity -> robot velocity, publish
// ============================================================
void encoder() {
  enc.linear.x  = (vel1 + vel2 + vel3 + vel4) / 4.0f * WHEEL_RADIUS;
  enc.linear.y  = (-vel1 + vel2 + vel3 - vel4) / 4.0f * WHEEL_RADIUS;
  enc.angular.z = (-vel1 + vel2 - vel3 + vel4) / (4.0f * KINEMATIC_L) * WHEEL_RADIUS;

  Enc_pub.publish(&enc);
}

// ============================================================
// ENCODER RPM — rad/s -> RPM, publish [FL, FR, RL, RR]
// ============================================================
void encoderRPM() {
  rpmFL = vel1 * 60.0f / (2.0f * PI);
  rpmFR = vel2 * 60.0f / (2.0f * PI);
  rpmRL = vel3 * 60.0f / (2.0f * PI);
  rpmRR = vel4 * 60.0f / (2.0f * PI);

  rpm_data[0] = rpmFL;
  rpm_data[1] = rpmFR;
  rpm_data[2] = rpmRL;
  rpm_data[3] = rpmRR;

  RPM_pub.publish(&rpm_msg);
}

// ============================================================
// STOP ROBOT
// ============================================================
void stopBase() {
  Vx = 0.0f;
  Vy = 0.0f;
  Wz = 0.0f;

  w1 = 0.0f;
  w2 = 0.0f;
  w3 = 0.0f;
  w4 = 0.0f;

  resetAllPID();

  motor1.setSpeed(0);
  motor2.setSpeed(0);
  motor3.setSpeed(0);
  motor4.setSpeed(0);
}
