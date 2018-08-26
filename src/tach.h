#pragma once

#include "hall_sensors.h"

typedef struct {
  int current_segment;
  int counter;
} tach_t;

void TachInit(tach_t*, hall_sensors_state_e);
int TachUpdate(tach_t*, hall_sensors_state_e);
