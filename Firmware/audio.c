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

#include "audio.h"
#include "midi.h"
#include "ui.h"
#include "pins.h"

#include <stdint.h>
#include <stdlib.h>
#include <xc.h>

/******************************************************************************/
int16_t audioBuffer[BUFFER_LEN * 2];
uint8_t audioMode;

#define MAXNOTES 4
static uint16_t notesCount;
static uint32_t notesInc[MAXNOTES];
static uint8_t  notesMIDI[MAXNOTES];
static int32_t oscCounts[MAXNOTES*2];
static int16_t oscScales[MAXNOTES*2];
static int16_t oscLevels[MAXNOTES*2];

/******************************************************************************/
void audioInit()
{
	audioMode = 0;
	notesCount = 0;
	for (int i = 0; i < MAXNOTES; i++) {
		notesInc[i] = 0;
		notesMIDI[i] = 0xFF;
	}
	
	for (int i = 0; i < MAXNOTES*2; i++) {
		oscCounts[i] = 0;
		oscScales[i] = 0;
		oscLevels[i] = 0;
	}
}

/******************************************************************************/
static const uint32_t pt[] = {
	(uint32_t) (0x1p24f * 4186.00f / SAMPLE_FRQ),
	(uint32_t) (0x1p24f * 4434.92f / SAMPLE_FRQ),
	(uint32_t) (0x1p24f * 4698.64f / SAMPLE_FRQ),
	(uint32_t) (0x1p24f * 4978.03f / SAMPLE_FRQ),
	(uint32_t) (0x1p24f * 5274.04f / SAMPLE_FRQ),
	(uint32_t) (0x1p24f * 5587.65f / SAMPLE_FRQ),
	(uint32_t) (0x1p24f * 5919.91f / SAMPLE_FRQ),
	(uint32_t) (0x1p24f * 6271.93f / SAMPLE_FRQ),
	(uint32_t) (0x1p24f * 6644.88f / SAMPLE_FRQ),
	(uint32_t) (0x1p24f * 7040.00f / SAMPLE_FRQ),
	(uint32_t) (0x1p24f * 7458.62f / SAMPLE_FRQ),
	(uint32_t) (0x1p24f * 7902.13f / SAMPLE_FRQ),
};

/******************************************************************************/
inline void audioTrigger()
{
	CM1CONbits.CEVT = 0;
	GATE_VCA_SetHigh();
	GATE_VCF_SetHigh();
}

inline void audioRelease()
{
	GATE_VCA_SetLow();
	GATE_VCF_SetLow();
}

/******************************************************************************/
void audioNoteOn(uint8_t note, uint8_t velo)
{
	uint16_t octave = note / 12;
	uint16_t degree = note % 12;
	uint32_t inc = pt[degree];
	inc = inc >> (9 - octave);
	
	if (!audioMode) {
		oscCounts[0] = 0;
		oscCounts[1] = 0;
		oscCounts[2] = 0;
		oscCounts[3] = 0;
		oscScales[0] = 256;
		oscScales[1] = 128;
		oscScales[2] = 64;
		oscScales[3] = 32;
		oscLevels[0] = 2;
		oscLevels[1] = 2;
		oscLevels[2] = 4;
		oscLevels[3] = 6;
		
		notesInc[0] = inc;
		notesMIDI[0] = note;
		notesCount = 1;		
		audioTrigger();
		
	}else{
		if (!notesCount) {
			for (int i = 0; i < MAXNOTES; i++)
				notesInc[i] = 0;
			audioTrigger();
		}
		for (int i = 0; i < MAXNOTES; i++) {
			if (notesMIDI[i] != 0xFF) 
				continue;
			uint16_t o = i * 2;
			oscCounts[o]   = 0;
			oscCounts[o+1] = 0;
			oscScales[o]   = 256;
			oscScales[o+1] = 128;
			oscLevels[o]   = 3;
			oscLevels[o+1] = 3;
			notesInc[i] = inc;
			notesMIDI[i] = note;
			notesCount++;
			break;
		}
	}
}

