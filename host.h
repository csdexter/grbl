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

/* Function generator interface.
 * Some architectures may have an actual function generator, others may
 * implement it using CTC (Counter-Timer-Comparator) circuits in waveform
 * generation mode and yet others may implement it entirely in software.
 * NOTE: the particular case of a square wave with variable duty cycle
 *       (i.e. PWM) is not offered by this API
 * NOTE: on some architectures, generating the waveform may be distinct from
 *       output circuitry, which will have to be configured separately */
typedef enum {
  HOST_FG_SINE,
  HOST_FG_SQUARE,
  HOST_FG_SAWTOOTH,
  HOST_FG_TRIANGLE,
} THostFunctionGeneratorWaveform;

/* Host GPIO interface */
#define HOST_GPIO_MODE_BYTE true
#define HOST_GPIO_MODE_BIT false
/* Outputs are identified by #defines starting with HOST_GPIO_ followed by the
 * short port name as given in the datasheet (e.g. PD for Port D), followed by
 * the bit number in that port, as given in the datasheet */
#define host_bitvalue(bit) (1 << (bit))
/*#define host_bit_of_output(output) (7 - (output) % 8)*/
/* If mode, then value is a pre-built bitmask and the whole destination will be
 * set in one go; else only the given bit will be set */
#define host_read_modify_write(destination,argument,value,mode) (mode ? \
    (destination(argument) = value) : \
    (value ? \
        (destination(argument) |= host_bitvalue(host_bit_of_output(argument))) : \
        (destination(argument) &= ~host_bitvalue(host_bit_of_output(argument)))))


#endif /* HOST_H_ */
