#include "var.h"

// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================
// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;

float euler[3];         // [psi, theta, phi]    Euler angle container

const double GRAVITY_THRESHOLD = 1.5;
const double HORIZONTAL_THRESHOLD = 1.5;
const double OUT_THRESHOLD = 1.5;


const double SCALING_FACTOR = 126.5;
const double BOOLES_STUPID_NUM = -500; // not needed unless we do booles integration method

const double MAX_VERTICAL = 1.5;
const double MAX_HORIZONTAL = 1.5;
const double MAX_OUT = 3.2;


volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
  mpuInterrupt = true;
}

void mpu_init(MPU6050 mpu, int interrupt_pin){
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties

  // initialize device
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();
  pinMode(interrupt_pin, INPUT);

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  Serial.println("");

  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip


  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
    Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
    attachInterrupt(digitalPinToInterrupt(interrupt_pin), dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }
}

bool mpu_setup(){
  // if programming failed, don't try to do anything
  if (!dmpReady) return false;

  // wait for MPU interrupt or extra packet(s) available
  while (!mpuInterrupt && fifoCount < packetSize) {
    // other program behavior stuff here
    // .
    // .
    // .
    // if you are really paranoid you can frequently test in between other
    // stuff to see if mpuInterrupt is true, and if so, "break;" from the
    // while() loop to immediately process the MPU data
    // .
    // .
    // .
  }

  // reset interrupt flag and get INT_STATUS byte
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  fifoCount = mpu.getFIFOCount();

  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();
    Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;
  }
  return true;
}

double smoothCount = 5;
void getLinearAccel(Acceleration * accel){
  accel -> x = 0;
  accel -> y = 0;
  accel -> z = 0;
  for (int readCount = 0; readCount < smoothCount; readCount++) {

    mpu_setup();

    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetAccel(&aa, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);

    accel -> x += (1/smoothCount)*(double)aaReal.x/SCALING_FACTOR;
    accel -> y += (1/smoothCount)*(double)aaReal.y/SCALING_FACTOR;
    accel -> z += (1/smoothCount)*(double)aaReal.z/SCALING_FACTOR;
  }
  if ( (accel -> z) > (GRAVITY_FACTOR - GRAVITY_THRESHOLD) && (accel -> z) < (GRAVITY_FACTOR + GRAVITY_THRESHOLD)) {
    accel -> z = GRAVITY_FACTOR;
  }
  if ( (accel -> x) > (HORIZONTAL_FACTOR - HORIZONTAL_THRESHOLD) && (accel -> x) < (HORIZONTAL_FACTOR + HORIZONTAL_THRESHOLD)) {
    accel -> x = HORIZONTAL_FACTOR;
  }
  if ( (accel -> y) > (OUT_FACTOR - OUT_THRESHOLD) && (accel -> y) < (OUT_FACTOR + OUT_THRESHOLD)) {
    accel -> y = OUT_FACTOR;
  }
}

// bool checkResetPosition (double arr [], double numberOfOccurences, double cutoffVal, int size=HISTORY_LENGTH) {
//   int count = 0;
//   for (int i = 0; i < HISTORY_LENGTH; i++) {
//     if (arr[i] == cutoffVal) {
//       count++;
//     }
//   }
//   if (count > numberOfOccurences) {
//     return true;
//   }
//   return false;
// }

void clearPositionAndVelocity (TrapezoidRule * position, TrapezoidRule * velocity) {
  position -> value = 0;
  position -> last_y = 0;
  velocity -> value = 0;
  velocity -> last_y = 0;

}




void getWorldAccel(Acceleration * accel){

  accel -> x = 0;
  accel -> y = 0;
  accel -> z = 0;
  for (int readCount = 0; readCount < smoothCount; readCount++) {

    mpu_setup();

    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetAccel(&aa, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);

    accel -> x += (1/smoothCount)*(double)aaReal.x/SCALING_FACTOR;
    accel -> y += (1/smoothCount)*(double)aaReal.y/SCALING_FACTOR;
    accel -> z += (1/smoothCount)*(double)aaReal.z/SCALING_FACTOR;
  }

  if ( (accel -> z) > (GRAVITY_FACTOR - GRAVITY_THRESHOLD) && (accel -> z) < (GRAVITY_FACTOR + GRAVITY_THRESHOLD)) {
    accel -> z = GRAVITY_FACTOR;
  }
  if ( (accel -> x) > (HORIZONTAL_FACTOR - HORIZONTAL_THRESHOLD) && (accel -> x) < (HORIZONTAL_FACTOR + HORIZONTAL_THRESHOLD)) {
    accel -> x = HORIZONTAL_FACTOR;
  }
  if ( (accel -> y) > (OUT_FACTOR - OUT_THRESHOLD) && (accel -> y) < (OUT_FACTOR + OUT_THRESHOLD)) {
    accel -> y = OUT_FACTOR;
  }
}

void printAccel(Acceleration accel){
  Serial.print("Acceleration \t");
  Serial.print(accel.x);
  Serial.print("\t");
  Serial.print(accel.y);
  Serial.print("\t");
  Serial.println(accel.z);
}

