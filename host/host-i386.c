/*
 * host-i386.c - Hosting-specific HAL routines
 *
 *  Created on: Sep 26, 2012
 *      Author: csdexter
 *
 * NOTE: this file assumes a standard UNIX system and strives to only use
 *       standards-compliant functions.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "host.h"

static const char *gpioNames[] = {
  "NONE/ERROR",
  "Y_STEP",
  "X_STEP",
  "Z_STEP",
  "X_DIR",
  "Y_DIR",
  "Z_DIR",
  "X_LIMIT",
  "Y_LIMIT",
  "Z_LIMIT",
  "SERVO_OFF",
  "SPINDLE_ON",
  "SPINDLE_CCW",
  "CHARGE_PUMP",
  "COOL_FLOOD",
  "COOL_MIST"
};

static const char *timerInterruptNames[] = {
  "NONE/ERROR",
  "OVERFLOW",
  "COMPARE_A",
  "COMPARE_B"
};

static const uint16_t timer0Prescalers[] =  HOST_TIMER_PRESCALERS_0;
static const uint16_t timer1Prescalers[] =  HOST_TIMER_PRESCALERS_1;
static const uint16_t timer2Prescalers[] =  HOST_TIMER_PRESCALERS_2;

typedef struct {
  const uint16_t *pDivisors;
  const uint8_t pCount;
  const uint32_t compareMax;
} TTimerPrescalerSpec;

static const TTimerPrescalerSpec timerProperties[] = {
  {timer0Prescalers, HOST_TIMER_PRESCALER_COUNT_0, HOST_TIMER_COMPARE_MAX_0},
  {timer1Prescalers, HOST_TIMER_PRESCALER_COUNT_1, HOST_TIMER_COMPARE_MAX_1},
  {timer2Prescalers, HOST_TIMER_PRESCALER_COUNT_2, HOST_TIMER_COMPARE_MAX_2},
};

static bool interruptsEnabled = false;

typedef struct {
  char name[7];
  uint8_t peripheral, instance, event;
  bool enabled;
  void(*vector)(void);
} TInterruptDescriptor;

#define PERIPHERAL_TIMER 0x01
#define EVENT_TIMER_OVERFLOW 0x01
#define EVENT_TIMER_COMPARE_A 0x02
#define EVENT_TIMER_COMPARE_B 0x03

static TInterruptDescriptor interrupts[] = {
  {"T0_A_V", PERIPHERAL_TIMER, 0, EVENT_TIMER_COMPARE_A, false, NULL},
  {"T0_B_V", PERIPHERAL_TIMER, 0, EVENT_TIMER_COMPARE_B, false, NULL},
  {"T0_O_V", PERIPHERAL_TIMER, 0, EVENT_TIMER_OVERFLOW, false, NULL},
  {"T1_A_V", PERIPHERAL_TIMER, 1, EVENT_TIMER_COMPARE_A, false, NULL},
  {"T1_B_V", PERIPHERAL_TIMER, 1, EVENT_TIMER_COMPARE_B, false, NULL},
  {"T1_O_V", PERIPHERAL_TIMER, 1, EVENT_TIMER_OVERFLOW, false, NULL},
  {"T2_A_V", PERIPHERAL_TIMER, 2, EVENT_TIMER_COMPARE_A, false, NULL},
  {"T2_B_V", PERIPHERAL_TIMER, 2, EVENT_TIMER_COMPARE_B, false, NULL},
  {"T2_O_V", PERIPHERAL_TIMER, 2, EVENT_TIMER_OVERFLOW, false, NULL}
};

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
    if(crc & 1) crc = (crc >> 1) ^ 0x8CU;
    else crc >>= 1;

  return crc;
}

uint16_t host_crc16(uint16_t crc, uint8_t data) {
  uint8_t i;

  crc ^= data;
  for (i = 0; i < 8; i++)
    if(crc & 1) crc = (crc >> 1) ^ 0xA001U;
    else crc >>= 1;

  return crc;
}

void host_init(int argc, char **argv) {
  printf("Hosting HAL for grbl up and running\n");
}

void host_sei(void) {
  printf("INTR: Interrupts are now globally enabled\n");
  interruptsEnabled = true;
}

static int _i386_compare_interrupts(const void *a, const void *b) {
  return strcmp(((const TInterruptDescriptor *)a)->name, ((const TInterruptDescriptor *)b)->name);
}

static void _i386_do_interrupt_work(void) {
  //TODO: Stub, add flesh ;-)
}

void i386_register_interrupt(const char *name, void(*isr)(void)) {
  TInterruptDescriptor tmpint, *tmpintptr = NULL;

  memset(&tmpint, 0x00, sizeof(tmpint));
  strcpy(tmpint.name, name);
  tmpintptr = bsearch(&tmpint, interrupts,
      sizeof(interrupts) / sizeof(interrupts[0]), sizeof(TInterruptDescriptor),
      _i386_compare_interrupts);
  if(tmpintptr) {
    tmpintptr->vector = isr;
    printf("INTR: Associated code at %p with vector %s\n", isr, name);
  }
}

void host_gpio_write(uint8_t output, uint8_t value, bool mode) {
  if(mode == HOST_GPIO_MODE_BIT)
    printf("GPIO: Output %s set %s\n", gpioNames[output],
        (value ? "HIGH" : "LOW"));
}

void host_gpio_direction(uint8_t output, bool direction, bool mode) {
  if(mode == HOST_GPIO_MODE_BIT)
    printf("GPIO: %s configured as %s\n", gpioNames[output],
        ((direction == HOST_GPIO_DIRECTION_INPUT) ? "INPUT" : "OUTPUT"));
}

uint8_t host_gpio_read(uint8_t output, bool mode) {
  printf("GPIO: Reads not implemented yet, returning HIGH\n");

  return ((mode == HOST_GPIO_MODE_BIT) ? true : 0xFF);
}

void host_nvs_store_byte(uint8_t *address, uint8_t value) {
  printf("NVST: Storing %"PRIX8"@0x%04"PRIX16"\n", value, (uint16_t)(ptrdiff_t)address);
}

void host_nvs_store_word(uint16_t *address, uint16_t value) {
  printf("NVST: Storing %"PRIX16"@0x%04"PRIX16"\n", value, (uint16_t)(ptrdiff_t)address);
}

void host_nvs_store_data(uint8_t *address, const void *value, size_t size){
  printf("NVST: Storing %zu bytes of data at 0x%04"PRIX16"\n", size, (uint16_t)(ptrdiff_t)address);
}

uint8_t host_nvs_fetch_byte(uint8_t *address) {
  printf("NVST: Reads not implemented yet, returning 0x00\n");

  return 0x00;
}

uint16_t host_nvs_fetch_word(uint16_t *address) {
  printf("NVST: Reads not implemented yet, returning 0x0000\n");

  return 0x0000;
}

void host_nvs_fetch_data(uint8_t *address, void *buffer, size_t size) {
  printf("NVST: Reads not implemented yet, returning all zeros\n");

  memset(buffer, 0x00, size);
}

void host_serialconsole_init() {
  printf("SCON: Serial console up and running\n");
}

void host_serialconsole_reset() {
  printf("SCON: Serial console Rx buffer scrapped\n");
}

char host_serialconsole_read(void) {
  //This is an input-driven program, it therefore makes sense to trap the
  //look-alike of the read() call and schedule interrupt work there
  if(interruptsEnabled) _i386_do_interrupt_work();

  int c = fgetc(stdin);

  return (c == EOF ? CONSOLE_NO_DATA : c);
}

bool host_serialconsole_write(char c, bool block) {
  fputc(c, stdout);

  return true;
}

bool host_serialconsole_printinteger(uint32_t n, bool block) {
  fprintf(stdout, "%"PRIu32, n);

  return true;
}

/* Exact copy of the AVR implementation, without blocking awareness */
//TODO: investigate whether this belongs in host.c since it's generic
bool host_serialconsole_printbinary(uint8_t n, bool block) {
  uint8_t i;

  for(i = 8; i; i--) {
    host_serialconsole_write('0' + (bool)(n & 0x80), block);
    n <<= 1;
  }

  return true;
}

