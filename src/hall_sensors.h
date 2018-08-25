#pragma once

#include "ch.h"
#include "hal.h"

typedef struct {
  ioline_t a, b, c;
} hall_sensors_t;

typedef enum {
  HALL_SENSOR_STATE_LOW  = 0b000,
  HALL_SENSOR_STATE_0    = 0b001,
  HALL_SENSOR_STATE_60   = 0b101,
  HALL_SENSOR_STATE_120  = 0b100,
  HALL_SENSOR_STATE_180  = 0b110,
  HALL_SENSOR_STATE_240  = 0b010,
  HALL_SENSOR_STATE_300  = 0b011,
  HALL_SENSOR_STATE_HIGH = 0b111
} hall_sensors_state_e;

void HallSensorsInit(hall_sensors_t*);
hall_sensors_state_e HallSensorsGetState(hall_sensors_t*);
float HallSensorsGetAngle(hall_sensors_t*);

