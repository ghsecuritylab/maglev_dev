#pragma once

#include "ch.h"
#include "hal.h"


#ifdef __cplusplus
extern "C" {
#endif
  
  typedef struct {
    ioline_t cs;
    ioline_t sck;
    ioline_t miso;
    ioline_t mosi;
  } mc_93aa46ae48_t;
  
  void Mc94AA46AE58Init(const mc_93aa46ae48_t*);
  void Mc94AA46AE58GetID(const mc_93aa46ae48_t*, uint8_t mac[6]);
  
#ifdef __cplusplus
}
#endif
