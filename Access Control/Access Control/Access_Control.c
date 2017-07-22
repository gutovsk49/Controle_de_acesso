/* -----------------------------------------------------------------------------
 * Project:			Access Control 1.0
 * File:			Access_Control.c
 * Author:			Augusto Rodrigues
 * Created:			2017-07-11
 * Modified:		2017-07-20
 * Version:		     1.0
 * Purpose:			Implements a Access control device with LCD display and matricial keypad for numerical passwords.
 * Improvement:		 -	Associate numeric password with user ID
					 -	Machine State for display printing
 * -------------------------------------------------------------------------- */

// -----------------------------------------------------------------------------
// System definitions ----------------------------------------------------------

#define F_CPU 16000000UL

// -----------------------------------------------------------------------------
// Header files ----------------------------------------------------------------

#include <avr/eeprom.h>
#include <string.h>
#include "globalDefines.h"
#include "ATmega328.h"
#include "lcd4d.h"
#include "keypad.h"

// -----------------------------------------------------------------------------
// Project definitions ---------------------------------------------------------
#define		LinePort		PORTB												//
#define		LineDdr			DDRB												// Registradores gerais do port sado para linhas do teclado matricial
#define		LinePin			PINB												//

#define		ColumnPort		PORTC												// Registradores gerais do port sado para colunas do teclado matricial
#define		ColumnDdr		DDRC												//

#define		LcdControlDdr	DDRD												// Registradores gerais do port de controle do display lcd
#define		LcdControlPort	PORTD												//

#define		LcdDataDdr		DDRD												//
#define		LcdDataPort		PORTD												// Registradores gerais do port de dados do display lcd
#define		LCDDataPin		PIND												//

#define		LCD_ENABLE		PD4													// Pino de Enable do Displat lcd
#define		LCD_RESET		PD5													// Pino de Reset do Displat lcd


#define		GREEN_LED		PD7													// Pino do led verde
// -----------------------------------------------------------------------------
// New data types --------------------------------------------------------------
typedef union systemFlags_t{													//
	struct{																		//
		uint8 pwRegister	: 1;
		uint8 pwEntry		: 1;
		uint8 pwLoading		: 1;
		uint8 access		: 1;												//
		uint8 unusedBits	: 4;												//	Estrutura de flags para implementação de máquina de estados
	};																			//
	uint8 allFlags;																//
} systemFlags_t;																//
// -----------------------------------------------------------------------------
// Function declaration --------------------------------------------------------

// -----------------------------------------------------------------------------
// Global variables ------------------------------------------------------------
attachLcd(lcd_display);
attachKeypad(keypad);
systemFlags_t systemFlags;
// -----------------------------------------------------------------------------
// Main function ---------------------------------------------------------------

