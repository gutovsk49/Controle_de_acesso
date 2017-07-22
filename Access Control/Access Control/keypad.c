/* -----------------------------------------------------------------------------
 * Project:			GPDSE AVR8 Library
 * File:			keypad.c
 * Module:			Matrix keypad controller
 * Author:			Leandro Schwarz
 * Version:			1.2
 * Last edition:	2016-05-18
 * Purpose:			Matrix keypad controller with support to 4x3, 4x4 and 5x3
 *					keypads and configurable debounce time
 * -------------------------------------------------------------------------- */

// -----------------------------------------------------------------------------
// Header files ----------------------------------------------------------------

#include "keypad.h"
#if __KEYPAD_H != 12
	#error Error 101 - Version mismatch on header and source code files (keypad).
#endif

/* -----------------------------------------------------------------------------
 * Function:	keypadResetConfiguration
 * Purpose:		Resets the keypad configuration struct
 * Arguments:	keypad		Pointer to the keypad struct
 * Returns:		KEYPAD_OK
 * -------------------------------------------------------------------------- */

keypadResult_t keypadResetConfiguration(volatile keypadConfiguration_t * keypad)
{
	keypad->linesDDR = NULL;
	keypad->linesPORT = NULL;
	keypad->linesPIN = NULL;
	keypad->columnsDDR = NULL;
	keypad->columnsPORT = NULL;
	free(keypad->keys);
	keypad->keys = NULL;
	keypad->linesLSB = 0;
	keypad->columnsLSB = 0;
	keypad->type = KEYPAD_TYPE_UNDEFINED;
	keypad->lines = 0;
	keypad->columns = 0;
	keypad->portConfigured = FALSE;
	keypad->keysConfigured = FALSE;
	keypad->initialized = FALSE;
	keypad->debounceTime = 0;
	
	return KEYPAD_OK;
}

/* -----------------------------------------------------------------------------
 * Function:	keypadSetPort
 * Purpose:		Configures the keypad I/O ports
 * Arguments:	keypad			Pointer to the keypad struct
 *				linesDDR		Pointer to I/O Data Direction Register
 *				linesPORT		Pointer to I/O Port Output Register
 *				linesPIN		Pointer to I/O Port Input Register
 *				linesLSB		Position of the first bit of the register
 *				columnsDDR		Pointer to I/O Data Direction Register
 *				columnsPORT		Pointer to I/O Port Output Register
 *				columnsLSB		Position of the first bit of the register
 * Returns:		KEYPAD_OK
 * -------------------------------------------------------------------------- */

keypadResult_t keypadSetPort(volatile keypadConfiguration_t * keypad, volatile uint8 * linesDDR, volatile uint8 * linesPORT, volatile uint8 * linesPIN, uint8 linesLSB, volatile uint8 * columnsDDR, volatile uint8 * columnsPORT, uint8 columnsLSB)
{
	keypad->linesDDR = linesDDR;
	keypad->linesPORT = linesPORT;
	keypad->linesPIN = linesPIN;
	keypad->linesLSB = linesLSB;
	keypad->columnsDDR = columnsDDR;
	keypad->columnsPORT = columnsPORT;
	keypad->columnsLSB = columnsLSB;
	keypad->portConfigured = TRUE;

	return KEYPAD_OK;
}

/* -----------------------------------------------------------------------------
 * Function:	keypadSetKeys
 * Purpose:		Configures the key layout
 * Arguments:	keypad			Pointer to the keypad struct
 *				type			Size of the keypad (use enumerations at
 *								keypadType_t)
 *				...				List of values to be stored in the keypad struct
 * Returns:		KEYPAD_OK
 *				KEYPAD_TYPE_NOT_ALLOWED
 *				KEYPAD_MEMORY_ALLOCATION_ERROR
 * -------------------------------------------------------------------------- */

keypadResult_t keypadSetKeys(volatile keypadConfiguration_t * keypad, keypadType_t type, ...)
{
	uint8 i;
	uint8 j;
	va_list args;

	// Keypad type
	switch(type){
	case KEYPAD_4X4:
		keypad->type = KEYPAD_4X4;
		keypad->lines = 4;
		keypad->columns = 4;
		break;
	case KEYPAD_4X3:
		keypad->type = KEYPAD_4X3;
		keypad->lines = 4;
		keypad->columns = 3;
		break;
	case KEYPAD_5X3:
		keypad->type = KEYPAD_5X3;
		keypad->lines = 5;
		keypad->columns = 3;
		break;
	default:
		return KEYPAD_TYPE_NOT_ALLOWED;
	}

	// Memory allocation
	keypad->keys = (uint8 *)realloc(keypad->keys, keypad->lines * keypad->columns);
	if(keypad->keys == NULL)
		return KEYPAD_MEMORY_ALLOCATION_ERROR;

	// Keypad keys
	va_start(args, type);
	for(i = 0;i < keypad->lines;i++){
		for(j = 0;j < keypad->columns;j++){
			keypad->keys[(keypad->lines * i) + j] = (uint8)va_arg(args, int16);
		}
	}
	va_end(args);
	keypad->keysConfigured = TRUE;

	return KEYPAD_OK;
}