bool host_serialconsole_printfloat(float n, uint8_t precision, bool block) {
  char buf[5];

  snprintf(buf, sizeof(buf), "%%.%"PRIu8"f", precision);
  fprintf(stdout, buf, n);

  return true;
}

void i386_timer_enable_interrupt(uint8_t timer, uint8_t which) {
  TInterruptDescriptor tmpint, *tmpintptr;

  memset(&tmpint, 0x00, sizeof(tmpint));
  tmpint.name[0] = 'T';
  tmpint.name[1] = '0' + timer;
  tmpint.name[2] = '_';
  switch(which) {
    case EVENT_TIMER_OVERFLOW: tmpint.name[3] = 'O'; break;
    case EVENT_TIMER_COMPARE_A: tmpint.name[3] = 'A'; break;
    case EVENT_TIMER_COMPARE_B: tmpint.name[3] = 'B'; break;
  }
  tmpint.name[4] = '_';
  tmpint.name[5] = 'V'; // \0 already there from memset() above
  tmpintptr = bsearch(&tmpint, interrupts,
      sizeof(interrupts) / sizeof(interrupts[0]), sizeof(TInterruptDescriptor),
      _i386_compare_interrupts);
  if(tmpintptr) {
    tmpintptr->enabled = true;
    printf("INTR: Enabled interrupt for timer %"PRIu8" event %s\n", timer, timerInterruptNames[which]);
  }
}

