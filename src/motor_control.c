#include "motor_control.h"

#include "ch.h"
#include "hal.h"


void MotorControlInit(volatile motor_control_t *m, pwmcallback_t cb) {
  
  // Initialize config
  m->config.frequency = m->clock_freq;
  m->config.period = (m->clock_freq / m->pwm_freq);
  m->config.callback = cb;
  m->config.channels[0].mode = PWM_OUTPUT_ACTIVE_HIGH;
  m->config.channels[1].mode = PWM_OUTPUT_ACTIVE_HIGH;
  m->config.channels[2].mode = PWM_OUTPUT_ACTIVE_HIGH;
  
  // Activate the timer
  pwmStart(m->driver, (void*)&m->config);
  pwmEnablePeriodicNotification(m->driver);
  
}

void MotorControlCb(volatile motor_control_t* m) {
  // Read ADC values
  //
  
  //
  
  if(m->enabled) {
    // Configure PWM channel
    pwmEnableChannel(m->driver, 0, 200);
    pwmEnableChannel(m->driver, 1, 0);
    pwmEnableChannel(m->driver, 2, 0);
    
    // Enable PWM driver
    palWriteLine(m->fault_clear, PAL_HIGH);
  } else {
    // Disable PWM driver
    palWriteLine(m->fault_clear, PAL_LOW);
    
    // Configure PWM channels
    pwmEnableChannel(m->driver, 0, 0);
    pwmEnableChannel(m->driver, 1, 0);
    pwmEnableChannel(m->driver, 2, 0);
  }
}