void printPosition(TrapezoidRule position){
  Serial.print("Position: \t");
  Serial.printf("%f",position.value);
  Serial.print("\n");
}
void printPositions(TrapezoidRule position_x, TrapezoidRule position_y, TrapezoidRule position_z){
  Serial.print("Position: \t");
  Serial.printf("x: %f \t",position_x.value);
  Serial.printf("y: %f \t",position_y.value);
  Serial.printf("z: %f \t",position_z.value);
  Serial.print("\n");
}

void getYawPitchRoll(YawPitchRoll * ypr){
  float raw[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetYawPitchRoll(raw, &q, &gravity);

  ypr -> yaw = raw[0] * 180 / M_PI;
  ypr -> pitch = raw[1] * 180 / M_PI;
  ypr -> roll = raw[2] * 180 / M_PI;
}

void printYawPitchRoll(YawPitchRoll ypr){
  Serial.print("ypr\t");
  Serial.print(ypr.yaw);
  Serial.print("\t");
  Serial.print(ypr.pitch);
  Serial.print("\t");
  Serial.println(ypr.roll);
}


void doubleTrapExecution(double accelVal, TrapezoidRule * velocity, TrapezoidRule * position) {
  unsigned long cur_time = millis();

  TrapezoidApproximate(velocity, cur_time, accelVal);
  TrapezoidApproximate(position, cur_time, velocity -> value);
}

void TrapezoidApproximate(TrapezoidRule * t, unsigned long new_x, double new_y) {
  // Serial.printf("Last y value %f \n", t->last_y);
  // Serial.printf("New Y Value %f \n", new_y);
  // Serial.printf("Time Difference Value %d \n", new_x - t->last_x);
  //double val = 0.5*(new_x - (t->last_x))*((new_y - t->zero) + (t->last_y - t->zero));
  Serial.printf("Difference in new val: %f and Difference in last val %f\n", (new_y - t->zero), (t->last_y - t->zero));
   t->value += 0.0005*(new_x - (t->last_x))*((new_y - t->zero) + (t->last_y - t->zero)); // 0.0005 is 0.5 / 1000 (ms)
   //Serial.printf("Summation: %f\n", t->value);
   t->last_y = new_y; // the value of the derivative of the last value
   t->last_x = new_x;
}

double amountVertical (double position) {
  return position/MAX_VERTICAL;
}
double amountHorizontal (double position) {
  return position/MAX_HORIZONTAL;
}
double amountOut (double position) {
  return position/MAX_OUT;
}

void TrapezoidZero(TrapezoidRule * t, double y) {
  t -> zero = y;
  t -> value = 0;
}

// int detectHandRaised (double position) {
//
// }




bool CheckBoolesIsFull (BoolesRule * data) {
  if (data -> first_y != BOOLES_STUPID_NUM && data -> second_y != BOOLES_STUPID_NUM && data -> third_y != BOOLES_STUPID_NUM && data -> fourth_y != BOOLES_STUPID_NUM && data -> fifth_y != BOOLES_STUPID_NUM) {
    return true;
  }
  return false;
}

void assignBoolesVal (BoolesRule * data, double y, double x ) {
  if (data -> fifth_y == BOOLES_STUPID_NUM) {
    data -> fifth_y = y;
    data -> fifth_x = x;
  } else if (data -> fourth_y == BOOLES_STUPID_NUM) {
    data -> fourth_y = y;
  } else if (data -> third_y == BOOLES_STUPID_NUM) {
    data -> third_y = y;
  } else if (data -> second_y == BOOLES_STUPID_NUM) {
    data -> second_y = y;
  } else if (data -> first_y == BOOLES_STUPID_NUM) {
    data -> first_y = y;
  } else {
    Serial.println("BOOLES INTEGRATION FUCKED UP");
  }
}

void clearBoolesYVal (BoolesRule * data) {
  data -> fifth_y = BOOLES_STUPID_NUM;
  data -> fourth_y = BOOLES_STUPID_NUM;
  data -> third_y = BOOLES_STUPID_NUM;
  data -> second_y = BOOLES_STUPID_NUM;
  data -> first_y = BOOLES_STUPID_NUM;
}

void doubleBoolesExecution(double accelVal, BoolesRule * velocity, BoolesRule * position) {
  unsigned long cur_time = millis();

  assignBoolesVal(velocity, accelVal,cur_time);

  // queue for velocity is full. Have all 5 values
  if (CheckBoolesIsFull(velocity)) {
    BoolesApproximate(velocity, cur_time);
    // add new values as a y for the position
    assignBoolesVal(position, velocity-> value, cur_time);
    // check if position has all 5 values
    if (CheckBoolesIsFull(position)) {
      BoolesApproximate(position, cur_time);
      clearBoolesYVal(position);
    }
    clearBoolesYVal(velocity);
  }
}

void BoolesApproximate(BoolesRule * b, unsigned long new_x) {
  // add values
  b->value += (2*(new_x - b->fifth_x)/45)*(7*b->fifth_y + 32*b->fourth_y + 12*b->third_y + 32*b->second_y + 7*b->first_y);
}
