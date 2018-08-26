#pragma once

#include "ch.h"
#include "hal.h"
#include "biquad.h"
#include "hall_sensors.h"
#include "tach.h"

#ifdef __cplusplus
extern "C" {
#endif
  
  typedef struct {
    bool enabled;
    float i_target;
    uint32_t error_count;
    
    float i_a;
    biquad_t ia_compensator;
    
    uint32_t pwm_freq;
    uint32_t clock_freq;
    int32_t max_duty_count;
    PWMDriver* driver;
    ioline_t nfault;
    ioline_t fault_clear;
    ioline_t bridge_enabled;
    PWMConfig config;
    
    ADCDriver* adc;
  } motor_control_t;
  
  void MotorControlInit(motor_control_t *m,
                        pwmcallback_t cb);
  void MotorControlCb(motor_control_t* m,
                      float i_a,
                      float i_b);
  
  
#ifdef __cplusplus
}
#endif


