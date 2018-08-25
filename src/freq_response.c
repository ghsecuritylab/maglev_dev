#include "freq_response.h"
#include "math.h"

void FreqResponse_Init(freq_response_t* fr) {
  
  
}

void FreqResponse_Poll(freq_response_t* fr) {
  if(!fr->samples_remaining) {
    // Emit current value
    if(fr->f >= fr->f_min) {
      if(fr->emit) {
        const freq_response_result_t result = { .freq = fr->f,
                                                .real = fr->sin_sum * fr->normalization_factor,
                                                .imag = fr->cos_sum * fr->normalization_factor,
                                                .exc_real = fr->sin_exc_sum * fr->normalization_factor,
                                                .exc_imag = fr->cos_exc_sum * fr->normalization_factor };
        fr->emit(fr, &result);
      }
    }
    
    // Configure for next value
    fr->f *= powf(10.f, 1.f / fr->frequencies_per_decade);
    if((fr->f < fr->f_min) || (fr->f > fr->f_max))
      fr->f = fr->f_min;
    
    const float samples_per_wavelength = (fr->fs / fr->f);
    fr->theta_increment = (2.f * M_PI) / samples_per_wavelength;
    fr->theta = 0.f;
    
    fr->sin_sum = 0.f;
    fr->cos_sum = 0.f;
    fr->sin_exc_sum = 0.f;
    fr->cos_exc_sum = 0.f;
    
    const int samples_remaining = (int)((fr->fs / fr->f) * fr->wavelengths_per_frequency);
    fr->normalization_factor = 2.f / samples_remaining;
    fr->samples_remaining = samples_remaining;
  }
}

float FreqResponse_Update(freq_response_t* fr, float sample) {
  return FreqResponse_UpdateWithOffset(fr, sample, sample);
}

float FreqResponse_UpdateWithOffset(freq_response_t* fr, float sample, float offset) {
  if(fr->samples_remaining) {
    fr->theta += fr->theta_increment;
    if(fr->theta > (2.f * M_PI))
      fr->theta -= (2.f * M_PI);
    
    const float sin_theta = sinf(fr->theta);
    const float cos_theta = cosf(fr->theta);
    
    fr->sin_sum += sin_theta * sample;
    fr->cos_sum += cos_theta * sample;
    
    const float excitation = offset + sin_theta * fr->magnitude;
    
    fr->sin_exc_sum += sin_theta * excitation;
    fr->cos_exc_sum += cos_theta * excitation;
    
    fr->samples_remaining--;
    
    return excitation;
  } else {
    return sample;
  }
}
