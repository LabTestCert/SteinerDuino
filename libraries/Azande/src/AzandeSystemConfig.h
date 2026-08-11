//-------------------------------------------------------------------
// File:      AzandeSystemConfig.h
// Version:   $Rev$
// Revised:   $Date$
//            $Author: Anders Zeijlon$
//
// Description:
//
// Copyright (c) 2015 Zeijlon Systems AB,
// All rights reserved.
//-------------------------------------------------------------------

#ifndef AZANDESYSTEMCONFIG_H_
#define AZANDESYSTEMCONFIG_H_

#define READ_BUFFER_SIZE 	50
#define AZANDE_MAX_COMMANDS 15
#define AZANDE_MAX_EVENTS 	15

/* MCU Specific
 *
 * Endianess:
 * #define MCU_IS_BIG_ENDIAN or not if Big endian
 *
 * Save messages constant data to program/flash memory:
 * #define __FLASH_DIRECTIVE
 * #define __FLASH_READ_BYTE(VALUE)
 * #define __FLASH_READ_PTR(VALUE)
 */
#if defined(ARDUINO_ARCH_AVR)
	#define MCU_IS_LITTLE_ENDIAN
	#include "avr/pgmspace.h"
	#define __FLASH_DIRECTIVE PROGMEM
	#define __FLASH_READ_BYTE(VALUE) pgm_read_byte_near(&(VALUE))
	#define __FLASH_READ_PTR(VALUE) pgm_read_ptr_near(&(VALUE))
#else // Any other type
	#define MCU_IS_LITTLE_ENDIAN
	#define __FLASH_DIRECTIVE
	#define __FLASH_READ_BYTE(VALUE) VALUE
	#define __FLASH_READ_PTR(VALUE) VALUE
#endif

#endif /* AZANDESYSTEMCONFIG_H_ */
