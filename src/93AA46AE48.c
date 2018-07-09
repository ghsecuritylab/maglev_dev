#include "93AA46AE48.h"
#include "ch.h"
#include "hal.h"

static uint8_t transfer_bit(const mc_93aa46ae48_t* mc, uint8_t bit) {
  
  palWriteLine(mc->mosi, bit ? PAL_HIGH : PAL_LOW);
  chThdSleepMicroseconds(1);
  palWriteLine(mc->sck, PAL_HIGH);
  chThdSleepMicroseconds(1);
  palWriteLine(mc->sck, PAL_LOW);
  
  return palReadLine(mc->miso);
}

static void write_address(const mc_93aa46ae48_t* mc, uint8_t opcode, uint8_t address) {
  // SB
  transfer_bit(mc, 1);
  
  // Opcode
  transfer_bit(mc, (opcode & 0b10) ? 1 : 0);
  transfer_bit(mc, (opcode & 0b01) ? 1 : 0);
  
  // Address
  int i;
  for(i = 0; i < 7; i++) {
    transfer_bit(mc, (address >> (6 - i)) & 0x01);
  }
}

static uint8_t read_byte(const mc_93aa46ae48_t* mc) {
  uint8_t b = 0;
  
  int i;
  for(i = 0; i < 8; i++) {
    b <<= 1;
    b |= transfer_bit(mc, 0);
  }
  
  return b;
}


void Mc94AA46AE58Init(const mc_93aa46ae48_t* mc) {
  palWriteLine(mc->cs, PAL_LOW);
}

void Mc94AA46AE58GetID(const mc_93aa46ae48_t* mc, uint8_t mac[6]) {
  
  palWriteLine(mc->cs, PAL_HIGH);
  write_address(mc, 0b10, 0x00);
  
  uint8_t epi = read_byte(mc);
  osalDbgAssert((epi == 0xa5), "Mc94AA46AE58GetID(), EEPROM missing or not programmed with MAC");
  
  int i = 0;
  for(i = 0; i < 6; i++) {
    mac[i] = read_byte(mc);
  }
  
  palWriteLine(mc->cs, PAL_LOW);
  chThdSleepMicroseconds(1);
  
}
