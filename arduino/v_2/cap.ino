#include "Adafruit_MPR121.h"

Adafruit_MPR121 cap = Adafruit_MPR121();

uint16_t lasttouched = 0;
uint16_t currtouched = 0;

void cap_init(){
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D

  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 connection failed");
  } else {
    Serial.println("MPR121 connection succesful");
  }
  cap.setThreshholds(3,2);
}

uint16_t cap_read() {
  lasttouched = currtouched;
  currtouched = cap.touched();
  return currtouched;
}

void print_status() {
  Serial.printf("Current Touched: %d  \t Last Touched %d \n", currtouched, lasttouched);
}

bool any_touched() {
  return currtouched != 0;
}

bool cap_touched(int pad) {
  return (currtouched & _BV(pad));
}

bool cap_turned_on(int pad) {
  return (cap_touched(pad) && !(lasttouched & _BV(pad)));
}

bool cap_turned_off(int pad) {
  return (!cap_touched(pad) && (lasttouched & _BV(pad)));
}
