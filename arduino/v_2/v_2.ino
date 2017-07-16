// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "var.h"

#include "MPU6050_6Axis_MotionApps20.h"
//#include "MPU6050.h" // not necessary if using MotionApps include file

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu_2(0x69); // <-- use for AD0 high

#define INTERRUPT_PIN 7  // use pin 2 on Arduino Uno & most boards

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n' };

const int INDEX_FINGER = 4;
const int MIDDLE_FINGER = 3;

const int WOBBLE_THRESHOLD = 35;
const int WOBBLE_COUNT = 50;

const int VOLUME_THRESHOLD = 5000;
const int VOLUME_COUNT = 30;

const int VOLUME_DECREASE_THRESHOLD = -2000;
const int VOLUME_DECREASE_COUNT = 10;

const double GRAVITY_FACTOR = 9.2; // SHOULD BE 9.8
const double HORIZONTAL_FACTOR = 4.5; // SHOULD BE 0
const double OUT_FACTOR = 2; // SHOULD BE 0;

const double ACCEL_Z_RESET_COUNT = HISTORY_LENGTH - 5;

float TEMPO = 700;
float noteStartTime;

bool isNotePlaying = false;

int channel = 1; // Defines the MIDI channel to send messages on (values from 1-16)
int velocity = 50; // Defines the velocity that the note plays at (values from 0-127)

int32_t current_note [2];
int current_note_spot = 0;

struct YawPitchRoll ypr;
struct Acceleration accel;
struct TrapezoidRule velocity_z;
struct TrapezoidRule position_z;

struct TrapezoidRule velocity_x;
struct TrapezoidRule position_x;

struct TrapezoidRule velocity_y;
struct TrapezoidRule position_y;

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {
  mpu_init(mpu, INTERRUPT_PIN);
  cap_init();

  Serial.println("Waiting...");
  // first data points are incorrect and need to clear buffer
  while (millis() < 6500) {
    mpu_setup();
    getYawPitchRoll(&ypr);
    getLinearAccel(&accel);
  }

  Serial.println("Done");
  velocity_z = {0,0,millis(), GRAVITY_FACTOR};
  position_z  = {0,0,millis(), 0};

  velocity_y = {0,0,millis(), OUT_FACTOR};
  position_y  = {0,0,millis(), 0};

  velocity_x = {0,0,millis(), HORIZONTAL_FACTOR};
  position_x  = {0,0,millis(), 0};
}


// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================
int accelSpot = 0;
void printArray (double array []) {
  Serial.println("Print Array");
  for (int i=0;i < HISTORY_LENGTH;i++) {
    Serial.printf(" %f",array[i]);
  }
  Serial.println("\n");
}
/*
  axis = 0 for x-axis
  axis = 1 for y-axis
  axis = 2 for z-axis
*/
bool detectRotation (YawPitchRoll ypr, double threshold, int axis = 1, double baseline = 0) {
  bool flag = false;
  if (axis == 0) {
    if (ypr.roll - baseline > threshold) flag = true;
  } else if (axis == 1 ) {
    if (ypr.pitch - baseline > threshold) flag = true;
  } else if (axis == 2) {
    if (ypr.yaw - baseline > threshold) flag = true;
  } else {
    Serial.println("You fucked up");
  }
  return flag;
}
double calculateRotationMagnitude (YawPitchRoll ypr, int axis = 1, double baseline = 0, double max = 10, double min = 0) {
  double magnitude = 0;
  if (axis == 0) {
    magnitude =  (max-min)/2 + (max-min)/2*(ypr.roll-baseline)/180; // ensures max of baseline set to -90
  } else if (axis == 1 ) {
    magnitude =  (max-min)/2 + ((max-min)/2)*(ypr.pitch-baseline)/180; // ensures max of baseline set to -90
  } else if (axis == 2) {
    magnitude =  (max-min)/2 + (max-min)/2*(ypr.yaw-baseline)/180; // ensures max of baseline set to -90
  } else {
    Serial.println("You fucked up");
  }
  return magnitude;
}


int FilledSpot = 0;

double AX_HIST[HISTORY_LENGTH];
double AY_HIST[HISTORY_LENGTH];
double AZ_HIST[HISTORY_LENGTH];

void addAccel (double ax, double ay, double az) {
  AX_HIST[FilledSpot] = ax;
  AY_HIST[FilledSpot] = ay;
  AZ_HIST[FilledSpot] = az;

  FilledSpot++;
  if (FilledSpot == HISTORY_LENGTH) {
    FilledSpot = 0;
  }
}

void clearArray (double * data) {
  for (int i = 0; i < HISTORY_LENGTH; i++) {
    data[i] = 0;
  }
}

bool DetectThreshold (double * data, double magnitude, int count_requirement, bool comparator = true) {
  int count = 0;
  //int len = sizeof(data)/sizeof(data[0]); //takes length of pointer instead of array
  int len = HISTORY_LENGTH;
  for (int i = 0; i < len; i++ ) {
    if (data[i] > magnitude && comparator) {
      count++;
    }
    else if (data[i] < magnitude && !comparator) {
      count++;
    }

    if (count > (count_requirement-1)) {
      return true;
    }
  }
  return false;
}

bool tryFindAction = false;

