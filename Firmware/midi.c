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

#include "midi.h"
#include "midi-defs.h"

#include "mseq.h"
#include "audio.h"
#include "store.h"
#include "ui.h"

#include "pins.h"
#include "config.h"

#include <xc.h>
#include <stdint.h>

/******************************************************************************/
static const uint8_t midiMsgLengths[] = {
	3,	// 0x8x - Note off
	3,	// 0x9x - Note on
	3,	// 0xAx - Polyphonic key pressure
	3,	// 0xBx - Control change
	2,	// 0xCx - Program change
	2,	// 0xDx - Channel pressure
	3,	// 0xEx - Pitch bend
	1,	// 0xFx - Sysex / realtime
};

/******************************************************************************/
uint16_t midiBuffer[MIDIRX_BUFFER_LEN];	// DMA RX buffer
uint8_t	 midiChannel;
uint8_t	 midiRT;

/******************************************************************************/
static uint16_t midiRd;
static uint8_t	midiBytes[4];
static uint16_t midiLength;
static uint16_t midiCount;

static void midiNoteOn(uint8_t note, uint8_t velo);
static void midiNoteOff(uint8_t note, uint8_t velo);

/******************************************************************************/
void midiInit()
{
	midiRd = 0;
	midiRT = 1;
	midiBytes[0] = 0;
	midiBytes[1] = 0;
	midiBytes[2] = 0;
	midiBytes[3] = 0;
	midiLength = 0;
	midiCount = 0;
}

void midiUpdate()
{
	int dmaRd = MIDIRX_BUFFER_LEN - DMACNT1;
	int len = (dmaRd - midiRd) & MIDIRX_BUFFER_MASK;

	for (int i = 0; i < len; i++) {
		uint8_t b = midiBuffer[midiRd];
		midiRd = (midiRd + 1) & MIDIRX_BUFFER_MASK;

	// Realtime messages
		if (midiRT && ((b & 0xF8) == 0xF8)) {
			switch(b) {
			case MIDI_TICK: mseqMIDITick(); break;
			case MIDI_START: mseqMIDIStart(); break;
			case MIDI_CONTINUE: mseqMIDIContinue(); break;
			case MIDI_STOP: mseqMIDIStop(); break;
			default: break;
			} continue;
		}

	// New MIDI status
		if (b & 0x80) {
			midiBytes[0] = b;
			midiLength = midiMsgLengths[(b >> 4) & 0x7];
			midiCount = 1;
		}else{
			midiBytes[midiCount++] = b;
			if (midiCount >= 3) midiCount = 3;
		}

	/* Parse a complete message */
		if (!midiBytes[0]) continue;
		if (midiBytes[0] == MIDI_SYSEX_BEGIN) continue;
		if (midiBytes[0] == MIDI_SYSEX_END) continue;

		if (midiCount == midiLength) {
			midiCount = 1;
			int status = midiBytes[0] & 0xF0;
			int channel = midiBytes[0] & 0x0F;
			if (channel != midiChannel) continue;

			switch(status) {
			case MIDI_NOTE_ON:
				if (midiBytes[2])
					midiNoteOn(midiBytes[1], midiBytes[2]);
				else midiNoteOff(midiBytes[1], 0);
				break;

			case MIDI_NOTE_OFF:
				midiNoteOff(midiBytes[1], midiBytes[2]);
				break;

			case MIDI_CC:

				switch(midiBytes[1]) {
				case MIDI_CC_MODWHEEL:
					uiFRCTuning(midiBytes[2]);
					break;

				case MIDI_CC_CUTOFF:
					audioSetCutoff(midiBytes[2]);
					break;

				case MIDI_CC_ALLNOTESOFF:
					audioAllNotesOff();
					break;

				case MIDI_CC_ALLSOUNDSOFF:
					audioAllSoundsOff();
					break;

				case MIDI_CC_RESETCTRLS:
					audioResetCtrls();
					break;

				case MIDI_CC_RTTOGGLE:
					midiRT = midiBytes[2];
					break;

				default: break;
				}
				break;

			case MIDI_PITCHBEND:
				audioSetBend((midiBytes[2] << 7) | midiBytes[1]);
				break;

			case MIDI_PROGRAM:
				audioSetWave(midiBytes[1] & 0x0F);
				break;

			default: break;
			}
		}
	}
}

/******************************************************************************/
void midiNoteOn(uint8_t note, uint8_t velo)
{
	mseqNoteOn(note);
}

void midiNoteOff(uint8_t note, uint8_t velo)
{
	mseqNoteOff(note);
}
