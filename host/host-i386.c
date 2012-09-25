/*
 * host-i386.c - Hosting-specific HAL routines
 *
 *  Created on: Sep 26, 2012
 *      Author: csdexter
 *
 * NOTE: this file assumes a standard UNIX system and strives to only use
 *       standards-compliant functions.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <time.h>

#include "host.h"

void i386_delay_us(uint32_t us) {
  struct timespec before, after;

  before.tv_sec = us / 1000000;
  before.tv_nsec = (us % 1000000) * 1000UL;
  while(nanosleep(&before, &after)) before = after;
}

/* Exact implementations of the C code from AVR's util/crc16.h */
uint8_t host_crc8(uint8_t crc, uint8_t data) {
  uint8_t i;

  crc ^= data;
  for (i = 0; i < 8; i++)
    if(crc & 1) crc = (crc >> 1) ^ 0x8C;
    else crc >>= 1;

  return crc;
}

uint16_t host_crc16(uint16_t crc, uint8_t data) {
  uint8_t i;

  crc ^= data;
  for (i = 0; i < 8; i++)
    if(crc & 1) crc = (crc >> 1) ^ 0xA001;
    else crc >>= 1;

  return crc;
}

