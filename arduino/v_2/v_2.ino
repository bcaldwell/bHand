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


const int HISTORY_LENGTH = 150;

const int WOBBLE_THRESHOLD = 70;
const int WOBBLE_COUNT = 50;

const int VOLUME_THRESHOLD = 5000;
const int VOLUME_COUNT = 30;

const int VOLUME_DECREASE_THRESHOLD = -2000;
const int VOLUME_DECREASE_COUNT = 10;

int AX_HIST [HISTORY_LENGTH];
int AY_HIST [HISTORY_LENGTH];
int AZ_HIST [HISTORY_LENGTH];

int GX_HIST [HISTORY_LENGTH];
int GY_HIST [HISTORY_LENGTH];
int GZ_HIST [HISTORY_LENGTH];
int FilledSpot = 0;

float TEMPO = 700;
float noteStartTime;

bool isNotePlaying = false;

int channel = 1; // Defines the MIDI channel to send messages on (values from 1-16)
int velocity = 50; // Defines the velocity that the note plays at (values from 0-127)

int32_t notes_to_play [][2] ={
  {48, 1},
  {48, 1},
  {55, 1},
  {55, 1},
  {57, 1},
  {57, 1},
  {55, 2},
  {53, 1},
  {53, 1},
  {52, 1},
  {52, 1},
  {50, 1},
  {50, 1},
  {48, 2},

  {55, 1},
  {55, 1},
  {53, 1},
  {53, 1},
  {52, 1},
  {52, 1},
  {50, 2},

  {55, 1},
  {55, 1},
  {53, 1},
  {53, 1},
  {52, 1},
  {52, 1},
  {50, 2},

  {48, 1},
  {48, 1},
  {55, 1},
  {55, 1},
  {57, 1},
  {57, 1},
  {55, 2},
  {53, 1},
  {53, 1},
  {52, 1},
  {52, 1},
  {50, 1},
  {50, 1},
  {48, 2},
  {0, 10},
};

int32_t current_note [2];
int current_note_spot = 0;


struct TrapezoidRule {
  double value;
  double last_y;
  double last_x;
  double zero;
};


struct YawPitchRoll ypr;

struct TrapezoidRule velocity_x = {0,0,millis(), 1300};
struct TrapezoidRule position_x  = {0,0,millis(), 0};

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {
  // initialize serial communication
  Serial.begin(115200);

  mpu_init(mpu, INTERRUPT_PIN);
}





// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void addGyro (YawPitchRoll ypr) {
  
}

void add (int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz) {
  AX_HIST[FilledSpot] = ax;
  AY_HIST[FilledSpot] = ay;
  AZ_HIST[FilledSpot] = az;
  GX_HIST[FilledSpot] = gx;
  GY_HIST[FilledSpot] = gy;
  GZ_HIST[FilledSpot] = gz;
//  Serial.println("Start");
//  Serial.println(sizeof(GZ_HIST));
//  Serial.println(sizeof(GZ_HIST[0]));
//  Serial.println(sizeof(ax));
//  Serial.println("End");
  FilledSpot++;

  if (FilledSpot == HISTORY_LENGTH) {
    FilledSpot = 0;
  }
}


// comparator value is to determine if the threshold should be less than the value or greater than the magnitude or less than magnitude
bool DetectThreshold (int * data, int magnitude, int count_requirement, bool comparator = true) {
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
  }

  if (count > (count_requirement-1)) {
    return true;
  }
  return false;
}

float value;

void loop() {

  if(!mpu_setup()) return;


  getYawPitchRoll(&ypr);
  printYawPitchRoll(ypr);
  //HERE
  //doubleTrapExecution();

  play_note();

  // mpu.dmpGetQuaternion(&q, fifoBuffer);
  // mpu.dmpGetAccel(&aa, fifoBuffer);
  // mpu.dmpGetGravity(&gravity, &q);
  // mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
  // mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

  // Serial.printf("a-z: %d \n", aaReal.z);
  // add(aaReal.x,aaReal.y,aaReal.z,ypr[0],ypr[1],ypr[2]);
  // Serial.printf("g-z: %d \n", ypr[1]);

  if (DetectThreshold(GY_HIST,WOBBLE_THRESHOLD,WOBBLE_COUNT)) {
    enactWarble();
  } else {
    usbMIDI.sendPitchBend(8192, channel);
  }
  if (DetectThreshold(AZ_HIST,VOLUME_THRESHOLD,VOLUME_COUNT)) {
    enactVolumeIncrease();
  }
  if (DetectThreshold(AZ_HIST,VOLUME_DECREASE_THRESHOLD,VOLUME_DECREASE_COUNT,false)) {
    enactVolumeDecrease();
  }
}
