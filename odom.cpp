#include <ros/ros.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2/LinearMath/Quaternion.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/Twist.h>
#include <cmath>


// ============================================================
// BATLYBOT — Odometry Publisher
// Subscribe : /raw_vel  (geometry_msgs/Twist)  ← จาก Teensy
// Publish   : /odom     (nav_msgs/Odometry)
// Broadcast : TF  odom → base_link
// Rate      : 50 Hz
// ============================================================


// ============================================================
// 1. ROBOT POSE  (world / odom frame)
// ============================================================

double x  = 0.0;   // m
double y  = 0.0;   // m
double th = 0.0;   // rad


// ============================================================
// 2. ROBOT VELOCITY  (base_link frame)
// ============================================================

double vx  = 0.0;   // m/s
double vy  = 0.0;   // m/s
double vth = 0.0;   // rad/s


// ============================================================
// 3. /raw_vel CALLBACK
// ============================================================

void rawVelCallback(const geometry_msgs::Twist& msg)
{
    vx  = msg.linear.x;
    vy  = msg.linear.y;
    vth = msg.angular.z;
}


// ============================================================
// 4. MAIN
// ============================================================

int main(int argc, char** argv)
{
    ros::init(argc, argv, "odometry_publisher");
    ros::NodeHandle nh;


    // --------------------------------------------------------
    // Publisher / Subscriber / TF
    // --------------------------------------------------------

    ros::Publisher odom_pub =
        nh.advertise<nav_msgs::Odometry>("odom", 50);

    ros::Subscriber raw_sub =
        nh.subscribe("raw_vel", 50, rawVelCallback);

    tf2_ros::TransformBroadcaster tf_broadcaster;


    // --------------------------------------------------------
    // Time
    // --------------------------------------------------------

    ros::Time current_time = ros::Time::now();
    ros::Time last_time    = current_time;

    ros::Rate rate(50.0);   // 50 Hz


    // --------------------------------------------------------
    // Main loop
    // --------------------------------------------------------

    while (ros::ok())
    {
        ros::spinOnce();

        current_time = ros::Time::now();
        double dt    = (current_time - last_time).toSec();

        // Guard: skip abnormal dt
        if (dt <= 0.0 || dt > 1.0)
        {
            last_time = current_time;
            rate.sleep();
            continue;
        }


        // ====================================================
        // Dead-reckoning
        // base_link velocity → odom frame displacement
        //
        //   dx  = (vx·cosθ - vy·sinθ) · dt
        //   dy  = (vx·sinθ + vy·cosθ) · dt
        //   dth =  vth · dt
        // ====================================================

        double delta_x  = (vx * std::cos(th) - vy * std::sin(th)) * dt;
        double delta_y  = (vx * std::sin(th) + vy * std::cos(th)) * dt;
        double delta_th = vth * dt;

        x  += delta_x;
        y  += delta_y;
        th += delta_th;

        // Normalize angle to [-π, π]
        th = std::atan2(std::sin(th), std::cos(th));


        // ====================================================
        // Quaternion  (yaw only)
        // ====================================================

        tf2::Quaternion q;
        q.setRPY(0.0, 0.0, th);


        // ====================================================
        // TF: odom → base_link
        // ====================================================

        geometry_msgs::TransformStamped tf_msg;

        tf_msg.header.stamp            = current_time;
        tf_msg.header.frame_id         = "odom";
        tf_msg.child_frame_id          = "base_link";

        tf_msg.transform.translation.x = x;
        tf_msg.transform.translation.y = y;
        tf_msg.transform.translation.z = 0.0;

        tf_msg.transform.rotation.x    = q.x();
        tf_msg.transform.rotation.y    = q.y();
        tf_msg.transform.rotation.z    = q.z();
        tf_msg.transform.rotation.w    = q.w();

        tf_broadcaster.sendTransform(tf_msg);


        // ====================================================
        // nav_msgs/Odometry
        // ====================================================

        nav_msgs::Odometry odom;

        // Header
        odom.header.stamp    = current_time;
        odom.header.frame_id = "odom";
        odom.child_frame_id  = "base_link";

        // Pose
        odom.pose.pose.position.x    = x;
        odom.pose.pose.position.y    = y;
        odom.pose.pose.position.z    = 0.0;
        odom.pose.pose.orientation.x = q.x();
        odom.pose.pose.orientation.y = q.y();
        odom.pose.pose.orientation.z = q.z();
        odom.pose.pose.orientation.w = q.w();

        // Pose covariance (diagonal 6x6)
        odom.pose.covariance[0]  = 0.001;   // x
        odom.pose.covariance[7]  = 0.001;   // y
        odom.pose.covariance[14] = 1e6;     // z  (2D robot)
        odom.pose.covariance[21] = 1e6;     // roll
        odom.pose.covariance[28] = 1e6;     // pitch
        odom.pose.covariance[35] = 0.001;   // yaw

        // Twist
        odom.twist.twist.linear.x  = vx;
        odom.twist.twist.linear.y  = vy;
        odom.twist.twist.linear.z  = 0.0;
        odom.twist.twist.angular.x = 0.0;
        odom.twist.twist.angular.y = 0.0;
        odom.twist.twist.angular.z = vth;

        // Twist covariance (diagonal 6x6)
        odom.twist.covariance[0]  = 0.0001;  // vx
        odom.twist.covariance[7]  = 0.0001;  // vy
        odom.twist.covariance[14] = 1e6;     // vz
        odom.twist.covariance[21] = 1e6;     // roll rate
        odom.twist.covariance[28] = 1e6;     // pitch rate
        odom.twist.covariance[35] = 0.0001;  // yaw rate

        odom_pub.publish(odom);


        // ====================================================
        // Update time
        // ====================================================

        last_time = current_time;
        rate.sleep();
    }

    return 0;
}