void loop() {
  cap_read();

  if(!mpu_setup()) return;

  //getYawPitchRoll(&ypr);
  //printYawPitchRoll(ypr);

  getLinearAccel(&accel);

  addAccel(accel.x, accel.y, accel.z);
  //getWorldAccel(&accel); // want to try with WorldAccel
  // printAccel(accel);

  if (cap_turned_on(MIDDLE_FINGER) || cap_turned_on(INDEX_FINGER)) {
      //print_status();
      tryFindAction = true;
      Serial.println("Reseting Position\n");
      clearArray(AZ_HIST);
      clearArray(AX_HIST);
  }
  if (cap_turned_off(MIDDLE_FINGER) || cap_turned_off(INDEX_FINGER)) {
      tryFindAction = false;
  }

  if (!tryFindAction) return;

  if (DetectThreshold(AZ_HIST, AZ_THRESHOLD_UP, AZ_COUNT, true)) {
    Serial.println("Moved Up!");
    tryFindAction = false;
    printAccel(accel);
    turnOnChannel(CC_HIGH_PASS,FILTER_ON, MAIN_CHANNEL);

  }
  if (DetectThreshold(AZ_HIST, AZ_THRESHOLD_DOWN, AZ_COUNT, false)) {
    Serial.println("Moved DOWN!");
    tryFindAction = false;
    printAccel(accel);
    turnOffChannel(CC_HIGH_PASS,FILTER_OFF, MAIN_CHANNEL);


  }
  if (DetectThreshold(AX_HIST, AX_THRESHOLD_RIGHT, AX_COUNT, true)) {
    Serial.println("Moved left!");
    tryFindAction = false;
    printAccel(accel);
    turnOnChannel(CC_LOW_PASS,FILTER_ON, MAIN_CHANNEL);


  }
  if (DetectThreshold(AX_HIST, AX_THRESHOLD_LEFT, AX_COUNT, false)) {
    Serial.println("Moved right!");
    tryFindAction = false;
    printAccel(accel);
    turnOffChannel(CC_LOW_PASS,FILTER_OFF, MAIN_CHANNEL);


  }
  return;

  if (accel.x > 60 || accel.x < -60) {
    Serial.println("Move in X-Position / Horizontal Plane");
  }
  if (accel.y > 60 || accel.y < -60) {
    Serial.println("Move in Y-Position / IN and OUT Plane");
  }
  if (accel.z > 60 || accel.z < -60) {
    Serial.println("Move in Z-Position / Vertical Plane");
  }

  doubleTrapExecution(accel.z, &velocity_z, &position_z);
  doubleTrapExecution(accel.x, &velocity_x, &position_x);
  doubleTrapExecution(accel.y, &velocity_y, &position_y);

  printPositions(position_x, position_y, position_z);

  double percentVertical = amountVertical(position_z.value);
  double percentHorizontal = amountHorizontal(position_x.value);
  double percentOut = amountOut(position_y.value);

  if ((percentVertical > 0.4 || percentVertical < -0.4) && tryFindAction) {
    #if DEBUG
    printPositions(position_x, position_y, position_z);
    #endif
    if (percentVertical > 0) {
      Serial.println("Arm Raised");
    } else {
      Serial.println("Arm Lowered");
    }
    Serial.printf("Percents: Vertical: %f \t Horizontal: %f \t Out: %f \n\n", percentVertical, percentHorizontal, percentOut);
    tryFindAction = false;
  }
  if ((percentHorizontal > 0.4 || percentHorizontal < -0.4) && tryFindAction) {
    #if DEBUG
    printPositions(position_x, position_y, position_z);
    #endif
    if (percentHorizontal > 0) {
      Serial.printf("Arm Left");
    } else {
      Serial.println("Arm Right");
    }
    Serial.printf("Percents: Vertical: %f \t Horizontal: %f \t Out: %f \n\n", percentVertical, percentHorizontal, percentOut);
    tryFindAction = false;
  }
  // if ((percentOut > 0.4 || percentOut < -0.4)&& tryFindAction) {
  //   if (percentOut > 0) {
  //     Serial.println("Arm Out");
  //   } else {
  //     Serial.println("Arm In");
  //   }
  //   Serial.printf("Percents: Vertical: %f \t Horizontal: %f \t Out: %f \n\n", percentVertical, percentHorizontal, percentOut);
  //   tryFindAction = false;
  // }
  if (cap_turned_on(MIDDLE_FINGER) || cap_turned_on(INDEX_FINGER)) {
      //print_status();
      tryFindAction = true;
      Serial.println("Reseting Position\n");
      clearPositionAndVelocity(&position_z, &velocity_z);
      clearPositionAndVelocity(&position_x, &velocity_x);
      clearPositionAndVelocity(&position_y, &velocity_y);
  }
  if (cap_turned_off(MIDDLE_FINGER) || cap_turned_off(INDEX_FINGER)) {
      tryFindAction = false;
      //print_status();
      //Serial.println("Finger Released \n");

  }

  delay(1); // may be bad

  double rotationMagnitude;
  // rotation about Y axis
  if (detectRotation(ypr, WOBBLE_THRESHOLD, 1,0 )) {
      //Serial.println("Y-Axis Rotation Detected");
      rotationMagnitude = calculateRotationMagnitude(ypr,1,0);
      //Serial.println(magnitude);
  }
  // rotation about X-Axis
  if (detectRotation(ypr, WOBBLE_THRESHOLD, 0,0 )) {
      //Serial.println("X-Axis Rotation Detected");
      rotationMagnitude = calculateRotationMagnitude(ypr,0,0);
      //Serial.println(magnitude);
  }
  // rotation about Z-Axis
  if (detectRotation(ypr, WOBBLE_THRESHOLD, 2,0 )) {
      //Serial.println("Z-Axis Rotation Detected");
      rotationMagnitude = calculateRotationMagnitude(ypr,2,0);
      //Serial.println(magnitude);
  }
}
