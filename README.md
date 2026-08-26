# 🤖 ROS Mecanum Robot

<p align="center">

  <img src="https://inwfile.com/s-dw/m35oir.jpg"
       width="42%"
       alt="Motor A+B Phase 12V">

  <img src="https://images.thingbits.net/eyJidWNrZXQiOiJ0aGluZ2JpdHMtbmV0Iiwia2V5IjoiM3N1aTdjemF1d3lkdnN0aGV4djM2OHYwYzAwMSJ9"
       width="42%"
       alt="Drive Motor">

</p>

<p align="center">
  ⚙️ <b>Motor A+B</b>
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  🚗 <b>Drive Motor</b>
</p>

<p align="center">

  <img src="https://www.pjrc.com/store/teensy40_card10a_rev2.png"
       width="42%"
       alt="Teensy 4.0">

  <img src="https://docs.hiwonder.com/projects/MentorPi/en/latest/_static/media/4.MotionControlLesson/4.1/media/image1.png"
       width="42%"
       alt="Mecanum Kinematics">

</p>

<p align="center">
  🧠 <b>Teensy 4.0</b>
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  🧮 <b>Mecanum Kinematics</b>
</p>
---

## 🚀 Features

- 🤖 Mecanum Wheel Drive
- 🧭 Odometry
- 🎯 PID Motor Control
- 🔄 Forward / Backward
- ↔️ Strafe Left / Right
- 🔃 Rotation
- 📡 ROS `cmd_vel`
- ⚡ Teensy 4.0
- 🔧 Encoder Feedback

---

## 🛠️ Hardware

| 🔩 Component | 📋 Description |
|---|---|
| 🧠 Teensy 4.0 | Main Controller |
| ⚙️ DC Motor | Drive Motor |
| 🔄 Mecanum Wheel | Omnidirectional Drive |
| 📡 Encoder | Wheel Feedback |
| ⚡ Motor Driver | Motor Control |

---

## 🤖 Robot Parameters

พารามิเตอร์เหล่านี้ใช้สำหรับกำหนดขนาดและรูปทรงทางเรขาคณิตของหุ่นยนต์
ซึ่งมีผลโดยตรงต่อการคำนวณ Mecanum Kinematics และการควบคุมการเคลื่อนที่ของหุ่นยนต์

📏 `WHEEL_RADIUS` → รัศมีของล้อ  
ใช้กำหนดระยะจากจุดศูนย์กลางของล้อไปยังจุดสัมผัสพื้น
และใช้ในการแปลงระหว่างความเร็วเชิงเส้นของล้อ (m/s)
กับความเร็วเชิงมุมของล้อ (rad/s)

↕️ `LX` → ระยะจากจุดศูนย์กลางของหุ่นยนต์ไปยังล้อด้านหน้าและด้านหลัง  
ใช้กำหนดระยะในแนวหน้า–หลังของหุ่นยนต์
และเป็นพารามิเตอร์ที่ใช้ในการคำนวณการหมุนของหุ่นยนต์

↔️ `LY` → ระยะจากจุดศูนย์กลางของหุ่นยนต์ไปยังล้อด้านซ้ายและด้านขวา  
ใช้กำหนดระยะในแนวซ้าย–ขวาของหุ่นยนต์
และเป็นพารามิเตอร์ที่ใช้ในการคำนวณการหมุนของหุ่นยนต์

🔄 `KINEMATIC_L` → ความยาวทาง Kinematic ของหุ่นยนต์  
เป็นค่าที่ได้จากผลรวมของ `LX` และ `LY`
และใช้ในสมการ Mecanum Kinematics เพื่อคำนวณผลกระทบ
ของความเร็วเชิงมุม `ω` ที่มีต่อความเร็วของล้อแต่ละล้อ

```text
KINEMATIC_L = LX + LY
WHEEL_RADIUS → meter (m)
LX           → meter (m)
LY           → meter (m)
KINEMATIC_L  → meter (m)

🧮 Kinematics

Mecanum wheel inverse kinematics is used to convert:
🚗 Robot Velocity
       │
       ▼
   Vx, Vy, ω
       │
       ▼
🧮 Mecanum Kinematics
       │
       ▼
⚙️ Wheel Velocity
       │
       ▼
🎯 PID Controller
       │
       ▼
🚗 Motor


Wheel Velocity

FL = (Vx - Vy - Lω) / R
FR = (Vx + Vy + Lω) / R
RL = (Vx + Vy - Lω) / R
RR = (Vx - Vy + Lω) / R

R = WHEEL_RADIUS
L = KINEMATIC_L

KINEMATIC_L = LX + LY


🧭 Odometry
⚙️ FL Encoder ──┐
⚙️ FR Encoder ──┤
⚙️ RL Encoder ──┼──► 🧮 Odometry ──► 📍 Robot Pose
⚙️ RR Encoder ──┘

Pose = [ X  Y  θ ]

X → ตำแหน่งตามแกน X
Y → ตำแหน่งตามแกน Y
θ → Orientation ของหุ่นยนต์