#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy
from geometry_msgs.msg import Twist
import sys
import termios
import tty


# ============================================================
# SPEED
# ============================================================

LINEAR_SPEED = 0.30
STRAFE_SPEED = 0.30
ANGULAR_SPEED = 0.80

SPEED_STEP = 0.05

MIN_LINEAR_SPEED = 0.05
MAX_LINEAR_SPEED = 1.00

MIN_ANGULAR_SPEED = 0.10
MAX_ANGULAR_SPEED = 2.00


# ============================================================
# KEYBOARD
# ============================================================

def get_key():

    fd = sys.stdin.fileno()

    old_settings = termios.tcgetattr(fd)

    try:
        tty.setraw(fd)

        key = sys.stdin.read(1)

    finally:
        termios.tcsetattr(
            fd,
            termios.TCSADRAIN,
            old_settings
        )

    return key


# ============================================================
# STOP
# ============================================================

def stop_robot(pub):

    cmd = Twist()

    cmd.linear.x = 0.0
    cmd.linear.y = 0.0
    cmd.linear.z = 0.0

    cmd.angular.x = 0.0
    cmd.angular.y = 0.0
    cmd.angular.z = 0.0

    # ส่ง STOP หลายครั้ง
    for i in range(3):
        pub.publish(cmd)
        rospy.sleep(0.05)


# ============================================================
# MAIN
# ============================================================

def main():

    global LINEAR_SPEED
    global STRAFE_SPEED
    global ANGULAR_SPEED

    rospy.init_node(
        "mecanum_teleop"
    )

    pub = rospy.Publisher(
        "/cmd_vel",
        Twist,
        queue_size=10
    )

    print("")
    print("======================================")
    print("       BATLYBOT MECANUM CONTROL")
    print("======================================")
    print("")
    print(" W : Forward")
    print(" S : Backward")
    print(" A : Strafe Left")
    print(" D : Strafe Right")
    print(" Q : Rotate CCW")
    print(" E : Rotate CW")
    print(" X : STOP")
    print("")
    print(" + : Speed UP")
    print(" - : Speed DOWN")
    print("")
    print(" ESC : EXIT")
    print("======================================")
    print("")

    try:

        while not rospy.is_shutdown():

            key = get_key()


            # =================================================
            # ESC
            #
            # ESC key = 27
            # =================================================

            if ord(key) == 27:

                print("")
                print("ESC pressed")
                print("Stopping robot...")

                stop_robot(pub)

                print("Exit.")

                break


            # =================================================
            # SPEED UP
            # =================================================

            if key == '+' or key == '=':

                LINEAR_SPEED += SPEED_STEP
                STRAFE_SPEED += SPEED_STEP
                ANGULAR_SPEED += SPEED_STEP

                LINEAR_SPEED = min(
                    LINEAR_SPEED,
                    MAX_LINEAR_SPEED
                )

                STRAFE_SPEED = min(
                    STRAFE_SPEED,
                    MAX_LINEAR_SPEED
                )

                ANGULAR_SPEED = min(
                    ANGULAR_SPEED,
                    MAX_ANGULAR_SPEED
                )

                print(
                    "\rSpeed: Linear={:.2f} m/s | Angular={:.2f} rad/s"
                    .format(
                        LINEAR_SPEED,
                        ANGULAR_SPEED
                    )
                )


            # =================================================
            # SPEED DOWN
            # =================================================

            elif key == '-' or key == '_':

                LINEAR_SPEED -= SPEED_STEP
                STRAFE_SPEED -= SPEED_STEP
                ANGULAR_SPEED -= SPEED_STEP

                LINEAR_SPEED = max(
                    LINEAR_SPEED,
                    MIN_LINEAR_SPEED
                )

                STRAFE_SPEED = max(
                    STRAFE_SPEED,
                    MIN_LINEAR_SPEED
                )

                ANGULAR_SPEED = max(
                    ANGULAR_SPEED,
                    MIN_ANGULAR_SPEED
                )

                print(
                    "\rSpeed: Linear={:.2f} m/s | Angular={:.2f} rad/s"
                    .format(
                        LINEAR_SPEED,
                        ANGULAR_SPEED
                    )
                )


            # =================================================
            # CREATE COMMAND
            # =================================================

            cmd = Twist()


            # =================================================
            # FORWARD
            # =================================================

            if key == 'w':

                cmd.linear.x = LINEAR_SPEED


            # =================================================
            # BACKWARD
            # =================================================

            elif key == 's':

                cmd.linear.x = -LINEAR_SPEED


            # =================================================
            # LEFT
            # =================================================

            elif key == 'a':

                cmd.linear.y = STRAFE_SPEED


            # =================================================
            # RIGHT
            # =================================================

            elif key == 'd':

                cmd.linear.y = -STRAFE_SPEED


            # =================================================
            # ROTATE CCW
            # =================================================

            elif key == 'q':

                cmd.angular.z = ANGULAR_SPEED


            # =================================================
            # ROTATE CW
            # =================================================

            elif key == 'e':

                cmd.angular.z = -ANGULAR_SPEED


            # =================================================
            # STOP
            # =================================================

            elif key == 'x':

                cmd.linear.x = 0.0
                cmd.linear.y = 0.0
                cmd.angular.z = 0.0


            # =================================================
            # PUBLISH
            # =================================================

            pub.publish(cmd)

    finally:

        # ==============================================
        # ถ้า Ctrl+C หรือโปรแกรมถูกปิด
        # ให้ STOP หุ่นยนต์
        # ==============================================

        stop_robot(pub)


# ============================================================
# START
# ============================================================

if __name__ == "__main__":

    try:

        main()

    except rospy.ROSInterruptException:

        pass
