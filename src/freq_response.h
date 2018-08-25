#pragma once

typedef struct freq_response_result freq_response_result_t;
typedef struct freq_response freq_response_t;

typedef void (*FreqResponse_Emit)(freq_response_t*, const freq_response_result_t*);

void FreqResponse_Init(freq_response_t*);
void FreqResponse_Poll(freq_response_t*);
float FreqResponse_Update(freq_response_t*, float sample);
float FreqResponse_UpdateWithOffset(freq_response_t*, float sample, float offset);

struct freq_response_result {
  float freq;
  float real;
  float imag;
  float exc_real;
  float exc_imag;
};

struct freq_response {
  float fs;
  float f_min, f_max;
  float frequencies_per_decade;
  float magnitude;
  int wavelengths_per_frequency;
  FreqResponse_Emit emit;
  
  float f;
  float theta, theta_increment;
  float normalization_factor;
  int samples_remaining;
  
  float sin_sum, cos_sum;
  float sin_exc_sum, cos_exc_sum;
};
