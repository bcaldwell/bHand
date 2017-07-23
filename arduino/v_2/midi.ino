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

QueueArray <MidiControl> TimerQueue;
QueueArray <MidiControl> TouchQueue;

MidiControl currentEffect;
MidiControl currentTimer;
MidiControl currentTouch;

void turnOnEffect (int finger, int direction) {
  // index is wired to 2 but mapped to 0

  currentEffect = Mapping[finger - 2][direction];

  // if (finger == INDEX_FINGER || finger == MIDDLE_FINGER) {
  TurnOnTimerEffect(&currentEffect);
  // } else {
    // TurnOnTouchEffect(currentEffect);
  // }
}

void TurnOnTimerEffect(MidiControl * effect) {
  toggleEffect(effect);
  effect -> off_time = millis() + 5000;
  TimerQueue.enqueue(*effect);
}

void TurnOnTouchEffect(MidiControl effect) {
  toggleEffect(&effect);
  TouchQueue.enqueue(effect);
}

void toggleEffect(MidiControl * effect) {
  int value;
  if (effect -> is_on) {
    value = effect -> off;
    effect -> is_on = false;
  } else {
    value = effect -> on;
    effect -> is_on = true;
  }

  usbMIDI.sendControlChange(effect -> control_change,value,effect -> channel);
}

void clearTouchEffect() {
  if (TouchQueue.isEmpty()) return;
  currentTouch = TouchQueue.dequeue();
  toggleEffect(&currentTouch);
}

void clearTimerEffects() {
  unsigned long cur_time = millis();
  while (!TimerQueue.isEmpty()) {
    currentTimer = TimerQueue.front();
    if (currentTimer.off_time > cur_time) return;
    toggleEffect(&currentTimer);
    TimerQueue.dequeue();
  }
}