int main(void)
{
	setMask(LcdDataDdr, 0xFF, 0);
	setMask(ColumnDdr, 0x0F, 0);
	systemFlags.allFlags = 0;
	
	// Variable declaration
	uint8	aux1				= 0;
	uint8	actual_key			= 0xFF;
	uint8	password[4]			= {0,0,0,0};
	uint8	saved_passwords[64][4];
	uint8	pw_counter			= 0;
	uint8	digit_counter		= 0;
	uint8	right_digits		= 0;
	uint16	eeprom_write_addr	= 0x0000;
	
	//EEPROM Config
	eepromSetOperationMode(EEPROM_ERASE_WRITE);
	//eepromReadyActivateInterrupt();
	
	// Keypad Config
	keypadResetConfiguration(&keypad);
	keypadSetPort(&keypad, &LineDdr, &LinePort, &LinePin, PB0, &ColumnDdr, &ColumnPort, PC0);
	keypadSetKeys(&keypad, KEYPAD_4X4,  0x01, 0x02, 0x03, 0x0A,
										0x04, 0x05, 0x06, 0x0B,
										0x07, 0x08, 0x09, 0x0C,
										0x0E, 0x00, 0x0F, 0x0D);
	keypadInit(&keypad, 20);
	
	// LCD Configuration
	lcdResetConfiguration(&lcd_display);
	lcdClearScreen(&lcd_display);
	lcdSetControlPort(&lcd_display, &LcdControlDdr, &LcdControlPort, LCD_ENABLE, LCD_RESET);
	lcdSetDataPort(&lcd_display, &LcdDataDdr, &LcdDataPort, PD0);
	lcdInit(&lcd_display, LCD_16X2, LCD_FONT_5X8);
	lcdStdio(&lcd_display);
	
	// Variable initialization
	
	printf("AccessControl_v1\n");													//	
	_delay_ms(2000);															// tela de versão
	lcdClearScreen(&lcd_display);												//
	
	
	actual_key = keypadRead(&keypad);											// leitura de tecla para entrada no modo de cadastro de senha
	setBit(PORTD, GREEN_LED);
	
	if(actual_key==0x0E)														// Tratamento do cadastro de senha
	{
		systemFlags.pwRegister = 1;
		pw_counter = eepromRead(eeprom_write_addr);								// Primeira posição da eeprom guarda a quantidade de senhas salvas (MAX 64 SENHAS)
		if(pw_counter>64)														// Se já foram feitos mais de 64 cadastros de senhas ou outro dado aleatorio
		{																		//  neste slot de memoria, houve corrupção de dados inicia-se a sobrescrição de senhas
			eeprom_write_addr = 0x0000;
			eepromWrite(eeprom_write_addr, 0);
		}
		pw_counter = eepromRead(eeprom_write_addr);
		eeprom_write_addr = pw_counter*4 + 1;								// seta o endereço da proximo cadastro para logo após o ultimo cadastro
		
		while(systemFlags.pwRegister)																//
		{																							//						
			printf("Cadastre a senha\n");															//
																									// 
			while(digit_counter<4)																	//
			{																						//
				actual_key = keypadRead(&keypad);													//
																									//
				if(actual_key != 0xFF)																//
				{																					// Tratamento para captação dos 4 digitos da senha e sua gravação na eeprom
					password[digit_counter] = actual_key;											//
					eepromWrite((eeprom_write_addr+digit_counter), password[digit_counter]);		//
																									//
					printf("*");																	//
					lcdCursorMove(&lcd_display, RIGHT);												//
																									//
					digit_counter++;																//
				}
			}
			digit_counter	= 0;
			
			if(pw_counter<64)
				pw_counter++;
				
			eepromWrite(0x0000, pw_counter);
			eeprom_write_addr+=4;
			
			lcdClearScreen(&lcd_display);
			printf("     senha     \n   cadastrada   ");
			_delay_ms(2000);
			lcdClearScreen(&lcd_display);
			printf("(1)novo cadastro\n(2)continuar");
			
			actual_key = keypadRead(&keypad);
			
			while((actual_key != 0x01) & (actual_key != 0x02))
				actual_key = keypadRead(&keypad);
			
			lcdClearScreen(&lcd_display);
			if(actual_key==0x02)
			{
				systemFlags.pwRegister = 0;
				printf("    cadastro    \n    concluido   ");
				_delay_ms(2000);
			}
			
			else if(actual_key == 0x01)
			{
				systemFlags.pwRegister = 1;
				printf("  novo cadastro ");
				_delay_ms(2000);
			}
			lcdClearScreen(&lcd_display);
		}
		
	}
	
	pw_counter = eepromRead(0x0000);
	if(pw_counter>64)														// Se já foram feitos mais de 64 cadastros de senhas ou outro dado aleatorio
	{																		//  neste slot de memoria, houve corrupção de dados inicia-se a sobrescrição de senhas
		eeprom_write_addr = 0x0000;
		eepromWrite(0x0000, 0);
		pw_counter = eepromRead(0x0000);
	}
	printf("pw_counter = %d", pw_counter);
	_delay_ms(2000);
	lcdClearScreen(&lcd_display);
	
	while(aux1<pw_counter)													//
	{																		//	algoritmo de leitura das senhas da eeprom para RAM no inicio do programa						
		while(digit_counter<4)												//	PRECISA SER CORRIGIDO/MELHORADO, USAR STRUCT DE VETORES[?]
		{
			saved_passwords[aux1][digit_counter]=eepromRead(1+(aux1*4+digit_counter));
			digit_counter++;	
		}
		digit_counter = 0;
		printf("senha %d=%d%d%d%d", aux1, saved_passwords[aux1][0],saved_passwords[aux1][1],saved_passwords[aux1][2],saved_passwords[aux1][3]);
		_delay_ms(2000);
		lcdClearScreen(&lcd_display);
		aux1++;
	}																																										
	aux1 = 0;	
		
	// Enable Global Interrupts
	sei();
	
	while(1)
	{
		printf("Senha:");
		
		digit_counter = 0;
		while(digit_counter<4)
		{
			actual_key = keypadRead(&keypad);
			if(actual_key != 0xFF)
			{
				password[0]				= password[1];
				password[1]				= password[2];
				password[2]				= password[3];
				password[3]				= actual_key;
				digit_counter++;
				if(digit_counter==4)
					systemFlags.pwEntry		= 1;
				
				printf("*");																	//
				lcdCursorMove(&lcd_display, RIGHT);
			}	
		}
		digit_counter = 0;
		
		while(systemFlags.pwEntry)
		{
			lcdClearScreen(&lcd_display);
			printf("verificando...");
			_delay_ms(1000);
			
			aux1 = 0;
			while(aux1<pw_counter)													//
			{																		//	
				while(digit_counter<4)												//	
				{
					if(saved_passwords[aux1][digit_counter]==password[digit_counter])
						right_digits++;
					digit_counter++;
				}
				if(right_digits==4)
				{
					systemFlags.access = 1;
					systemFlags.pwEntry = 0;
					break;
				}
				else
					right_digits = 0;
					
				digit_counter = 0;
				aux1++;
			}
			aux1 = 0;
			lcdClearScreen(&lcd_display);
			if(systemFlags.access)
			{
				clrBit(PORTD, GREEN_LED);
				systemFlags.access = 0;
				printf("Liberado");
			}
			else
			{
				printf("acesso negado");
				systemFlags.pwEntry = 0;
			}
				
			_delay_ms(3000);
			setBit(PORTD, GREEN_LED);
		}
		 
		_delay_ms(1000);
		lcdClearScreen(&lcd_display);
	}

	return 0;
}

// -----------------------------------------------------------------------------
// Interruption handlers -------------------------------------------------------

// -----------------------------------------------------------------------------
// Function definitions --------------------------------------------------------

