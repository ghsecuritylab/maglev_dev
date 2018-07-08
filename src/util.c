#include <ch.h>
#include "util.h"

#define STM32_UUID ((const uint32_t *)0x1FFF7A10)
#define STM32_UUID_LEN 12

const char* uniqueID() {
  static const char* uid = NULL;
  static char buffer[STM32_UUID_LEN * 2 + 1] = {0};
  
  if(!uid) {
    bytesToHex(buffer, (const uint8_t*)STM32_UUID, STM32_UUID_LEN);
    uid = buffer;
  }
  
  return uid;
}

char nibbleToHex(const uint8_t nibble) {
  const char* hex = "0123456789ABCDEF";
  return hex[nibble & 0xF];
}

void bytesToHex(char* buffer, const uint8_t* bytes, size_t numBytes) {
  size_t i;
  for(i = 0; i < numBytes; i++) {
    buffer[2*i] = nibbleToHex(bytes[i] >> 4);
    buffer[2*i + 1] = nibbleToHex(bytes[i]);
  }
}
