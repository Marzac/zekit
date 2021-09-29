/**
 * ZeKit Firmware
 * Copyright (C) 2021 - Frédéric Meslin
 * Contact: fred@fredslab.net

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.	 If not, see <https://www.gnu.org/licenses/>.
 */
/******************************************************************************/

#ifndef PINS_H
#define PINS_H

/******************************************************************************/
/** GPIOs definitions */
typedef enum{
	PORTA_CLOCK_START	= 0x0001,	// RA0
	PORTA_CLOCK_INPUT	= 0x0002,	// RA1
	PORTA_TACT_PATTERN	= 0x0004,	// RA2
	PORTA_TACT_SYSTEM	= 0x0008,	// RA3
	PORTA_TACT_WAVE		= 0x0010,	// RA4
}PORTA_PINS;

typedef enum {
	PORTB_ENV_VCA		= 0x0001,	// RB0
	PORTB_GATE_VCA		= 0x0002,	// RB1
	PORTB_ENV_VCF		= 0x0004,	// RB2
	PORTB_GATE_VCF		= 0x0008,	// RB3
	PORTB_MIDI_IN		= 0x0010,	// RB4
	PORTB_PUMP			= 0x0020,	// RB5
	PORTB_TACT_PLAY		= 0x0040,	// RB6
	PORTB_TACT_REC		= 0x0080,	// RB7
	PORTB_TACT_TAP		= 0x0100,	// RB8
	PORTB_TACT_SAVE		= 0x0200,	// RB9
	PORTB_ICSP_PGD		= 0x0400,	// RB10
	PORTB_ICSP_PGC		= 0x0800,	// RB11
	PORTB_RESERVED1		= 0x1000,	// RB12
	PORTB_DAC_SDO		= 0x2000,	// RB13
	PORTB_DAC_WS		= 0x4000,	// RB14
	PORTB_DAC_SCK		= 0x8000,	// RB15
}PORTB_PINS;

#define PORTA_TACTS		(PORTA_TACT_PATTERN | PORTA_TACT_SYSTEM | PORTA_TACT_WAVE)
#define PORTB_TACTS		(PORTB_TACT_PLAY | PORTB_TACT_REC | PORTB_TACT_TAP | PORTB_TACT_SAVE)

/******************************************************************************/
/** GPIOs helpers */
#define VCF_ENV_SetHigh()		(LATB &= ~PORTB_GATE_VCF)
#define VCF_ENV_SetLow()		(LATB |= PORTB_GATE_VCF)
#define VCA_ENV_SetHigh()		(LATB &= ~PORTB_GATE_VCA)
#define VCA_ENV_SetLow()		(LATB |= PORTB_GATE_VCA)
#define VCF_ENV_Read()			(LATB & PORTB_GATE_VCF)
#define VCA_ENV_Read()			(LATB & PORTB_GATE_VCA)

#define TACT_PATTERN_Read()		(PORTA & PORTA_TACT_PATTERN)
#define TACT_SYSTEM_Read()		(PORTA & PORTA_TACT_SYSTEM)
#define TACT_WAVE_Read()		(PORTA & PORTA_TACT_WAVE)
#define TACT_PLAY_Read()		(PORTB & PORTB_TACT_PLAY)
#define TACT_REC_Read()			(PORTB & PORTB_TACT_REC)
#define TACT_TAP_Read()			(PORTB & PORTB_TACT_TAP)
#define TACT_SAVE_Read()		(PORTB & PORTB_TACT_SAVE)

#define LED_PATTERN_SetHigh()	(LATA &= ~PORTA_TACT_PATTERN)
#define LED_SYSTEM_SetHigh()	(LATA &= ~PORTA_TACT_SYSTEM)
#define LED_WAVE_SetHigh()		(LATA &= ~PORTA_TACT_WAVE)
#define LED_PLAY_SetHigh()		(LATB &= ~PORTB_TACT_PLAY)
#define LED_REC_SetHigh()		(LATB &= ~PORTB_TACT_REC)
#define LED_TAP_SetHigh()		(LATB &= ~PORTB_TACT_TAP)
#define LED_SAVE_SetHigh()		(LATB &= ~PORTB_TACT_SAVE)

#define LED_PATTERN_SetLow()	(LATA |= PORTA_TACT_PATTERN)
#define LED_SYSTEM_SetLow()		(LATA |= PORTA_TACT_SYSTEM)
#define LED_WAVE_SetLow()		(LATA |= PORTA_TACT_WAVE)
#define LED_PLAY_SetLow()		(LATB |= PORTB_TACT_PLAY)
#define LED_REC_SetLow()		(LATB |= PORTB_TACT_REC)
#define LED_TAP_SetLow()		(LATB |= PORTB_TACT_TAP)
#define LED_SAVE_SetLow()		(LATB |= PORTB_TACT_SAVE)

#define LED_MODES_Blank()		(LATA |= PORTA_TACT_PATTERN | PORTA_TACT_SYSTEM | PORTA_TACT_WAVE)
#define LED_SEQ_Blank()			(LATB |= PORTB_TACT_PLAY | PORTB_TACT_REC | PORTB_TACT_TAP | PORTB_TACT_SAVE)

#endif
