#pragma once

#include "ch.h"
#include "lwip/apps/mqtt.h"
#include "cwpack/cwpack.h"

#define TOPIC_BASE "controller/________________________/"
#define M1_BASE TOPIC_BASE "m1/"
#define M1_CMD M1_BASE "cmd"
#define M1_TELEM M1_BASE "telem"

typedef struct comms_s comms_t;

enum incoming_publish_type {
  INPUB_NONE = 0,
  INPUB_CMD_M1 = 1,
  INPUB_CMD_M2 = 2
};

typedef void (*inpub_handler)(comms_t*, enum incoming_publish_type, cw_unpack_context*);
typedef systime_t (*outpub_handler)(comms_t*, cw_pack_context*);

struct comms_s {
  inpub_handler inpub_cb;
  outpub_handler telem_cb;
  
  mqtt_connection_status_t connection_status;
  bool connection_response_received;
  
  mqtt_client_t client;
  
  char m1_cmd_topic[sizeof(M1_CMD)];
  char m1_telem_topic[sizeof(M1_TELEM)];
  
  char pub_buffer[1024];
  char sub_buffer[1024];
  char* sub_ptr;
  
  systime_t time_last_heartbeat;
  
  enum incoming_publish_type incoming_publish;
  
  THD_WORKING_AREA(waTask, 2048);
};

void CommsInit(comms_t*);