void i386_timer_disable_interrupt(uint8_t timer, uint8_t which) {
  TInterruptDescriptor tmpint, *tmpintptr;

  memset(&tmpint, 0x00, sizeof(tmpint));
  tmpint.name[0] = 'T';
  tmpint.name[1] = '0' + timer;
  tmpint.name[2] = '_';
  switch(which) {
    case EVENT_TIMER_OVERFLOW: tmpint.name[3] = 'O'; break;
    case EVENT_TIMER_COMPARE_A: tmpint.name[3] = 'A'; break;
    case EVENT_TIMER_COMPARE_B: tmpint.name[3] = 'B'; break;
  }
  tmpint.name[4] = '_';
  tmpint.name[5] = 'V'; // \0 already there from memset() above
  tmpintptr = bsearch(&tmpint, interrupts,
      sizeof(interrupts) / sizeof(interrupts[0]), sizeof(TInterruptDescriptor),
      _i386_compare_interrupts);
  if(tmpintptr) {
    tmpintptr->enabled = false;
    printf("INTR: Disabled interrupt for timer %"PRIu8" event %s\n", timer, timerInterruptNames[which]);
  }
}

void host_timer_set_compare(uint8_t timer, uint8_t channel, uint32_t value) {
  printf("CTTM: Set counter/timer %"PRIu8" %s value to %"PRIu32"\n", timer, timerInterruptNames[channel], value);
}

void host_timer_set_count(uint8_t timer, uint32_t count) {
  printf("CTTM: Set counter/timer %"PRIu8" count value to %"PRIu32"\n", timer, count);
}

void host_timer_set_prescaler(uint8_t timer, uint8_t prescaler) {
  if(prescaler)
    printf("CTTM: Set counter/timer %"PRIu8" prescaler/divisor to %"PRIu8"\n", timer, prescaler);
  else printf("CTTM: Set counter/timer %"PRIu8" to no prescaler (i.e. stopped)\n", timer);
}

void host_timer_enable_ctc(uint8_t timer) {
  printf("CTTM: Set counter/timer %"PRIu8" operation mode to CTC\n", timer);
}

uint32_t i386_timer_set_reload(uint8_t timer, uint32_t cycles) {
  uint8_t i;
  uint16_t ceiling = 0;

  for(i = 0; i < timerProperties[timer].pCount; i++) {
    if(cycles < timerProperties[timer].compareMax * timerProperties[timer].pDivisors[i]) {
      ceiling = cycles / timerProperties[timer].pDivisors[i];
      host_timer_set_compare(timer, HOST_TIMER_CHANNEL_A, ceiling);
      host_timer_enable_ctc(timer);
      host_timer_set_prescaler(timer, timerProperties[timer].pDivisors[i]);
      return ceiling * timerProperties[timer].pDivisors[i];
    }
  }

  host_timer_set_compare(timer, HOST_TIMER_CHANNEL_A, timerProperties[timer].compareMax - 1);
  host_timer_enable_ctc(timer);
  host_timer_set_prescaler(timer, timerProperties[timer].pDivisors[i]);
  return (timerProperties[timer].compareMax - 1) * timerProperties[timer].pDivisors[i];
}

void host_functiongenerator_start(uint8_t output, uint32_t frequency, uint8_t form) {
  if(form == HOST_FG_SQUARE)
    printf("GPIO: Output %s configured for WGM, square-wave, 50%%, %"PRIu32"Hz\n", gpioNames[output], frequency);
}

void host_functiongenerator_stop(uint8_t output) {
  printf("GPIO: Output %s WGM configuration disabled\n", gpioNames[output]);
}
