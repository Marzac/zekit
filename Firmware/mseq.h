/**
 * ZeKit Firmware v2.0
 * Copyright (C) 2021/2022 - Frédéric Meslin
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

#ifndef MSEQ_H
#define MSEQ_H

	#include <stdint.h>
	#include <stdbool.h>

/******************************************************************************/
	#define SEQ_PATTERNS_MAX	16		// number of patterns
	#define SEQ_STEPS_MAX		96		// max: 127 steps
	#define SEQ_NOTES_MAX		4		// notes per step
	#define SEQ_CLOCK_TIMEOUT	1500	// timeout for clock switching

/******************************************************************************/
/* States and configuration */
	typedef enum {
		MSEQ_STATE_RESET = 0,
		MSEQ_STATE_PLAY,
		MSEQ_STATE_RECORD,
	}MSEQ_STATES;

	typedef enum {
		MSEQ_CLOCK_TAKE_NONE	= 0x0,
		MSEQ_CLOCK_TAKE_MIDI	= 0x1,
		MSEQ_CLOCK_TAKE_EXT		= 0x2,
		MSEQ_CLOCK_TAKE_BOTH	= 0x3,
		MSEQ_CLOCK_DIV_0		= 0x0,
		MSEQ_CLOCK_DIV_2		= 0x4,
		MSEQ_CLOCK_DIV_4		= 0x8,
	}MSEQ_CLOCKINGS;

/******************************************************************************/	
/* Base and pattern functions */
	void mseqInit();
	void mseqUpdate();
	MSEQ_STATES mseqGetState();
	
	void mseqSetPattern(int pattern);
	int mseqGetPattern();

/******************************************************************************/
/* Clock related functions */
	void mseqSetClocking(int config);
	int mseqGetClocking();
	
	void mseqMIDITick();
	void mseqMIDIStart();
	void mseqMIDIContinue();
	void mseqMIDIStop();

	void mseqExtClockTick();
	void mseqExtClockStart();

/******************************************************************************/
/* Notes on / off */
	void mseqNoteOn(uint8_t note);
	void mseqNoteOff(uint8_t note);

/******************************************************************************/
/* Sequencer UI functions */
	void mseqPressSave();
	void mseqPressTap();
	void mseqPressRec();
	void mseqPressPlay();
	void mseqTap();

	extern bool seqPlayBlink;
	extern bool seqRecBlink;
	extern bool seqTapBlink;
	extern bool seqSaveBlink;

#endif
