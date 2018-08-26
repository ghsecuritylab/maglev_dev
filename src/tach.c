#include "tach.h"
#include "hall_sensors.h"

void TachInit(tach_t* tach, hall_sensors_state_e initial_state) {
  tach->current_segment = HallSensorsStateToSegment(initial_state);
}

int TachUpdate(tach_t* tach, hall_sensors_state_e current_state) {
  int new_segment = HallSensorsStateToSegment(current_state);
  
  int segment_diff = new_segment - tach->current_segment;
  
  if(segment_diff > 2)
    segment_diff -= 6;
  
  if(segment_diff < -2)
    segment_diff += 6;
  
  tach->counter += segment_diff;
  tach->current_segment = new_segment;
  
  return tach->counter;
}
