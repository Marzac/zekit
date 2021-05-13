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

#include "midi.h"
#include "mseq.h"
#include "audio.h"
#include "pins.h"

#include <stdint.h>
#include <stdlib.h>
#include <xc.h>

/******************************************************************************/
MIDINote midiNotes[MIDI_MAXNOTES];

uint16_t midiBuffer[64];
static uint16_t midiRd;
static uint8_t  midiBytes[4];
static uint16_t midiLength;
static uint16_t midiCount;

//static void midiNoteOn(uint8_t note, uint8_t velo);
//static void midiNoteOff(uint8_t note);

/******************************************************************************/
void midiInit()
{
	for (int i = 0; i < MIDI_MAXNOTES; i++) {
		MIDINote * n = &midiNotes[i];
		n->note = 0xFF;
        n->velo = 0;
	}

	midiRd = 0;
	midiBytes[0] = 0;
	midiBytes[1] = 0;
	midiBytes[2] = 0;
	midiBytes[3] = 0;
	midiLength = 0;
	midiCount = 0;
}

void midiUpdate()
{
	const uint8_t mLens[] = {
		3,	// Note off
		3,	// Note on
		3,	// Polyphonic key pressure
		3,	// Control change
		2,	// Program change
		2,	// Channel pressure
		3,	// Pitch bend
		1,	// Sysex / realtime
	};
	
	int dmaRd = 64 - DMACNT1;
	int len = (dmaRd - midiRd) & 63;

	for (int i = 0; i < len; i++) {
		uint8_t b = midiBuffer[midiRd];
		midiRd = (midiRd + 1) & 63;

	// Realtime messages
		if ((b & 0xF8) == 0xF8)
			continue;
	
	// New status
		if (b & 0x80) {
			midiBytes[0] = b;
			midiLength = mLens[(b >> 4) & 0x7];
			midiCount = 1; 
		}else{
			midiBytes[midiCount++] = b;
			if (midiCount >= 3) midiCount = 3;
		}
			
	/* Parse a complete message */
		if (!midiBytes[0]) continue;
		if (midiBytes[0] == 0xF0) continue;
		if (midiBytes[0] == 0xF7) continue;

		if (midiCount == midiLength) {
			if (midiBytes[0] == 0x90) {
				if (midiBytes[2]) seqNoteOn(midiBytes[1], midiBytes[2]);
				else seqNoteOff(midiBytes[1]);
			}else if (midiBytes[0] == 0x80) {
				seqNoteOff(midiBytes[1]);
			}
            midiCount = 1;
		}
	}
}

/******************************************************************************/
void midiNoteOn(uint8_t note, uint8_t velo)
{
	for (int i = 0; i < MIDI_MAXNOTES; i++) {
		MIDINote * n = &midiNotes[i];
		if (n->note != 0xFF) continue;
		n->note = note;
		n->velo = velo;
		return;
	}
	audioNoteOn(note, velo);
	//seqNoteOn(note, velo);
}

void midiNoteOff(uint8_t note)
{
	for (int i = 0; i < MIDI_MAXNOTES; i++) {
		MIDINote * n = &midiNotes[i];
		if (n->note != note) continue;
		n->note = 0xFF;
		n->velo = 0;
	}
	audioNoteOff(note);
	//seqNoteOff(note);
}
