#include "hall_sensors.h"
#include "math.h"

void HallSensorsInit(hall_sensors_t* hs) {
  (void)hs;
}

hall_sensors_state_e HallSensorsGetState(hall_sensors_t* hs) {
  return (palReadLine(hs->a) ? 0b001 : 0b000) |
         (palReadLine(hs->b) ? 0b010 : 0b000) |
         (palReadLine(hs->c) ? 0b100 : 0b000);
}

int HallSensorsStateToSegment(hall_sensors_state_e state) {
  switch(state) {
    case HALL_SENSOR_STATE_60:
      return 1;
    case HALL_SENSOR_STATE_120:
      return 2;
    case HALL_SENSOR_STATE_180:
      return 3;
    case HALL_SENSOR_STATE_240:
      return 4;
    case HALL_SENSOR_STATE_300:
      return 5;
    case HALL_SENSOR_STATE_LOW:
    case HALL_SENSOR_STATE_HIGH:
    case HALL_SENSOR_STATE_0:
    default:
      return 0;
  }
}

float HallSensorsStateToAngle(hall_sensors_state_e state) {
  return HallSensorsStateToSegment(state) * (float)(M_PI / 3.f);
}
