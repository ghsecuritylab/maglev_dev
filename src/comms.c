#include "comms.h"
#include "lwip/apps/mqtt.h"
#include <string.h>
#include <util.h>

#define HEARTBEAT_TOPIC_BASE TOPIC_BASE "heartbeat"
#define HEARTBEAT_INTERVAL 1000

static void mqtt_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
  (void)client;
  comms_t* c = (comms_t*)arg;
  
  c->connection_status = status;
  c->connection_response_received = true;
}

static void process_incoming_publish(comms_t* c) {
  uint32_t len = c->sub_ptr - c->sub_buffer;
  
  cw_unpack_context uc;
  cw_unpack_context_init(&uc, c->sub_buffer, len, 0);
  
  if(c->inpub_cb) {
    c->inpub_cb(c, c->incoming_publish, &uc);
  }
}

static void mqtt_pub_cb(void* arg, const char* topic, uint32_t tot_len) {
  comms_t* c = (comms_t*)arg;
  
  c->incoming_publish = INPUB_NONE;
  c->sub_ptr = c->sub_buffer;
  
  if(tot_len > sizeof(c->sub_buffer)) {
    return; // Do not attempt to handle messages that are too long
  }
  
  if(strncmp(topic, c->m1_cmd_topic, sizeof(c->m1_cmd_topic)) == 0) {
    c->incoming_publish = INPUB_CMD_M1;
  }
}

static void mqtt_data_cb(void* arg, const uint8_t* data, uint16_t len, uint8_t flags) {
  comms_t* c = (comms_t*)arg;
  
  if(c->incoming_publish == INPUB_NONE)
    return; // Not in the correct state
  if((c->sub_ptr - c->sub_buffer) + len > sizeof(c->sub_buffer))
    return; // Bail if length is too long
  
  memcpy(c->sub_ptr, data, len);
  c->sub_ptr += len;
  
  if(flags & MQTT_DATA_FLAG_LAST) {
    process_incoming_publish(c);
    c->incoming_publish = INPUB_NONE;
  }
}

static err_t heartbeat(comms_t* c, const char* topic) {
  err_t err = ERR_OK;
  
  systime_t time_now = chVTGetSystemTimeX();
  if(TIME_I2MS(time_now - c->time_last_heartbeat) >= HEARTBEAT_INTERVAL) {
    cw_pack_context pc;
    cw_pack_context_init(&pc, c->pub_buffer, sizeof(c->pub_buffer), 0);
    
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
  
  char heartbeat_topic[] = HEARTBEAT_TOPIC_BASE;
  memcpy(heartbeat_topic + 11, uniqueID(), 24);
  
  memcpy(c->m1_cmd_topic, M1_CMD, sizeof(M1_CMD));
  memcpy(c->m1_cmd_topic + 11, uniqueID(), 24);
  
  memcpy(c->m1_telem_topic, M1_TELEM, sizeof(M1_TELEM));
  memcpy(c->m1_telem_topic + 11, uniqueID(), 24);
  
  
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
    
    mqtt_set_inpub_callback(&c->client, mqtt_pub_cb, mqtt_data_cb, c);
    mqtt_subscribe(&c->client, c->m1_cmd_topic, 0, 0, c);
    
    systime_t next_outpub = chVTGetSystemTime();
    while(c->connection_status == MQTT_CONNECT_ACCEPTED) {
      systime_t time_now = chVTGetSystemTime();
      
      if((c->telem_cb) && (time_now > next_outpub)) {
        cw_pack_context pc;
        cw_pack_context_init(&pc, c->pub_buffer, sizeof(c->pub_buffer), 0);
        
        const systime_t delay = c->telem_cb(c, &pc);
        
        const int length = pc.current - pc.start;
        err = mqtt_publish(&c->client, c->m1_telem_topic, pc.start, length, 0, 0, NULL, NULL);
        
        next_outpub = time_now + delay;
      }
      
      heartbeat(c, heartbeat_topic);
      
      chThdSleep(TIME_MS2I(1));
    }
  }
}

void CommsInit(comms_t* c) {
  c->connection_status = MQTT_CONNECT_DISCONNECTED;
  
  chThdCreateStatic(c->waTask, sizeof(c->waTask), NORMALPRIO + 1, _CommTask, c);
}
