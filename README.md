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
  &nbsp;&nbsp;&nbsp;&nbsp;
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
  &nbsp;&nbsp;&nbsp;&nbsp;
  🔄 <b>Mecanum Kinematics</b>
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

## 🛠️ Hardware

| 🔩 Component | 📋 Description |
|---|---|
| 🧠 Teensy 4.0 | Main Controller |
| ⚙️ DC Motor | Drive Motor |
| 🔄 Mecanum Wheel | Omnidirectional Drive |
| 📡 Encoder | Wheel Feedback |
| ⚡ Motor Driver | Motor Control |

## 🧮 Kinematics

Mecanum wheel inverse kinematics is used to convert:

```text
🚗 Robot Velocity
      ↓
   Vx, Vy, ω
      ↓
🧮 Mecanum Kinematics
      ↓
⚙️ Wheel Velocity
      ↓
🔄 PID Controller
      ↓
🚗 Motor