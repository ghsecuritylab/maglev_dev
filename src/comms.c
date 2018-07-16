#include "comms.h"
#include "lwip/apps/mqtt.h"
#include "cwpack/cwpack.h"
#include <string.h>
#include <util.h>

#define HEARTBEAT_TOPIC_BASE "controller/________________________/heartbeat"
#define HEARTBEAT_INTERVAL 1000

static void mqtt_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
  (void)client;
  comms_t* c = (comms_t*)arg;
  
  c->connection_status = status;
  c->connection_response_received = true;
}

static err_t heartbeat(comms_t* c, const char* topic) {
  err_t err = ERR_OK;
  
  systime_t time_now = chVTGetSystemTimeX();
  if(TIME_I2MS(time_now - c->time_last_heartbeat) >= HEARTBEAT_INTERVAL) {
    cw_pack_context pc;
    cw_pack_context_init(&pc, c->cwpack_buffer, sizeof(c->cwpack_buffer), 0);
    
    cw_pack_map_size(&pc, 1);
    cw_pack_str(&pc, "uptime", 6);
    cw_pack_unsigned(&pc, time_now);
    
    int length = pc.current - pc.start;
    
    err = mqtt_publish(&c->client, topic, pc.start, length, 0, 0, NULL, NULL);
    c->time_last_heartbeat = time_now;
  }
  
  return err;
}

static THD_FUNCTION(_CommTask, arg) {
  comms_t* c = (comms_t*)arg;
  
  ip_addr_t broker_addr = IPADDR4_INIT_BYTES(192, 168, 1, 1);
  u16_t broker_port = 1883;
  err_t err;
  
  char topic[] = HEARTBEAT_TOPIC_BASE;
  memcpy(topic + 11, uniqueID(), 24);
  
  while(true) {
    do {
      c->connection_response_received = false;
      chThdSleepMilliseconds(2000);
      
      static struct mqtt_connect_client_info_t client_info = { .client_id = "mqtt_client_ex", .client_user = NULL, .client_pass = NULL, .keep_alive = 0, .will_topic = NULL };
      while((err = mqtt_client_connect(&c->client, &broker_addr, broker_port, &mqtt_cb, c, &client_info)) != ERR_OK)
        chThdSleepMilliseconds(2000);
      
      while(!c->connection_response_received)
        chThdSleepMilliseconds(100);
    } while(c->connection_status != MQTT_CONNECT_ACCEPTED);
    
    systime_t time = chVTGetSystemTime();
    while(c->connection_status == MQTT_CONNECT_ACCEPTED) {
      heartbeat(c, topic);
      
      time += TIME_MS2I(20);
      chThdSleepUntil(time);
    }
  }
}

void CommsInit(comms_t* c) {
  c->connection_status = MQTT_CONNECT_DISCONNECTED;
  
  chThdCreateStatic(c->waTask, sizeof(c->waTask), NORMALPRIO + 1, _CommTask, c);
}
