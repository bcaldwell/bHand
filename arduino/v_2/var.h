#ifndef VAR
#define VAR

// #define DEBUG 1

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

#endif //VAR
