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
  m->config.channels[3].mode = PWM_OUTPUT_ACTIVE_HIGH;
  
  // Activate the timer
  pwmStart(m->driver, (void*)&m->config);
  pwmEnableChannel(m->driver, 3, m->config.period / 2);
  pwmEnablePeriodicNotification(m->driver);
  
}

void MotorControlCb(volatile motor_control_t* m,
                    float i_a,
                    float i_b) {
  // Confirm that we've received a new ADC value since last cycle
  if(!(ADC1->SR & ADC_SR_JEOC))
    m->error_count++;
  ADC1->SR = 0;
  
  m->i_a = i_a;
  m->i_b = i_b;
  
  
  if(m->enabled) {
    // Configure PWM channel
    m->driver->tim->CCR[0] = 200;
    m->driver->tim->CCR[1] = 0;
    m->driver->tim->CCR[2] = 0;
    
    // Enable PWM driver
    palWriteLine(m->fault_clear, PAL_HIGH);
  } else {
    // Disable PWM driver
    palWriteLine(m->fault_clear, PAL_LOW);
    
    // Configure PWM channels
    m->driver->tim->CCR[0] = 0;
    m->driver->tim->CCR[1] = 0;
    m->driver->tim->CCR[2] = 0;
  }
}
