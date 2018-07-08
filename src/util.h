#pragma once

#include <stdint.h>

const char* uniqueID(void);
char nibbleToHex(const uint8_t nibble);
void bytesToHex(char* buffer, const uint8_t* bytes, size_t numBytes);
