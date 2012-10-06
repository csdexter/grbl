/*
 * host-i386-private.h
 *
 *  Created on: Oct 6, 2012
 *      Author: csdexter
 */

#ifndef HOST_HAL_PRIVATE_H_
#define HOST_HAL_PRIVATE_H_

#include <stdbool.h>
#include <stdint.h>


// Constants
#define EVENT_TIMER_OVERFLOW 0x01
#define EVENT_TIMER_COMPARE_A 0x02
#define EVENT_TIMER_COMPARE_B 0x03

#define NVS_STORE_NAME "host-nvs.bin"

#define TIMER_MODE_NORMAL 0x01
#define TIMER_MODE_CTC 0x02

// Record types
typedef struct {
  const uint16_t *pDivisors;
  const uint8_t pCount;
  const uint32_t compareMax;
} TTimerPrescalerSpec;

typedef struct {
  char name[7];
  bool enabled;
  void(*vector)(void);
} TInterruptDescriptor;

typedef struct {
  uint16_t count;
  uint16_t channel[2];
  uint16_t prescaler;
  bool wide;
  uint8_t mode;
} TTimerDescriptor;

// Local functions
static int _i386_compare_interrupts(const void *a, const void *b);
static void _i386_do_interrupt_work(void);
static TInterruptDescriptor *_i386_interrupt_by_name(const char *name);
static TInterruptDescriptor *_i386_timer_interrupt_by_type(uint8_t timer,
    uint8_t which);
static void _i386_init_nvs(const char *fileName);
static void _i386_nvs_write(const void *data, size_t size, long int offset);
static void _i386_nvs_read(void *data, size_t size, long int offset);
void _i386_exit_handler(int signum);


#endif /* HOST_HAL_PRIVATE_H_ */
