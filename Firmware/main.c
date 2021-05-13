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
 
#include "setup.h"
#include "midi.h"
#include "mseq.h"
#include "audio.h"
#include "ui.h"

#include "pins.h"

#include <stdint.h>
#include <stdlib.h>
#include <xc.h>

uint16_t uwTick = 0;

/******************************************************************************/
#define BUT_SEQ_SetHigh()			(_LATB10 = 1)
#define BUT_SEQ_SetLow()			(_LATB10 = 0)
#define BUT_SEQ_GetValue()			_RB10
#define BUT_SEQ_SetDigitalInput()	(_TRISB10 = 1)
#define BUT_SEQ_SetDigitalOutput()	(_TRISB10 = 0)

#define BUT_TRIG_SetHigh()			(_LATB11 = 1)
#define BUT_TRIG_SetLow()			(_LATB11 = 0)
#define BUT_TRIG_GetValue()			_RB11
#define BUT_TRIG_SetDigitalInput()	(_TRISB11 = 1)
#define BUT_TRIG_SetDigitalOutput() (_TRISB11 = 0)

/******************************************************************************/
int main(void)
{
	setupInit();
	midiInit();
	audioInit();
	seqInit();
	uiInit();
	
	while (1)
	{
		uiUpdate();
		midiUpdate();
		seqUpdate();
	}

	return 1;
}
