#include "config.h"
#include <Math.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// Mise en place des variables
Servo esc_1; //FRONT RIGHT (avant droit)
Servo esc_2; //BACK LEFT (arrière gauche)
Servo esc_3; //FRONT LEFT (avant gauche)
Servo esc_4; //BACK RIGHT (arrière droit)

// PID
bool auto_stabilisation_mode = false; // Activer ou pas le mode auto-stabilisé
int dt = 3;
float pid_roll_out,   pid_roll_setpoint,  roll_error, Proportional_roll,  Integral_roll,  Derivative_roll,  last_roll_error, AS_roll_error, AS_Integral_roll, AS_Derivative_roll, AS_last_roll_error = 0;
float pid_pitch_out,  pid_pitch_setpoint, pitch_error, Proportional_pitch, Integral_pitch, Derivative_pitch, last_pitch_error, AS_pitch_error, AS_Integral_pitch, AS_Derivative_pitch, AS_last_pitch_error = 0;
float pid_yaw_out,    pid_yaw_setpoint,   yaw_error,  Proportional_yaw,   Integral_yaw,   Derivative_yaw,   last_yaw_error = 0;

// MOTORS
int motorFR, motorBL, motorFL, motorBR;

// RX
int throttle = THROTTLE_RMIN;
volatile int input0, input1, input2, input3, input4;
volatile unsigned long chrono_start0, chrono_start1, chrono_start2, chrono_start3, chrono_start4;
volatile int last_interrupt_time0, last_interrupt_time1, last_interrupt_time2, last_interrupt_time3, last_interrupt_time4;

// IMU
float roll_speed, roll_angle;
float pitch_speed, pitch_angle;
float yaw_speed, yaw_angle;
const float pi = 3.14159265359;
Adafruit_BNO055 bno = Adafruit_BNO055();

////////////////////////////////////////////////////////////////////////
//INTERRUPTS
////////////////////////////////////////////////////////////////////////
void calcSignal0() {
  last_interrupt_time0 = micros();
  if (digitalRead(A0) == HIGH) {
    chrono_start0 = micros();
  }
  else {
    if (chrono_start0 != 0) {
      input0 = ((volatile int)micros() - chrono_start0);
      chrono_start0 = 0;
    }
  }
}

void calcSignal1() {
  last_interrupt_time1 = micros();
  if (digitalRead(A1) == HIGH) {
    chrono_start1 = micros();
  }
  else {
    if (chrono_start1 != 0) {
      input1 = ((volatile int)micros() - chrono_start1);
      chrono_start1 = 0;
    }
  }
}

void calcSignal2() {
  last_interrupt_time2 = micros();
  if (digitalRead(A2) == HIGH) {
    chrono_start2 = micros();
  }
  else {
    if (chrono_start2 != 0) {
      input2 = ((volatile int)micros() - chrono_start2);
      chrono_start2 = 0;
    }
  }
}

void calcSignal3() {
  last_interrupt_time3 = micros();
  if (digitalRead(A3) == HIGH) {
    chrono_start3 = micros();
  }
  else {
    if (chrono_start3 != 0) {
      input3 = ((volatile int)micros() - chrono_start3);
      chrono_start3 = 0;
    }
  }
}

