/* -----------------------------------------------------------------------------
 * Project:			GPDSE AVR8 Library
 * File:			keypad.h
 * Module:			Matrix keypad controller
 * Author:			Leandro Schwarz
 * Version:			1.2
 * Last edition:	2016-05-18
 * Purpose:			Matrix keypad controller with support to 4x3, 4x4 and 5x3
 *					keypads and configurable debounce time
 * -------------------------------------------------------------------------- */

#ifndef __KEYPAD_H
#define __KEYPAD_H 12

// -----------------------------------------------------------------------------
// Header files ----------------------------------------------------------------

#include "globalDefines.h"
#if __GLOBALDEFINES_H < 123
	#error Error 100 - The defintion file is outdated (globalDefines must be version 12.3 or greater).
#endif
#include <stdarg.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------
// New data types --------------------------------------------------------------

typedef enum keypadType_t{
	KEYPAD_TYPE_UNDEFINED = 0,
	KEYPAD_4X4 = 1,
	KEYPAD_4X3,
	KEYPAD_5X3
} keypadType_t;

typedef enum keypadResult_t{
	KEYPAD_OK = 1,
	KEYPAD_TYPE_NOT_ALLOWED,
	KEYPAD_MEMORY_ALLOCATION_ERROR,
	KEYPAD_PORTS_NOT_CONFIGURED,
	KEYPAD_KEYS_NOT_CONFIGURED,
	KEYPAD_NOT_INITIALIZED
} keypadResult_t;

typedef struct keypadConfiguration_t{
	volatile uint8 * linesDDR;
	volatile uint8 * linesPORT;
	volatile uint8 * linesPIN;
	volatile uint8 * columnsDDR;
	volatile uint8 * columnsPORT;
	uint8 * keys;
	uint8 linesLSB:			3;
	uint8 columnsLSB:		3;
	keypadType_t type:		2;
	uint8 lines:			3;
	uint8 columns:			3;
	uint8 portConfigured:	1;
	uint8 keysConfigured:	1;
	uint8 initialized:		1;
	uint8 debounceTime:		7;
} keypadConfiguration_t;

// -----------------------------------------------------------------------------
// Macrofunctions --------------------------------------------------------------

#define attachKeypad(object) volatile keypadConfiguration_t object = {.linesDDR = NULL, .linesPORT = NULL, .linesPIN = NULL, .columnsDDR = NULL, .columnsPORT = NULL, .keys = NULL, .linesLSB = 0, .columnsLSB = 0, .type = KEYPAD_TYPE_UNDEFINED, .lines = 0, .columns = 0, .portConfigured = FALSE, .keysConfigured = FALSE, .initialized = FALSE, .debounceTime = 1}

// -----------------------------------------------------------------------------
// Functions declarations ------------------------------------------------------

keypadResult_t keypadResetConfiguration(volatile keypadConfiguration_t * keypad);
keypadResult_t keypadSetPort(volatile keypadConfiguration_t * keypad, volatile uint8 * linesDDR, volatile uint8 * linesPORT, volatile uint8 * linesPIN, uint8 linesLSB, volatile uint8 * columnsDDR, volatile uint8 * columnsPORT, uint8 columnsLSB);
keypadResult_t keypadSetKeys(volatile keypadConfiguration_t * keypad, keypadType_t type, ...);
keypadResult_t keypadInit(volatile keypadConfiguration_t * keypad, uint8 debounceTime);
uint8 keypadRead(volatile keypadConfiguration_t * keypad);

#endif