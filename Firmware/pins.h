/**
 * ZeKit Firmware
 * (c) Fred's Lab 2020-2021
 * fred@fredslab.net
 * 
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2021 Frédéric Meslin
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PINS_H
#define PINS_H

#define FCY_FRQ		16000000UL
#define TICK_FRQ    1000UL
#define PUMP_FRQ	150000UL
#define SAMPLE_FRQ	200000UL
#define BUFFER_LEN	128

extern uint16_t uwTick;

typedef enum{
	POT_CUTOFF	= 0x0001,
	POT_WAVE	= 0x0002,
	GATE_VCF	= 0x0004,
	GATE_VCA	= 0x0008,
}PORTA_PINS;

typedef enum {
	ICSP_PGD	= 0x0001,
	ICSP_PGC	= 0x0002,
	POT_LFO		= 0x0004,
	POT_PITCH	= 0x0008,
	MIDIIN		= 0x0010,
	PUMP		= 0x0020,
	BUT_PARA	= 0x0040,
	BUT_PLAY	= 0x0080,
	BUT_TAP		= 0x0100,
	BUT_RECORD	= 0x0200,
	BUT_SEQ		= 0x0400,
	BUT_TRIG	= 0x0800,
	ENV_VCF		= 0x1000,
	DAC_SCK		= 0x2000,
	DAC_WS		= 0x4000,
	DAC_SDO		= 0x8000,
}PORTB_PINS;

#define GATE_VCF_SetHigh()	(_LATA2 = 0)
#define GATE_VCF_SetLow()	(_LATA2 = 1)
#define GATE_VCA_SetHigh()	(_LATA3 = 0)
#define GATE_VCA_SetLow()	(_LATA3 = 1)

#endif