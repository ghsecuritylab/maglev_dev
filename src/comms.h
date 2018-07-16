#pragma once

#include "ch.h"
#include "lwip/apps/mqtt.h"

typedef struct {
  mqtt_connection_status_t connection_status;
  bool connection_response_received;
  
  mqtt_client_t client;
  
  char cwpack_buffer[1024];
  systime_t time_last_heartbeat;
  
  THD_WORKING_AREA(waTask, 2048);
} comms_t;

void CommsInit(comms_t*);
