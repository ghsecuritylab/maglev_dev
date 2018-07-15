#include "biquad.h"
#include <float.h>

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

void BiquadInit(biquad_t* b, float b0, float b1, float b2, float a1, float a2) {
  b->b0 = b0;
  b->b1 = b1;
  b->b2 = b2;
  b->a1 = a1;
  b->a2 = a2;
  
  BiquadReset(b);
}

void BiquadReset(biquad_t* b) {
  b->in1 = b->in2 = b->out1 = b->out2 = 0.f;
}

float BiquadUpdateLim(biquad_t* b, float input, float lim_low, float lim_high) {
  const float out_raw = b->b0 * input
                      + b->b1 * b->in1
                      + b->b2 * b->in2
                      - b->a1 * b->out1
                      - b->a2 * b->out2;
  const float out = MIN(MAX(out_raw, lim_low), lim_high);
  
  b->in2 = b->in1;
  b->in1 = input;
  
  b->out2 = b->out1;
  b->out1 = out;
  
  return out;
}

float BiquadUpdate(biquad_t* b, float input) {
  return BiquadUpdateLim(b, input, FLT_MIN, FLT_MAX);
}
