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

float HallSensorsGetAngle(hall_sensors_t* hs) {
  hall_sensors_state_e state = HallSensorsGetState(hs);
  switch(state) {
    case HALL_SENSOR_STATE_60:
      return 60.f * (float)(M_PI / 180.f);
    case HALL_SENSOR_STATE_120:
      return 120.f * (float)(M_PI / 180.f);
    case HALL_SENSOR_STATE_180:
      return 180.f * (float)(M_PI / 180.f);
    case HALL_SENSOR_STATE_240:
      return 240.f * (float)(M_PI / 180.f);
    case HALL_SENSOR_STATE_300:
      return 300.f * (float)(M_PI / 180.f);
    case HALL_SENSOR_STATE_LOW:
    case HALL_SENSOR_STATE_HIGH:
    case HALL_SENSOR_STATE_0:
    default:
      return 0.f * (float)(M_PI / 180.f);
  }
}
