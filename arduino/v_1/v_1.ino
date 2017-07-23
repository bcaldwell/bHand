/*
   MPU6050_raw.ino : example of reading raw IMU data from MPU6050 using Teensy 3.X or Teensy LC

   This file is part of MPU6050.

   MPU6050 is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   Hackflight is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with Hackflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MPU6050.h"
const int HISTORY_LENGTH = 150;

const int WOBBLE_THRESHOLD = 115;
const int WOBBLE_COUNT = 50;

const int VOLUME_THRESHOLD = 12;
const int VOLUME_COUNT = 30; 

const int VOLUME_DECREASE_THRESHOLD = -10;
const int VOLUME_DECREASE_COUNT = 20; 

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



MPU6050 imu;

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

void setup()
{
    Serial.begin(115200);
    
    Serial.println("Reached Setup");
    current_note[0] = notes_to_play[0][0];
    current_note[1] = notes_to_play[0][1];
    Serial.println("Finished Assignment");

    
    Wire.begin();
        Serial.println("3");

 
    if (!imu.begin(AFS_2G, GFS_250DPS)) {
        Serial.println("MPU6050 is online...");
    }
    else {
        Serial.println("Failed to init MPU6050");
        while (true) 
            ;
    }

    usbMIDI.sendControlChange(101,0, channel);
    usbMIDI.sendControlChange(100,0, channel);
    usbMIDI.sendControlChange(6,10, channel);
    usbMIDI.sendControlChange(38,0, channel);
    
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



void loop()
{  
    play_note();

    int16_t ax, ay, az, gx, gy, gz;
    float tscale = 9.81/16384;
    float rscale = 1.0/131.0;

    
    
    if (imu.getMotion6Counts(&ax, &ay, &az, &gx, &gy, &gz)) {
        ax = ax * tscale;
        ay = ay * tscale;
        az = az * tscale;
        gx = gx * rscale;
        gy = gy * rscale;
        gz = gz * rscale;

        Serial.println("Z Acceleration");
        Serial.println(az);

        add (ax, ay, az, gx, gy, gz);

        if (DetectThreshold(GY_HIST,WOBBLE_THRESHOLD,WOBBLE_COUNT)) {
          enactWarble();
        } else {
          usbMIDI.sendPitchBend(8192, channel);
        }
        if (DetectThreshold(AZ_HIST,VOLUME_THRESHOLD,VOLUME_COUNT)) {
          enactVolumeIncrease();
        }
        

//        if (DetectWobble()) {
//          enactWarble();
//        } else {
//            usbMIDI.sendPitchBend(8192, channel);
//        }
//        // function name should be based off user action, not intended effect
//        if (DetectVolumeDecrease()){
//           enactVolumeDecrease();
//        }
//        else {
//         // Serial.println(":(");
//          //Serial.println(gy);
//        }

        
//        if (DetectVolumeChange()){
//           enactVolumeChange();
//        }
//        else {
//         // Serial.println(":(");
//          //Serial.println(gy);
//        }
        
//          Serial.print(ax * tscale);
//          Serial.print(" ");
//          Serial.print(ay * tscale);
//          Serial.print(" ");
//          Serial.print(az * tscale);
//          Serial.print(" ");
//          Serial.print(gx * rscale);
//          Serial.print(" ");
//          Serial.print(gy * rscale);
//          Serial.print(" ");
//          Serial.print(gz * rscale);
//          Serial.println();
          
      }    
    delay(2);
}

float value;

void enactWarble() {
  value = sin((millis() - noteStartTime)/250.0 * 2.0 * PI);
  usbMIDI.sendPitchBend(8192 + (value * 4000), channel);
  Serial.println("Wobble enacted 5");
}

void enactVolumeDecrease() {
  Serial.println("Volume Decrease");
  if (random(0,20) > 15) {
    velocity -= 1;
    velocity = min(velocity, 0);
  }
}

void enactVolumeIncrease() {
  Serial.println("Volume Increase");
  if (random(0,20) > 15) {
    velocity += 1;
    velocity = min(velocity, 127);
  }
}

void play_note()
{
  Serial.println(velocity);
  usbMIDI.sendNoteOn(current_note[0],velocity,channel); // Turn the note ON

  if (!isNotePlaying) {
      Serial.println(velocity);
      usbMIDI.sendNoteOn(current_note[0],velocity,channel); // Turn the note ON
      noteStartTime = millis(); 
      isNotePlaying = true;
  }
  else if (millis() - noteStartTime > TEMPO * current_note[1]) {
      usbMIDI.sendNoteOff(current_note[0],0,channel); // Turn the note OFF - don't forget to do this ;)
      isNotePlaying = false;
      current_note_spot++;
      current_note[0] = notes_to_play[current_note_spot][0]; // hacky but c doesn't like equating array sections (odd I know. I thought pointers would be fine!)
      current_note[1] = notes_to_play[current_note_spot][1];
      usbMIDI.sendPitchBend(8192, channel);
  }
}

//bool DetectWobble () {
//  int count = 0;
//  for (int i = 0; i < HISTORY_LENGTH; i++ ) {
//    if (GY_HIST[i] > WOBBLE_THRESHOLD) {
//      count++;
//    }
//  }
//
//  if (count > WOBBLE_COUNT) {
//    return true;
//  }
//  return false;
//  
//}

//bool DetectVolumeDecrease () {
//  int count = 0;
//  for (int i = 0; i < HISTORY_LENGTH; i++ ) {
//    if (AZ_HIST[i] < VOLUME_DECREASE_THRESHOLD) {
//      count++;
//    }
//  }
//
//  if (count > VOLUME_COUNT) {
//    return true;
//  }
//  return false;
//}
//
//bool DetectVolumeChange () {
//  int count = 0;
//  for (int i = 0; i < HISTORY_LENGTH; i++ ) {
//    if (AZ_HIST[i] > VOLUME_THRESHOLD) {
//      count++;
//    }
//  }
//
//  if (count > VOLUME_COUNT) {
//    return true;
//  }
//  return false;
//}

