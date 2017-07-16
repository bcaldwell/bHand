#ifndef VAR
#define VAR

// #define DEBUG 1


const int HISTORY_LENGTH = 20;

const double AZ_THRESHOLD_UP = 40;
const double AZ_THRESHOLD_DOWN = -30;

const int AZ_COUNT = 1;
const double AX_THRESHOLD_RIGHT = 40;
const double AX_THRESHOLD_LEFT = -40;
const int AX_COUNT = 1;

const int CC_HIGH_PASS= 1;
const int CC_LOW_PASS= 2;
const int FILTER_ON = 100;
const int FILTER_OFF = 2;
const int MAIN_CHANNEL = 1;


struct YawPitchRoll {
  double yaw;
  double pitch;
  double roll;
};

struct Acceleration {
  double x;
  double y;
  double z;
};

struct TrapezoidRule {
  double value;
  double last_y;
  unsigned long last_x;
  double zero;
};

struct BoolesRule {
  double value;
  double first_y;
  double second_y;
  double third_y;
  double fourth_y;
  double fifth_y;
  unsigned long first_x;
  unsigned long second_x;
  unsigned long third_x;
  unsigned long fourth_x;
  unsigned long fifth_x;

  double zero;
};

struct MidiControl {
  int channel;
  int control_change;
  int on;
  int off;

  bool is_on;

  unsigned long off_time;
};

#endif //VAR