/* -----------------------------------------------------------------------------
 * Function:	keypadInit
 * Purpose:		Performs the keypad initialization
 * Arguments:	keypad			Pointer to the keypad struct
 *				debounceTime	Debouce time in miliseconds
 * Returns:		KEYPAD_OK
 *				KEYPAD_PORTS_NOT_CONFIGURED
 *				KEYPAD_KEYS_NOT_CONFIGURED
 *				KEYPAD_TYPE_NOT_ALLOWED
 * -------------------------------------------------------------------------- */

keypadResult_t keypadInit(volatile keypadConfiguration_t * keypad, uint8 debounceTime)
{
	if(keypad->portConfigured == FALSE)
		return KEYPAD_PORTS_NOT_CONFIGURED;

	if(keypad->keysConfigured == FALSE)
		return KEYPAD_KEYS_NOT_CONFIGURED;

	if(keypad->type == KEYPAD_TYPE_UNDEFINED)
		return KEYPAD_TYPE_NOT_ALLOWED;

	switch(keypad->type){
	case KEYPAD_4X4:
		clrMask(*(keypad->linesDDR), 0x0F, keypad->linesLSB);
		setMask(*(keypad->linesPORT), 0x0F, keypad->linesLSB);
		setMask(*(keypad->columnsDDR), 0x0F, keypad->columnsLSB);
		setMask(*(keypad->columnsPORT), 0x0F, keypad->columnsLSB);
		break;
	case KEYPAD_4X3:
		clrMask(*(keypad->linesDDR), 0x0F, keypad->linesLSB);
		setMask(*(keypad->linesPORT), 0x0F, keypad->linesLSB);
		setMask(*(keypad->columnsDDR), 0x07, keypad->columnsLSB);
		setMask(*(keypad->columnsPORT), 0x07, keypad->columnsLSB);
		break;
	case KEYPAD_5X3:
		clrMask(*(keypad->linesDDR), 0x1F, keypad->linesLSB);
		setMask(*(keypad->linesPORT), 0x1F, keypad->linesLSB);
		setMask(*(keypad->columnsDDR), 0x07, keypad->columnsLSB);
		setMask(*(keypad->columnsPORT), 0x07, keypad->columnsLSB);
		break;
	default:
		return KEYPAD_TYPE_NOT_ALLOWED;
	}
	keypad->debounceTime = debounceTime;
	keypad->initialized = TRUE;

	return KEYPAD_OK;
}

/* -----------------------------------------------------------------------------
 * Function:	keypadRead
 * Purpose:		Reads the keypad and returns the key pressed
 * Arguments:	keypad			Pointer to the keypad struct
 * Returns:		Value stored in the keypad stuct, initialized with keypadSetKeys
 * -------------------------------------------------------------------------- */

uint8 keypadRead(volatile keypadConfiguration_t * keypad)
{
	uint8 i;
	uint8 j;
	uint8 k;
	uint8 aux;
	uint8 key = 0xFF;

	for(i = 0;i < keypad->columns;i++){					// For each column
		cli();
		clrBit(*(keypad->columnsPORT), (i + keypad->columnsLSB));	// Clear one columns
		sei();
		noOperation(5);												// Wait for syncronization
		aux = *(keypad->linesPIN) >> keypad->linesLSB;
		for(j = 0;j < keypad->lines;j++){				// For each line
			if(isBitClr(aux, j)){					// Tests if the key is pressed
				key = keypad->keys[(keypad->lines * j) + i];			// Decodes the key using the table
				for(k = 0;k < keypad->debounceTime; k++)
					_delay_ms(1);		// Debounce time
			}
			while(isBitClr(*(keypad->linesPIN) >> keypad->linesLSB,j)){
			}
				;	// Wait until key is released
		}
		cli();
		setBit(*(keypad->columnsPORT), (i + keypad->columnsLSB));	// Restore column value
		sei();
	}

	return key;
}