void calcSignal4() {
  last_interrupt_time4 = micros();
  if (digitalRead(A4) == HIGH) {
    chrono_start4 = micros();
  }
  else {
    if (chrono_start4 != 0) {
      input4 = ((volatile int)micros() - chrono_start4);
      chrono_start4 = 0;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////







////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PID controller
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void pid_compute() {

  //ROLL calculations
  pid_roll_setpoint = 0;
  if (input0 > THROTTLE_RMID + 10)pid_roll_setpoint = (input0 - THROTTLE_RMID + 10) / 2.0;
  else if (input0 < THROTTLE_RMID - 10)pid_roll_setpoint = (input0 - THROTTLE_RMID - 10) / 2.0;

  roll_error = roll_speed - pid_roll_setpoint;

  Proportional_roll = ROLL_PID_KP * roll_error;
  Integral_roll += ROLL_PID_KI * roll_error;
  if (Integral_roll > ROLL_PID_MAX)Integral_roll = ROLL_PID_MAX;
  else if (Integral_roll < ROLL_PID_MIN)Integral_roll = ROLL_PID_MIN;
  Derivative_roll = ROLL_PID_KD * (roll_error - last_roll_error);

  pid_roll_out = Proportional_roll + Integral_roll + Derivative_roll;
  if (pid_roll_out > ROLL_PID_MAX)pid_roll_out = ROLL_PID_MAX;
  else if (pid_roll_out < ROLL_PID_MIN)pid_roll_out = ROLL_PID_MIN;

  last_roll_error = roll_error;


  //PITCH calculations
  pid_pitch_setpoint = 0;
  if (input1 > THROTTLE_RMID + 10)pid_pitch_setpoint = (input1 - THROTTLE_RMID + 10) / 2.0;
  else if (input1 < THROTTLE_RMID - 10)pid_pitch_setpoint = (input1 - THROTTLE_RMID - 10) / 2.0;

  pitch_error = pitch_speed - pid_pitch_setpoint;

  Proportional_pitch = PITCH_PID_KP * pitch_error;
  Integral_pitch += PITCH_PID_KI * pitch_error;
  if (Integral_pitch > PITCH_PID_MAX)Integral_pitch = PITCH_PID_MAX;
  else if (Integral_pitch < PITCH_PID_MIN)Integral_pitch = PITCH_PID_MIN;
  Derivative_pitch = PITCH_PID_KD * (pitch_error - last_pitch_error);

  pid_pitch_out = Proportional_pitch + Integral_pitch + Derivative_pitch;
  if (pid_pitch_out > PITCH_PID_MAX)pid_pitch_out = PITCH_PID_MAX;
  else if (pid_pitch_out < PITCH_PID_MIN)pid_pitch_out = PITCH_PID_MIN;

  last_pitch_error = pitch_error;

  //YAW calculations
  pid_yaw_setpoint = 0;
  if (input3 > THROTTLE_RMID + 10)pid_yaw_setpoint = (input3 - THROTTLE_RMID + 10) / 2.0;
  else if (input3 < THROTTLE_RMID - 10)pid_yaw_setpoint = (input3 - THROTTLE_RMID - 10) / 2.0;

  yaw_error = yaw_speed - pid_yaw_setpoint;

  Proportional_yaw = YAW_PID_KP * yaw_error;
  Integral_yaw += YAW_PID_KI * yaw_error;
  if (Integral_yaw > YAW_PID_MAX)Integral_yaw = YAW_PID_MAX;
  else if (Integral_yaw < YAW_PID_MIN)Integral_yaw = YAW_PID_MIN;
  Derivative_yaw = YAW_PID_KD * (yaw_error - last_yaw_error);

  pid_yaw_out = Proportional_yaw + Integral_yaw + Derivative_yaw;
  if (pid_yaw_out > YAW_PID_MAX)pid_yaw_out = YAW_PID_MAX;
  else if (pid_yaw_out < YAW_PID_MIN)pid_yaw_out = YAW_PID_MIN;

  last_yaw_error = yaw_error;
}

/////////AUTO STABILIZATION MANQUANTE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////









// Fonction qui empeche les moteurs de tourner
void motors_set_to_zero() {
  esc_1.writeMicroseconds(MOTOR_ZERO_LEVEL);
  esc_3.writeMicroseconds(MOTOR_ZERO_LEVEL);
  esc_2.writeMicroseconds(MOTOR_ZERO_LEVEL);
  esc_4.writeMicroseconds(MOTOR_ZERO_LEVEL);
}
/////////////////////////////////////////////////////////////////////////////////////////
// Fonctions de debug
/////////////////////////////////////////////////////////////////////////////////////////
void print_motors() {
  Serial.print(motorFR);
  Serial.print(",");
  Serial.print(motorFL);
  Serial.print(",");
  Serial.print(motorBL);
  Serial.print(",");
  Serial.print(motorBR);
  Serial.print(",");
  Serial.print(roll_speed);
  Serial.print(",");
  Serial.print(pitch_speed);
  Serial.print(",");
  Serial.println(yaw_speed);
}

void print_pid_out() {
  Serial.print(pid_roll_out);
  Serial.print(",");
  Serial.print(pid_pitch_out);
  Serial.print(",");
  Serial.print(pid_yaw_out);
  Serial.print(",");
  Serial.println(pitch_speed);
}

void print_pid_setpoints() {
  Serial.print(pid_roll_setpoint);
  Serial.print(",");
  Serial.print(pid_pitch_setpoint);
  Serial.print(",");
  Serial.println(pid_yaw_setpoint);
}
void print_imu() {
  Serial.print(roll_speed);
  Serial.print(",");
  Serial.print(pitch_speed);
  Serial.print(",");
  Serial.println(yaw_speed);
}

void print_rx() {
  Serial.print(input0);
  Serial.print(",");
  Serial.print(input1);
  Serial.print(",");
  Serial.print(input2);
  Serial.print(",");
  Serial.println(input3);
}

void print_roll_pid() {
  Serial.print(Proportional_roll);
  Serial.print(",");
  Serial.print(Integral_roll);
  Serial.print(",");
  Serial.println(Derivative_roll);
}


// Fonction qui s'execute une fois et au début
void setup() {
  Serial.begin(9600);
  //BNO init
  bno.begin();
  bno.setExtCrystalUse(true);

  //RX init
  attachInterrupt(A0, calcSignal0, CHANGE);
  attachInterrupt(A1, calcSignal1, CHANGE);
  attachInterrupt(A2, calcSignal2, CHANGE);
  attachInterrupt(A3, calcSignal3, CHANGE);
  attachInterrupt(A4, calcSignal4, CHANGE);

  //Motors init
  esc_1.attach(MOTOR_PIN_FRONT_RIGHT, 1000, 2000);
  esc_2.attach(MOTOR_PIN_FRONT_LEFT, 1000, 2000);
  esc_3.attach(MOTOR_PIN_BACK_LEFT, 1000, 2000);
  esc_4.attach(MOTOR_PIN_BACK_RIGHT, 1000, 2000);

  esc_1.writeMicroseconds(1000);
  esc_2.writeMicroseconds(1000);
  esc_3.writeMicroseconds(1000);
  esc_4.writeMicroseconds(1000);

  Proportional_roll, Integral_roll, Derivative_roll = 0;
  Proportional_pitch, Integral_pitch, Derivative_pitch = 0;
  Proportional_yaw, Integral_yaw, Derivative_yaw = 0;

  delay(5000);
}


// Fonction qui s'execute a chaque cycle
void loop() {

  //IMU calculations
  if (auto_stabilisation_mode == false) {
    imu::Vector<3> gyroscope = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    roll_speed = gyroscope.x() * 180 / pi;
    pitch_speed = gyroscope.y() * 180 / pi;
    yaw_speed = gyroscope.z() * 180 / pi;
  } else {
    imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
    roll_angle = euler.z();
    pitch_angle = euler.y();
    yaw_angle = euler.x();
  }


  //MOTORS
  throttle = map(input2, THROTTLE_RMIN, THROTTLE_RMAX, MOTOR_ZERO_LEVEL, MOTOR_MAX_LEVEL);
  //PID
  pid_compute();


  ///ARM switch
  if (input4 > 1500) {
    motorFR = throttle + pid_pitch_out + pid_roll_out - pid_yaw_out;
    if (motorFR > 2000) motorFR = 2000;
    else if (motorFR < 1000) motorFR = 1000;

    motorFL = throttle + pid_pitch_out - pid_roll_out + pid_yaw_out;
    if (motorFL > 2000) motorFL = 2000;
    else if (motorFL < 1000) motorFL = 1000;

    motorBL = throttle - pid_pitch_out - pid_roll_out - pid_yaw_out;
    if (motorBL > 2000) motorBL = 2000;
    else if (motorBL < 1000) motorBL = 1000;

    motorBR = throttle - pid_pitch_out + pid_roll_out + pid_yaw_out;
    if (motorBR > 2000) motorBR = 2000;
    else if (motorBR < 1000) motorBR = 1000;

    esc_1.writeMicroseconds(motorFR);
    esc_2.writeMicroseconds(motorFL);
    esc_3.writeMicroseconds(motorBL);
    esc_4.writeMicroseconds(motorBR);

  } else if (input4 < 1500) {
    motorFR = 1000;
    motorFL = 1000;
    motorBL = 1000;
    motorBR = 1000;
    esc_1.writeMicroseconds(motorFR);
    esc_2.writeMicroseconds(motorFL);
    esc_3.writeMicroseconds(motorBL);
    esc_4.writeMicroseconds(motorBR);
    Proportional_roll, Integral_roll, Derivative_roll = 0;
    Proportional_pitch, Integral_pitch, Derivative_pitch = 0; //reset after disarm
    Proportional_yaw, Integral_yaw, Derivative_yaw = 0;
  }

  print_motors();
  delay(4);
}
