// const int CHANNEL = 1;
//
// const int CC_SQUARE_WAVE = 1;
// const int CC_HIGH_PASS = 2;
// const int CC_BAND_PASS = 3;
// const int CC_LOW_PASS = 4;
// const int CC_VOLUME = 5;
// const int CC_REVERB = 6;
//
// // channel 1-16
// // controlChange 1-127
// // each control change 1-127
//
//
// void sendControlChange(int CC, int magnitude, int32_t channel) {
//   usbMIDI.sendControlChange(CC,magnitude,channel);
// }


void turnOnChannel (int CC,int value, int channel) {
  usbMIDI.sendControlChange(CC,value,channel);
}

void turnOffChannel (int CC,int value, int channel) {
  usbMIDI.sendControlChange(CC,value,channel);
}



void enactVolumeIncrease() {
  Serial.println("Volume Increase");
  if (random(0,20) > 15) {
    velocity += 1;
    velocity = min(velocity, 127);
  }
}

// void play_note()
// {
//   //Serial.println(velocity);
//   usbMIDI.sendNoteOn(current_note[0],velocity,channel); // Turn the note ON
//
//   if (!isNotePlaying) {
//       //Serial.println(velocity);
//       usbMIDI.sendNoteOn(current_note[0],velocity,channel); // Turn the note ON
//       noteStartTime = millis();
//       isNotePlaying = true;
//   }
//   else if (millis() - noteStartTime > TEMPO * current_note[1]) {
//       usbMIDI.sendNoteOff(current_note[0],0,channel); // Turn the note OFF - don't forget to do this ;)
//       isNotePlaying = false;
//       current_note_spot++;
//       current_note[0] = notes_to_play[current_note_spot][0]; // hacky but c doesn't like equating array sections (odd I know. I thought pointers would be fine!)
//       current_note[1] = notes_to_play[current_note_spot][1];
//       usbMIDI.sendPitchBend(8192, channel);
//   }
// }

double value;
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