void audioNoteOff(uint8_t note)
{
	for (int i = 0; i < MAXNOTES; i++) {
		if (notesMIDI[i] != note)
			continue;
		notesMIDI[i] = 0xFF;
		notesCount--;
	}
	if (notesCount < 0) notesCount = 0;
	if (!notesCount) audioRelease();
}

/******************************************************************************/

/*
	counts[0] = 0;
	counts[1] = 0x80000000;
	counts[2] = 0;
	counts[3] = 0;
	scales[0] = 256;
	scales[1] = 0;
	scales[2] = 0;
	levels[0] = 2;
	levels[1] = 16;
	levels[2] = 16;
*/	
/*	
	counts[0] = 0;
	counts[1] = 0;
	counts[2] = 0;
	counts[3] = 0;
	scales[0] = 127;
	scales[1] = 64;
	scales[2] = 32;
	levels[0] = 2;
	levels[1] = 4;
	levels[2] = 6;
*/

/******************************************************************************/
void audioToggleMode()
{
	audioMode ^= 1;
	ledsState &= ~BUT_PARA;
	if (audioMode) ledsState |= BUT_PARA;
}

/******************************************************************************/
void audioRender(int16_t * buffer)
{
	if (!audioMode) {
		uint32_t inc = notesInc[0];
		uint32_t register incs[4];
		incs[0] = inc * (uint32_t) oscScales[0];
		incs[1] = inc * (uint32_t) oscScales[1];
		incs[2] = inc * (uint32_t) oscScales[2];
		incs[3] = inc * (uint32_t) oscScales[3];	

		for (int i = 0; i < BUFFER_LEN / 2; i++) {	
			oscCounts[0] += incs[0];
			oscCounts[1] += incs[1];
			oscCounts[2] += incs[2];
			oscCounts[3] += incs[3];
			
			int16_t v = 0;
			int16_t a = oscCounts[0] >> 16;
			v += a >> oscLevels[0];
			int16_t b = oscCounts[1] >> 16;
			v -= b >> oscLevels[1];
			int16_t c = oscCounts[2] >> 16;
			v += c >> oscLevels[2];
			int16_t d = oscCounts[3] >> 16;
			v -= d >> oscLevels[3];
			*buffer++ = 0;
			*buffer++ = v;
		}
	}else{

		uint32_t inc;
		uint32_t register incs[8];
		inc = notesInc[0];
		incs[0] = inc * (uint32_t) oscScales[0];
		incs[1] = inc * (uint32_t) oscScales[1];
		inc = notesInc[1];
		incs[2] = inc * (uint32_t) oscScales[2];
		incs[3] = inc * (uint32_t) oscScales[3];	
		inc = notesInc[2];
		incs[4] = inc * (uint32_t) oscScales[4];
		incs[5] = inc * (uint32_t) oscScales[5];
		inc = notesInc[3];
		incs[6] = inc * (uint32_t) oscScales[6];
		incs[7] = inc * (uint32_t) oscScales[7];
		
		for (int i = 0; i < BUFFER_LEN / 2; i++) {	
			oscCounts[0] += incs[0];
			oscCounts[1] += incs[1];
			oscCounts[2] += incs[2];
			oscCounts[3] += incs[3];
			oscCounts[4] += incs[4];
			oscCounts[5] += incs[5];
			oscCounts[6] += incs[6];
			oscCounts[7] += incs[7];
			
			int16_t v = 0;
			int16_t a = oscCounts[0] >> 16;
			v += a >> oscLevels[0];
			int16_t b = oscCounts[1] >> 16;
			v += b >> oscLevels[1];
			int16_t c = oscCounts[2] >> 16;
			v += c >> oscLevels[2];
			int16_t d = oscCounts[3] >> 16;
			v += d >> oscLevels[3];
			int16_t e = oscCounts[4] >> 16;
			v += e >> oscLevels[4];
			int16_t f = oscCounts[5] >> 16;
			v += f >> oscLevels[5];
			/*
			int16_t g = oscCounts[6] >> 16;
			v += g >> oscLevels[6];
			int16_t h = oscCounts[7] >> 16;
			v += h >> oscLevels[7];
			 */
			*buffer++ = 0;
			*buffer++ = v;
		}
	}
}

