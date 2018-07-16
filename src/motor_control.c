#include "motor_control.h"

#include "ch.h"
#include "hal.h"
#include "biquad.h"
#include <math.h>

static const float max_duty = 0.95f;
static const float adc_sample_duty = 0.97f;

static const float inv_sqrt3 = 0.5773502692f;
static const float sqrt3 = 1.7320508076f;
#define MIN(x,y) ((x)<(y)?(x):(y))

static alphabeta_t clarke(float a, float b) {
  const alphabeta_t ret =  { .alpha = a,
                             .beta = inv_sqrt3 * (a + b + b) };
  return ret;
}

static dq_t park(alphabeta_t ab, float sin_theta, float cos_theta) {
  const dq_t dq = { .d = ab.alpha * cos_theta + ab.beta * sin_theta,
                    .q = ab.beta * cos_theta - ab.alpha * sin_theta };
  return dq;
}

static alphabeta_t inv_park(dq_t dq, float sin_theta, float cos_theta) {
  const alphabeta_t ab = { .alpha = dq.d * cos_theta - dq.q * sin_theta,
                           .beta  = dq.q * cos_theta + dq.d * sin_theta };
  
  return ab;
}

void MotorControlInit(motor_control_t *m, pwmcallback_t cb) {
  
  // Configure biquads
  BiquadInit(&m->iq_compensator, 0.012169901f, 0.0015820871f, -0.010587814f, -1.76f, 0.76f);
  BiquadInit(&m->id_compensator, 0.012169901f, 0.0015820871f, -0.010587814f, -1.76f, 0.76f);
  
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

int theta_counter = 0;

void MotorControlCb(motor_control_t* m,
                    float i_a,
                    float i_b) {
  // Confirm that we've received a new ADC value since last cycle
  if(!(ADC1->SR & ADC_SR_JEOC))
    m->error_count++;
  ADC1->SR = 0;
  
  const float sin_theta = sinf(theta_counter / 10000.f);
  const float cos_theta = cosf(theta_counter / 10000.f);
  
  const alphabeta_t i_ab = clarke(i_a, i_b);
  const dq_t i_dq = park(i_ab, sin_theta, cos_theta);
  
  const float id_error = 0.f - i_dq.d;
  const float iq_error = m->i_target - i_dq.q;
  
  if(m->enabled) {
    // Iterate the compensator
    m->max_duty_count = 300;
    const dq_t vdq = { .d = BiquadUpdateLim(&m->id_compensator, id_error, -m->max_duty_count, m->max_duty_count),
                       .q = BiquadUpdateLim(&m->iq_compensator, iq_error, -m->max_duty_count, m->max_duty_count) };
    
    // Transform back to abc space
    const alphabeta_t vab = inv_park(vdq, sin_theta, cos_theta);
    const int16_t va = vab.alpha;
    const int16_t vb = (-vab.alpha + sqrt3 * vab.beta) * 0.5f;
    const int16_t vc = (-vab.alpha - sqrt3 * vab.beta) * 0.5f;
    const int16_t min_v = MIN(MIN(va, vb), vc);
    
    // Configure PWM channel
    m->driver->tim->CCR[0] = va - min_v;
    m->driver->tim->CCR[1] = vb - min_v;
    m->driver->tim->CCR[2] = vc - min_v;
    
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
    BiquadReset(&m->id_compensator);
    BiquadReset(&m->iq_compensator);
  }
  
  m->i_a = i_a;
  m->i_b = i_b;
}
