#include "motor_control.h"

#include "ch.h"
#include "hal.h"
#include "biquad.h"
#include <math.h>
#include <string.h>

#include "freq_response.h"
#include "lwip/udp.h"

static const float max_duty = 0.95f;
static const float adc_sample_duty = 0.97f;

static const float inv_sqrt3 = 0.5773502692f;
static const float sqrt3 = 1.7320508076f;
#define MIN(x,y) ((x)<(y)?(x):(y))

static struct udp_pcb* freq_pcb;
static struct pbuf* freq_pb;

// static freq_response_t ia_freq = { .fs = 35e3, .f_min = 10.f, .f_max = 35e3/2, .frequencies_per_decade = 20.f, .wavelengths_per_frequency = 20, .magnitude = 1.f };
static freq_response_t ia_freq = { .fs = 35e3, .f_min = 400.f, .f_max = 400.f, .frequencies_per_decade = 20.f, .wavelengths_per_frequency = 20, .magnitude = 0.2f };

void fr_emit(freq_response_t* fr, const freq_response_result_t* result) {
  (void)fr;
  
  memcpy(freq_pb->payload, result, sizeof(freq_response_result_t));
  freq_pb->len = freq_pb->tot_len = sizeof(freq_response_result_t);
  udp_sendto(freq_pcb, freq_pb, IP_ADDR_BROADCAST, 10100);
}

THD_WORKING_AREA(mcTask, 2048);
static THD_FUNCTION(_MCTask, arg) {
  (void)arg;

  chThdSleep(TIME_MS2I(2200));
  
  freq_pcb = udp_new();
  udp_bind(freq_pcb, IP_ADDR_ANY, 10100);
  ip_set_option(freq_pcb, SOF_BROADCAST);
  freq_pb = pbuf_alloc(PBUF_TRANSPORT, 512, PBUF_RAM);
  
  ia_freq.emit = fr_emit;
  
  while(true) {
    FreqResponse_Poll(&ia_freq);
    chThdYield();
  }
}


void MotorControlInit(motor_control_t *m, pwmcallback_t cb) {
  
  // Configure biquads
  // Poles at 0.4 and 1.0
  // Zeros at 0.93 and -1.0
  // Gain 1300 at 1kHz (35kHz switching frequency)
  BiquadInit(&m->ia_compensator, 163.85044f, 49.155133f, -114.69531f, -1.4f, 0.4f);
  
  // Set up a task to configure and poll the loop gain measurement
  chThdCreateStatic(mcTask, sizeof(mcTask), LOWPRIO, _MCTask, NULL);
  
  // Initialize config
  m->config.frequency = m->clock_freq;
  m->config.period = (m->clock_freq / m->pwm_freq);
  m->config.callback = cb;
  m->config.channels[0].mode = PWM_OUTPUT_ACTIVE_HIGH;
  m->config.channels[1].mode = PWM_OUTPUT_ACTIVE_HIGH;
  m->config.channels[2].mode = PWM_OUTPUT_ACTIVE_HIGH;
  m->config.channels[3].mode = PWM_OUTPUT_ACTIVE_HIGH;
  
  m->max_duty_count = m->config.period * max_duty;
  
  // Activate the timer
  pwmStart(m->driver, (void*)&m->config);
  pwmEnableChannel(m->driver, 3, m->config.period * adc_sample_duty);
  pwmEnablePeriodicNotification(m->driver);
  
}

void MotorControlCb(motor_control_t* m,
                    float i_a,
                    float i_b) {
  (void)i_b;
  
  // Confirm that we've received a new ADC value since last cycle
  if(!(ADC1->SR & ADC_SR_JEOC))
    m->error_count++;
  ADC1->SR = 0;
  
  if(m->enabled) {
    // Iterate the compensator
    // const float ia_error = m->i_target - i_a;
    const float ia_error = FreqResponse_Update(&ia_freq, m->i_target - i_a);
    const float va = BiquadUpdateLim(&m->ia_compensator, ia_error, -m->max_duty_count, m->max_duty_count);
    
    // Configure PWM channel
    m->driver->tim->CCR[0] = va;
    
    // Enable PWM driver
    palWriteLine(m->fault_clear, PAL_HIGH);
  } else {
    // Disable PWM driver
    palWriteLine(m->fault_clear, PAL_LOW);
    
    // Configure PWM channels
    m->driver->tim->CCR[0] = 0;
    m->driver->tim->CCR[1] = 0;
    m->driver->tim->CCR[2] = 0;
    
    // Reset the compensators
    BiquadReset(&m->ia_compensator);
  }
  
  m->i_a = i_a;
}
