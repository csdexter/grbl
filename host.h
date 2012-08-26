/*
 * host.h - abstracts away hardware by including the architecture-specific HAL
 *
 *  Created on: Aug 21, 2012
 *      Author: csdexter
 */

#ifndef HOST_H_
#define HOST_H_

#include <stdbool.h>
#include <stddef.h>

#if defined(__GNUC__)
# if defined(__AVR__)
#  include "host/host-avr.h"
# elif defined(__arm__)
#  include "host/host-arm.h"
# elif defined(__i386__)
#  include "host/host-i386.h"
# endif
#endif

/* Host settings Storage/Retrieval interface */
typedef enum {
  HOST_SETTING_OK,
  HOST_SETTING_READONLY,
  HOST_SETTING_NOSPACE,
  HOST_SETTING_NOSIG,
  HOST_SETTING_NOCRC
} THostSettingStatus;

THostSettingStatus host_settings_store(const uint16_t signature,
    const void *settings, const size_t size);
THostSettingStatus host_settings_fetch(const uint16_t signature,
    void *settings, const size_t size);

/* Host serial console baud rate. Low-level functions are in the
 * architecture-specific header file */
#define CONSOLE_BAUD_RATE 9600
/* Host serial console Rx and Tx ring buffer sizes, in bytes */
#define CONSOLE_RXBUF_SIZE 128
#define CONSOLE_TXBUF_SIZE 16
/* Host serial console will readback this value when there's nothing to read */
#define CONSOLE_NO_DATA 0xFF
/* Host serial console variable string output. Blocks until buffer space is
 * available if block is set to true, returns false if in non-blocking mode and
 * no buffer space */
bool host_serialconsole_printstring(const char *s, bool block);
/* Host serial console constant string output. Blocks until buffer space is
 * available if block is set to true, returns false if in non-blocking mode and
 * no buffer space */
bool host_serialconsole_printmessage(const char *s, bool block);


#endif /* HOST_H_ */
