#include <Arduino.h>
#include <ros.h>
#include <Servo.h>
#include <std_msgs/Int32.h>
#include <PositionControl.h>
#include <std_msgs/Float32.h>
#include <sensor_msgs/JointState.h>

ros::NodeHandle nh;

const int stepPin = 24; 
const int dirPin_stepper = 25; 
const int enPin = 28;

#define dirPin_elbow 8
#define pwmPin_elbow 9
#define encA_elbow 7
#define encB_elbow 6

#define dirPin_wrist 4
#define pwmPin_wrist 5
#define encA_wrist 2
#define encB_wrist 3

const int D0=10;
const int D1=12;

std_msgs::Float32 position_ticks_elbow;
std_msgs::Float32 position_ticks_wrist;
float theta1=0;
float theta2=0;
Servo grip;
int pos[4];
PositionControl elbow;
PositionControl wrist;
double *setpoint_elbow=&elbow.setPoint;
double *setpoint_wrist=&wrist.setPoint;
void writeStepper(int angle){

    if(angle<0){
        digitalWrite(dirPin_stepper,LOW);
            }
    else if(angle>=0){
        digitalWrite(dirPin_stepper,HIGH);

    }
        angle= (640/18*abs(angle)); // Enables the motor to move in a particular direction
        for(int x = 0; x < angle; x++) {   // 6400 stepes for 180 degree rotation when no of microsteps is 3200 
        digitalWrite(stepPin,HIGH); 
        delayMicroseconds(500); 
        digitalWrite(stepPin,LOW); 
        delayMicroseconds(500); 
  }
}
void writeActuator(float angle){
  angle=angle*150/85;
    if(angle >= 0){
        digitalWrite(D0 , HIGH);
        digitalWrite(D1 , LOW);
        delay((abs(angle) *1000)/12);
        digitalWrite(D0 , LOW);
        digitalWrite(D1 , LOW);

}
else if (angle<=0)
{
        digitalWrite(D0 , LOW);
        digitalWrite(D1 , HIGH);
        
        delay((abs(angle) *1000)/12);
        digitalWrite(D0 , LOW);
        digitalWrite(D1 , LOW);

}
}

void writeElbow(float angle){

  *setpoint_elbow = long((angle *elbow.CPR)/360);
  nh.loginfo("data received");
    }
void writeWrist(float angle){

  *setpoint_wrist = long((angle *elbow.CPR)/360);
  nh.loginfo("data received");
    }
void writeGrip( int angle){
  grip.write(angle); //set servo angle, should be from 0-180  
  // digitalWrite(13, HIGH-digitalRead(13));  //toggle led 
    nh.loginfo("data received for servo");
 
 }

    // void writeWrist(float angle);
    // void writeGrip(int angle);

// int servo_pins[5] = {10, 9, 6, 5, 3}; // PWM Pins on Arduino Uno
// int mid_positions[5] = {100, 100, 100, 90, 100};
// int SERVO_CURRENT_POSITIONS[5];

// float TARGET_JOINT_POSITIONS[5] = {0,0,0,0,0};

// Convert the joint state values to degrees, adjust for the center and write to the servo
// void writeServos() {
//   for (int j = 0; j < 5; j++) {
//     int target_angle;
//     if (j == 2) {
//       // Due to difference in mounting directions
//       target_angle = - TARGET_JOINT_POSITIONS[j]*(180/3.14) + mid_positions[j];
//     } else {
//       target_angle = TARGET_JOINT_POSITIONS[j]*(180/3.14) + mid_positions[j];
//     }
//     robot_servos[j].write(target_angle);
//     SERVO_CURRENT_POSITIONS[j] = target_angle;
//   }
//   nh.spinOnce();
// }

// Subscriber Callback to store the jointstate position values in the global variables
void Callback(const sensor_msgs::JointState &msg) {
  nh.loginfo("data recieved");
  pos[0] = msg.position[0];
  pos[1] = msg.position[1];
  pos[2] = msg.position[2];
  pos[3] = msg.position[3];
  pos[4] = msg.position[4];
  // Call the method to write the joint positions to the servo motors
//   writeServos();
    writeStepper(pos[0]);
    writeActuator(pos[1]);
    writeElbow(pos[2]);
    writeWrist(pos[3]);
    writeGrip(pos[4]);


}

// void Callback(const std_msgs::Int32 &msg) {
//     nh.loginfo("data recieved");
//   angle = msg.data;
//   // Call the method to write the joint positions to the servo motors
// //   writeServos();
//         pos[0]=0;
//         digitalWrite(dirPin_stepper,pos[0]);
//         int angle= (640/18*angle); // Enables the motor to move in a particular direction
//         for(int x = 0; x < angle; x++) {   // 6400 stepes for 180 degree rotation when no of microsteps is 3200
        
//         digitalWrite(stepPin,HIGH); 
//         delayMicroseconds(500); 
//         digitalWrite(stepPin,LOW); 
//         delayMicroseconds(500);
//             nh.loginfo("moving motor");
 
//   }
// }

ros::Publisher ticks_elbow_pub("position_ticks_elbow", &position_ticks_elbow);
ros::Publisher ticks_wrist_pub("position_ticks_wrist", &position_ticks_wrist);

ros::Subscriber<sensor_msgs::JointState> stepper_pos_in("joint_states", &Callback);
// ros::Subscriber<std_msgs::Int32> stepper_pos_in("joint_states", &Callback);

void setup() {
  // Initial the servo motor connections and initialize them at home position
//   for (unsigned int i = 0; i < 5; i++) {
//     robot_servos[i].attach(servo_pins[i]);
//     robot_servos[i].write(mid_positions[i]);
//     SERVO_CURRENT_POSITIONS[i] = mid_positions[i];
//   }

  // Set the communication BaudRate and start the node
  nh.getHardware()->setBaud(115200);
  nh.initNode();
    grip.attach(18);

  nh.subscribe(stepper_pos_in);
  nh.advertise(ticks_elbow_pub);
  nh.advertise(ticks_wrist_pub);


  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin_stepper,OUTPUT);
  pinMode(enPin,OUTPUT);
  digitalWrite(enPin,LOW);

  elbow.setPin(dirPin_elbow, pwmPin_elbow, encA_elbow, encB_elbow);
  wrist.setPin(dirPin_wrist, pwmPin_wrist, encA_wrist, encB_wrist);

  elbow.setReversePolarity(false);
  wrist.setReversePolarity(true);

  elbow.setPIDOutput(-150, 150);
  wrist.setPIDOutput(-50, 50);

  //Tolerance for error in control, have tolerance more than 0.01, otherwise motor inertia takes more time to reach the position with multiple oscillation
  elbow.setTolerance(0.001);
  wrist.setTolerance(0.001);

  // delay(5000);
  elbow.setCPR(26500);
  wrist.setCPR(12532);
}

void loop() {
  // Keep calling the spinOnce() method in this infinite loop to stay tightly coupled with the ROS Serial
  elbow.controlLoop();
  wrist.controlLoop();
  nh.spinOnce();
  if (nh.connected()) {
    position_ticks_elbow.data=elbow.currentPosition;
    ticks_elbow_pub.publish(&position_ticks_elbow);

    position_ticks_wrist.data=wrist.currentPosition;
    ticks_wrist_pub.publish(&position_ticks_wrist);

  }
  delay(3);
}