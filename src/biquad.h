#pragma once

typedef struct {
  float b0, b1, b2;
  float a1, a2;
  
  float in1, in2;
  float out1, out2;
} biquad_t;

void BiquadInit(biquad_t*, float b0, float b1, float b2, float a1, float a2);
void BiquadReset(biquad_t*);
float BiquadUpdateLim(biquad_t*, float input, float lim_low, float lim_high);
float BiquadUpdate(biquad_t*, float input);
