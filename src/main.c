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

#include "web/web.h"
#include "motor_control.h"
#include "93AA46AE48.h"

#include "lwip/apps/mqtt.h"
#include "util.h"
#include <string.h>

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    chThdSleepMilliseconds(200);
  }
}


volatile mqtt_connection_status_t mqtt_status = MQTT_CONNECT_DISCONNECTED;
volatile bool mqtt_response_received = false;
void mqtt_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
  mqtt_status = status;
  mqtt_response_received = true;
}

static THD_WORKING_AREA(waMQTT, 2048);
static THD_FUNCTION(MQTT, arg) {
  
  mqtt_client_t client;
  ip_addr_t broker_addr = IPADDR4_INIT_BYTES(192, 168, 1, 1);
  u16_t broker_port = 1883;
  err_t err;
  
  do {
    mqtt_response_received = false;
    chThdSleepMilliseconds(2000);
    
    static struct mqtt_connect_client_info_t client_info = { .client_id = "mqtt_client_ex", .client_user = NULL, .client_pass = NULL, .keep_alive = 0, .will_topic = NULL };
    while((err = mqtt_client_connect(&client, &broker_addr, broker_port, &mqtt_cb, NULL, &client_info)) != ERR_OK)
      chThdSleepMilliseconds(2000);
    
    while(!mqtt_response_received)
      chThdSleepMilliseconds(100);
  } while(mqtt_status != MQTT_CONNECT_ACCEPTED);
  
  while(true) {
    err = mqtt_publish(&client, "/test", uniqueID(), strlen(uniqueID()), 0, 0, NULL, NULL);
    chThdSleepMilliseconds(2000);
  }
}

static volatile motor_control_t m1 = { .driver = &PWMD1,
                                       .clock_freq = STM32_TIMCLK2,
                                       .pwm_freq = 35e3,
                                       .nfault = LINE_TIM1_BKIN1,
                                       .fault_clear = LINE_GPIOG_PIN1,
                                       .bridge_enabled = LINE_GPIOG_PIN0
                                      };
static volatile motor_control_t m2 = { .driver = &PWMD8,
                                       .clock_freq = STM32_TIMCLK2,
                                       .pwm_freq = 35e3,
                                       .nfault = LINE_TIM8_BKIN1,
                                       .fault_clear = LINE_GPIOF_PIN12,
                                       .bridge_enabled = LINE_GPIOF_PIN11
                                     };

static void pwm_cb(PWMDriver *pwmp)  {
  if(pwmp == &PWMD1)
    MotorControlCb(&m1);
  else if(pwmp == &PWMD8)
    MotorControlCb(&m2);
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

  /*
   * Set up the motor controllers
   */
  MotorControlInit(&m1, pwm_cb);
  
  
  /*
   * Activates the serial driver 3 using the driver default configuration.
   */
  // sdStart(&SD3, NULL);
  

  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO + 1, Thread1, NULL);
  
  /*
   * Creates the HTTP thread (it changes priority internally).
   */
  chThdCreateStatic(wa_http_server, sizeof(wa_http_server), NORMALPRIO + 1,
                    http_server, NULL);
  
  /*
   * Creates the MQTT publish thread.
   */
  chThdCreateStatic(waMQTT, sizeof(waMQTT), NORMALPRIO + 1,
                    MQTT, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true) {
    chThdSleepMilliseconds(500);
  }
}
