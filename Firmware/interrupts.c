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
 
#include <xc.h>
#include <stdint.h>

/******************************************************************************/
#include "audio.h"
void __attribute__((interrupt,no_auto_psv)) _DMA0Interrupt(void)
{
	if (DMAINT0bits.HALFIF) {
		audioRender(&audioBuffer[0]);
		DMAINT0bits.HALFIF = 0;
	}
	
	if (DMAINT0bits.DONEIF) {
		audioRender(&audioBuffer[BUFFER_LEN]);
		DMAINT0bits.DONEIF = 0;
	}
	
	IFS0bits.DMA0IF = 0;
}

void __attribute__((interrupt,no_auto_psv)) _CompInterrupt(void)
{
	_LATA2 = 1;	// Decay ENVF
	IFS1bits.CMIF = 0;
}

extern uint16_t uwTick;
void __attribute__((interrupt,no_auto_psv)) _T1Interrupt(void)
{
	uwTick++;
	IFS0bits.T1IF = 0;
}

