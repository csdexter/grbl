/*
 * host.h - abstracts away hardware by including the architecture-specific HAL
 *
 *  Created on: Aug 21, 2012
 *      Author: csdexter
 */

#ifndef HOST_H_
#define HOST_H_

#if defined(__GNUC__)
# if defined(__AVR__)
#  include "host/host-avr.h"
# elif defined(__arm__)
#  include "host/host-arm.h"
# elif defined(__i386__)
#  include "host/host-i386.h"
# endif
#endif

#endif /* HOST_H_ */
