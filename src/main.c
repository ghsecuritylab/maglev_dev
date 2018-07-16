/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "rt_test_root.h"
#include "oslib_test_root.h"

#include "lwipthread.h"

#include "comms.h"
#include "motor_control.h"
#include "93AA46AE48.h"

#include "util.h"
#include <string.h>

static motor_control_t m1 = { .driver = &PWMD1,
                              .clock_freq = STM32_TIMCLK2,
                              .pwm_freq = 35e3,
                              .nfault = LINE_TIM1_BKIN1,
                              .fault_clear = LINE_GPIOG_PIN1,
                              .bridge_enabled = LINE_GPIOG_PIN0
                             };
static motor_control_t m2 = { .driver = &PWMD8,
                              .clock_freq = STM32_TIMCLK2,
                              .pwm_freq = 35e3,
                              .nfault = LINE_TIM8_BKIN1,
                              .fault_clear = LINE_GPIOF_PIN12,
                              .bridge_enabled = LINE_GPIOF_PIN11
                            };

const float i_scale = (3.f / 4095.f) / 20.f / 0.001f;

static void pwm_cb(PWMDriver *pwmp)  {
  if(pwmp == &PWMD1)
    MotorControlCb(&m1, ((int16_t)ADC1->JDR1) * i_scale,
                        ((int16_t)ADC1->JDR2) * i_scale);
  else if(pwmp == &PWMD8)
    MotorControlCb(&m2, ADC2->JDR1, ADC2->JDR2);
}

static const mc_93aa46ae48_t mc_93aa46ae48 = { .cs = LINE_SPI3_CS,
                                               .sck = LINE_SPI3_SCK,
                                               .miso = LINE_SPI3_MISO,
                                               .mosi = LINE_SPI3_MOSI };
static uint8_t _macaddress[6];
static const struct lwipthread_opts _lwip_opts = { .address = IP4_ADDR_VALUE(192, 168, 1, 10),
                                                   .gateway = IP4_ADDR_VALUE(192, 168, 1, 1),
                                                   .netmask = IP4_ADDR_VALUE(255, 255, 255, 0),
                                                   .addrMode = NET_ADDRESS_STATIC,
                                                   .macaddress = _macaddress };

static void configure_motor_adcs(void) {
  rccEnableADC1(true);
  rccEnableADC2(true);
  
  // Configure the ADC
  ADC1->SMPR2 = (0b010 << 9) | (0b010 << 12);
  ADC1->JSQR = (0b01 << 20) | (3 << 10) | (4 << 15);
  ADC1->CR1 = ADC_CR1_SCAN;
  ADC1->CR2 = ADC_CR2_ADON |
              ADC_CR2_JEXTEN_1;
  ADC1->JOFR1 = ADC1->JOFR2 = 0x7FF;
  
  // Trigger one sample capture, so the first MC interrupt has data
  ADC1->CR2 |= ADC_CR2_JSWSTART;
}

static void handle_incoming_command(comms_t* c,
                                    enum incoming_publish_type inpub_type,
                                    cw_unpack_context* uc)
{
  (void)c;
  motor_control_t* mc = NULL;
  if(inpub_type == INPUB_CMD_M1)
    mc = &m1;
  else if(inpub_type == INPUB_CMD_M2)
    mc = &m2;
  else
    return;

  cw_unpack_next(uc);
  if (uc->item.type != CWP_ITEM_MAP)
    return;
  
  int num_items = uc->item.as.map.size;
  int i;
  for(i = 0; i < num_items; i++) {
    cw_unpack_next(uc);
    if (uc->item.type != CWP_ITEM_STR)
      return;
    
    const char* key = uc->item.as.str.start;
    
    cw_unpack_next(uc);
    if(strncmp(key, "enable", uc->item.as.str.length) == 0) {
      if(uc->item.type == CWP_ITEM_BOOLEAN)
        mc->enabled = uc->item.as.boolean;
    }
    if(strncmp(key, "i_target", uc->item.as.str.length) == 0) {
      if(uc->item.type == CWP_ITEM_FLOAT)
        mc->i_target = uc->item.as.real;
    }
  }
}

static comms_t _comms = { .inpub_cb = handle_incoming_command };

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();
  
  // Read the external memory to get a MAC address before initializing LWIP
  Mc94AA46AE58Init(&mc_93aa46ae48);
  Mc94AA46AE58GetID(&mc_93aa46ae48, _macaddress);
  lwipInit(&_lwip_opts);

  // Set up the motor controllers
  configure_motor_adcs();
  MotorControlInit(&m1, pwm_cb);
  
  // Set up comms
  CommsInit(&_comms);
  
  // Shut down the thread
  chThdExit(0);
  
  return -1;
  // Main thread (in this case, just loop)
  // while (true) {
  //   chThdSleepMilliseconds(500);
  // }
}
