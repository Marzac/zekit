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

#include "ui.h"
#include "pins.h"
#include "audio.h"
#include "mseq.h"

#include <stdint.h>
#include <stdlib.h>
#include <xc.h>

/******************************************************************************/
uint16_t switchesState;
uint16_t ledsState;

/******************************************************************************/
void uiInit()
{
	switchesState = 0;
	ledsState = 0;
}

void uiUpdate()
{
	LATB  |= 0x0FC0;
	TRISB |= 0x0FC0;
	__asm__ volatile ("repeat #15\n nop\n");

// Read switches state
	uint16_t last = switchesState;
	uint16_t state = (PORTB & 0x0FC0) ^ 0x0FC0;
	uint16_t toggle = state ^ last;
	switchesState = state;
	
	uint16_t on = switchesState & toggle;
	if (on & BUT_SEQ) seqPressNext();
	if (on & BUT_PLAY) seqPressPlay();
	if (on & BUT_TAP) seqPressTap();
	if (on & BUT_RECORD) seqPressRecord();
	if (on & BUT_TRIG) seqNoteOn(48, 127);
	if (on & BUT_PARA) audioToggleMode();

	uint16_t off = (~switchesState) & toggle;
	if (off & BUT_TRIG) seqNoteOff(48);
	
// Show switches state
	TRISB &= ~0x0FC0;
	LATB = (LATB & ~0x0FC0) | (ledsState ^ 0x0FC0);
}
