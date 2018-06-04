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

#include "lwip/apps/mqtt.h"

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palSetLine(LINE_LED1);
    chThdSleepMilliseconds(50);
    palSetLine(LINE_LED2);
    chThdSleepMilliseconds(50);
    palSetLine(LINE_LED3);
    chThdSleepMilliseconds(200);
    palClearLine(LINE_LED1);
    chThdSleepMilliseconds(50);
    palClearLine(LINE_LED2);
    chThdSleepMilliseconds(50);
    palClearLine(LINE_LED3);
    chThdSleepMilliseconds(200);
  }
}


volatile mqtt_connection_status_t mqtt_status = MQTT_CONNECT_DISCONNECTED;
void mqtt_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
  mqtt_status = status;
}

static THD_WORKING_AREA(waMQTT, 2048);
static THD_FUNCTION(MQTT, arg) {
  
  mqtt_client_t client;
  ip_addr_t broker_addr = IPADDR4_INIT_BYTES(192, 168, 1, 1);
  u16_t broker_port = 1883;
  
  struct mqtt_connect_client_info_t client_info = { .client_id = "mqtt_client_ex", .client_user = NULL, .client_pass = NULL, .keep_alive = 0, .will_topic = NULL };
  
  chThdSleepMilliseconds(2000);
  err_t err;
  
  while((err = mqtt_client_connect(&client, &broker_addr, broker_port, &mqtt_cb, NULL, &client_info)) != ERR_OK)
    chThdSleepMilliseconds(2000);
  
  while(mqtt_status != MQTT_CONNECT_ACCEPTED)
    chThdSleepMilliseconds(2000);
  
  while(true) {
    err = mqtt_publish(&client, "/test", "hello 123", 10, 0, 0, NULL, NULL);
    chThdSleepMilliseconds(2000);
  }
}

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
  lwipInit(NULL);

  /*
   * Activates the serial driver 3 using the driver default configuration.
   */
  sdStart(&SD3, NULL);

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
    if (palReadLine(LINE_BUTTON)) {
      test_execute((BaseSequentialStream *)&SD3, &rt_test_suite);
      test_execute((BaseSequentialStream *)&SD3, &oslib_test_suite);
    }
    chThdSleepMilliseconds(500);
  }
}
