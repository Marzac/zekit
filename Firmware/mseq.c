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

#include "mseq.h"
#include "audio.h"
#include "store.h"
#include "ui.h"

#include "pins.h"
#include "config.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

/*****************************************************************************/
// Sequencer functions
typedef struct {
	uint8_t state;
	uint16_t dt, plldt, stamp;
	uint16_t tick;

	uint8_t	pattern;
	uint8_t	nextPattern;
	uint8_t	step;
	bool halfStep;

	bool mustClear;

	uint8_t	root;
} Sequencer;
Sequencer seq;

static void seqHome();
static void seqClean();
static void seqPlay();

static void seqPlayBlinkFlash();
static void seqRecBlinkFlash();
static void seqTapBlinkFlash();
static void seqSaveBlinkFlash();

static void seqPatternsDefault();
static void seqPatternsLoad();
static void seqPatternsSave(int id);

/*****************************************************************************/
// Pattern functions
#define STEP_EMPTY		0x80
#define STEP_TIE		0x81

typedef struct {
	uint8_t root;
	uint8_t length;
	uint8_t id;
	uint8_t flags;
	uint8_t notes[SEQ_STEPS_MAX][SEQ_NOTES_MAX];
} Pattern;
Pattern patterns[SEQ_PATTERNS_MAX];

static void patternClear(Pattern * p);
static void patternInsert(Pattern * p, uint8_t note);
static void patternAdvance(Pattern * p);
static void patternLoad(uint32_t addr, Pattern * p);
static void patternSave(uint32_t addr, const Pattern * p);

/*****************************************************************************/
// Clocks and internal states
static bool extClock;
static bool midiClock;
static uint16_t extClockTicks;
static uint16_t midiClockEvents;

static uint16_t clockStamp;
static uint16_t tapStamps[3];
static uint16_t tapCount;

static int8_t lastNotes[SEQ_NOTES_MAX];

/*****************************************************************************/
// Sequencer UI
bool seqPlayBlink;
bool seqRecBlink;
bool seqTapBlink;
bool seqSaveBlink;
static uint16_t seqPlayBlinkStamp;
static uint16_t seqRecBlinkStamp;
static uint16_t seqTapBlinkStamp;
static uint16_t seqSaveBlinkStamp;

/******************************************************************************/
void mseqInit()
{
	seq.state = SEQ_RESET;
	seq.dt = 300000UL / (1200 * 2);
	seq.stamp = uwTick;
	seq.tick = 0;

	seq.pattern = 0;
	seq.nextPattern = 0;
	seq.step = 0;
	seq.halfStep = false;
	seq.mustClear = false;
	seq.root = 48;

	extClockTicks = 0;
	midiClockEvents = 0;

	clockStamp = uwTick;
	tapStamps[0] = uwTick;
	tapStamps[1] = uwTick;
	tapStamps[2] = uwTick;
	tapCount = 0;

	seqPlayBlink = false;
	seqRecBlink = false;
	seqTapBlink = false;
	seqSaveBlink = false;

	for (int n = 0; n < SEQ_NOTES_MAX; n++)
		lastNotes[n] = STEP_EMPTY;

	seqPatternsDefault();
	seqPatternsLoad();
}

void mseqUpdate()
{
	if (extClock || midiClock) {
		uint16_t dt = uwTick - seq.stamp;
		if (dt > SEQ_CLOCK_TIMEOUT) {
			seqClean();
			midiClock = extClock = false;
			seq.state = SEQ_RESET;
		}
	}

	if (seqPlayBlink) {
		uint16_t dt = uwTick - seqPlayBlinkStamp;
		if (dt > 125) seqPlayBlink = false;
	}

	if (seqRecBlink) {
		uint16_t dt = uwTick - seqRecBlinkStamp;
		if (dt > 125) seqRecBlink = false;
	}

	if (seqTapBlink) {
		uint16_t dt = uwTick - seqTapBlinkStamp;
		if (dt > 125) seqTapBlink = false;
	}

	if (seqSaveBlink) {
		uint16_t dt = uwTick - seqSaveBlinkStamp;
		if (dt > 1000) seqSaveBlink = false;
	}

	if (tapCount) {
		uint16_t dt = uwTick - tapStamps[tapCount-1];
		if (dt >= SEQ_CLOCK_TIMEOUT) tapCount = 0;
	}

	seqPlay();
}

/******************************************************************************/
MSEQ_STATES mseqGetState() {return seq.state;}

void mseqSetPattern(int pattern)
{
	seq.nextPattern = pattern;
	if (seq.state == SEQ_RESET)
		seq.pattern = seq.nextPattern;
}
int mseqGetPattern() {return seq.nextPattern;}

/******************************************************************************/
void mseqMIDITick()
{
	if (extClock || !midiClock)
		return;

	if (midiClockEvents) {
		midiClockEvents--;
		return;
	}

	midiClockEvents = 5;
	extClockTicks++;
}

void mseqMIDIStart()
{
	midiClock = true;
	midiClockEvents = 0;
	extClockTicks = 0;

	seqHome();
	seq.state = SEQ_PLAY;
}

void mseqMIDIContinue()
{
	midiClock = true;
	seq.state = SEQ_PLAY;
}

void mseqMIDIStop()
{
	midiClock = false;
	seq.state = SEQ_RESET;
	seqClean();
}

/******************************************************************************/
void mseqExtClockTick()
{
	if (!extClock) {
		clockStamp = uwTick;
		extClockTicks = 1;
		extClock = true;
		return;
	}

	uint16_t dt = uwTick - clockStamp;
	if (dt < 20) return;
	clockStamp = uwTick;

	seq.plldt = dt >> 1;
	extClockTicks++;
}

void mseqExtClockStart()
{
	seqHome();
	seq.state = SEQ_PLAY;
}

/******************************************************************************/
void seqHome()
{
	seq.step = 0;
	seq.halfStep = false;
	seq.tick = extClockTicks;
	seq.stamp = uwTick;
}

void seqClean()
{
	audioAllNotesOff();
	for (int n = 0; n < SEQ_NOTES_MAX; n++)
		lastNotes[n] = -1;
}

void seqPlay()
{
// Manage the clock sources
	if (extClock) {
		if (seq.tick == extClockTicks) {
			if (!seq.halfStep) return;
			uint16_t dt = uwTick - seq.stamp;
			if (dt < seq.plldt) return;
		}else seqTapBlinkFlash();
	}else if (midiClock) {
		if (seq.tick == extClockTicks) return;
		if (extClockTicks & 1) seqTapBlinkFlash();
	}else{
		uint16_t dt = uwTick - seq.stamp;
		if (dt < seq.dt) return;
	}

// Update sequencer state
	seq.stamp = uwTick;
	seq.tick = extClockTicks;
	if (seq.state != SEQ_PLAY)
		return;

// Change pattern
	if (!seq.step && !seq.halfStep)
		seq.pattern = seq.nextPattern;

// Play pattern notes
	Pattern * p = &patterns[seq.pattern];
	if (!p->length) return;
	int nextStep = (seq.step + 1) % p->length;

	for (int n = 0; n < SEQ_NOTES_MAX; n++) {
		int note = p->notes[seq.step][n];
		if (note == STEP_EMPTY) {
			if (lastNotes[n] > 0)
				audioNoteOff(lastNotes[n]);
			lastNotes[n] = -1;
		}else{
			if (seq.halfStep) {
				int next = p->notes[nextStep][n];
				if (next != STEP_TIE) {
					if (lastNotes[n] > 0)
						audioNoteOff(lastNotes[n]);
					lastNotes[n] = -1;
				}
			}else{
				if (note < STEP_EMPTY) {
					note += seq.root - p->root;
					if (note < 0) note = 0;
					if (note > 127) note = 127;
					audioNoteOn(note);
					lastNotes[n] = note;
				}
			}
		}
	}
				
// Advance the playback
	if (seq.halfStep) 
		seq.step = nextStep;
	seq.halfStep = !seq.halfStep;
	if (seq.halfStep) seqTapBlinkFlash();
}

/*****************************************************************************/
void mseqNoteOn(uint8_t note)
{
	if (seq.state == SEQ_RESET) {
		seqPlayBlinkFlash();
		audioNoteOn(note);
	}else if (seq.state == SEQ_PLAY) {
		seqPlayBlinkFlash();
		seq.root = note;
	}else if (seq.state == SEQ_RECORD) {
		seqRecBlinkFlash();
		Pattern * p = &patterns[seq.pattern];
		if (seq.mustClear) {
			patternClear(p);
			seq.mustClear = false;
		}
		patternInsert(p, note);
		audioNoteOn(note);
	}
}

void mseqNoteOff(uint8_t note)
{
	if (seq.state == SEQ_RESET) {
		audioNoteOff(note);
	}else if (seq.state == SEQ_RECORD) {
		Pattern * p = &patterns[seq.pattern];
		if (p->notes[p->length][0] == note)
			patternAdvance(p);
	}
}

/*****************************************************************************/
void mseqPressSave()
{
	if (seq.state == SEQ_RESET) {
		if (!seqSaveBlink) {
			seqPatternsSave(seq.pattern);
			seqSaveBlinkFlash();
		}
	}else if (seq.state == SEQ_RECORD) {
		Pattern * p = &patterns[seq.pattern];
		patternInsert(p, STEP_EMPTY);
		patternAdvance(p);
	}
}

void mseqPressTap()
{
	if (seq.state == SEQ_PLAY) mseqTap();
	else if (seq.state == SEQ_RECORD) {
		Pattern * p = &patterns[seq.pattern];
		patternInsert(p, STEP_TIE);
		patternAdvance(p);
	}
}

void mseqPressRec()
{
	seqClean();
	if (seq.state == SEQ_RESET ||
		seq.state == SEQ_PLAY) {
		seq.step = 0;
		seq.mustClear = true;
		seq.state = SEQ_RECORD;
	}else if (seq.state == SEQ_RECORD) {
		seq.step = 0;
		seq.state = SEQ_RESET;
	}
}

void mseqPressPlay()
{
	seqClean();
	if (seq.state == SEQ_RESET ||
		seq.state == SEQ_RECORD) {
		seq.plldt = 0;
		seq.stamp = uwTick - seq.dt;
		seq.tick = extClockTicks;
		seq.step = 0;
		seq.halfStep = false;
		seq.root = patterns[seq.pattern].root;
		seq.state = SEQ_PLAY;
	}else seq.state = SEQ_RESET;
}

/******************************************************************************/
void mseqTap()
{
	tapStamps[tapCount++] = uwTick;
	if (tapCount != 3) return;
	tapCount = 0;

	uint16_t dt1 = tapStamps[1] - tapStamps[0];
	uint16_t dt2 = tapStamps[2] - tapStamps[1];
	seq.dt = (dt1 + dt2) >> (2 + 1);
	seq.stamp = uwTick;
}

/*****************************************************************************/
void seqPatternsDefault()
{
// Clear everything
	for (int k = 0; k < SEQ_PATTERNS_MAX; k++) {
		Pattern * p = &patterns[k];
		patternClear(p);
		p->id = k;
	}

// MONO	PATTERNS
// Pattern 1
	Pattern * p1 = &patterns[0];
	p1->root = NOTE_BASE;
	p1->length = 4;
	p1->notes[0][0] = NOTE_BASE;
	p1->notes[1][0] = NOTE_BASE;
	p1->notes[2][0] = NOTE_BASE;
	p1->notes[3][0] = NOTE_BASE;

// Pattern 2
	Pattern * p2 = &patterns[1];
	p2->root = NOTE_BASE;
	p2->length = 4;
	p2->notes[0][0] = NOTE_BASE;
	p2->notes[1][0] = NOTE_BASE + 12;
	p2->notes[2][0] = NOTE_BASE;
	p2->notes[3][0] = NOTE_BASE + 12;

// Pattern 3
	Pattern * p3 = &patterns[2];
	p3->root = NOTE_BASE;
	p3->length = 4;
	p3->notes[0][0] = NOTE_BASE;
	p3->notes[1][0] = NOTE_BASE + 12;
	p3->notes[2][0] = NOTE_BASE + 7;
	p3->notes[3][0] = NOTE_BASE + 12;

// Pattern 4
	Pattern * p4 = &patterns[3];
	p4->root = NOTE_BASE;
	p4->length = 4;
	p4->notes[0][0] = NOTE_BASE;
	p4->notes[1][0] = NOTE_BASE + 10;
	p4->notes[2][0] = NOTE_BASE + 12;
	p4->notes[3][0] = NOTE_BASE + 3;

// Pattern 5
	Pattern * p5 = &patterns[4];
	p5->root = NOTE_BASE;
	p5->length = 8;
	p5->notes[0][0] = NOTE_BASE;
	p5->notes[1][0] = STEP_EMPTY;
	p5->notes[2][0] = NOTE_BASE;
	p5->notes[3][0] = NOTE_BASE;
	p5->notes[4][0] = STEP_EMPTY;
	p5->notes[5][0] = NOTE_BASE;
	p5->notes[6][0] = NOTE_BASE;
	p5->notes[7][0] = STEP_EMPTY;

// Pattern 6
	Pattern * p6 = &patterns[5];
	p6->root = NOTE_BASE;
	p6->length = 8;
	p6->notes[0][0] = NOTE_BASE;
	p6->notes[1][0] = NOTE_BASE;
	p6->notes[2][0] = STEP_EMPTY;
	p6->notes[3][0] = NOTE_BASE;
	p6->notes[4][0] = STEP_EMPTY;
	p6->notes[5][0] = NOTE_BASE;
	p6->notes[6][0] = STEP_TIE;
	p6->notes[7][0] = STEP_EMPTY;

// Pattern 7
	Pattern * p7 = &patterns[6];
	p7->root = NOTE_BASE;
	p7->length = 8;
	p7->notes[0][0] = NOTE_BASE;
	p7->notes[1][0] = STEP_TIE;
	p7->notes[2][0] = NOTE_BASE + 7;
	p7->notes[3][0] = NOTE_BASE;
	p7->notes[4][0] = STEP_EMPTY;
	p7->notes[5][0] = NOTE_BASE + 12;
	p7->notes[6][0] = STEP_EMPTY;
	p7->notes[7][0] = NOTE_BASE + 12;

// Pattern 8
	Pattern * p8 = &patterns[7];
	p8->root = NOTE_BASE;
	p8->length = 8;
	p8->notes[0][0] = NOTE_BASE;
	p8->notes[1][0] = NOTE_BASE - 5;
	p8->notes[2][0] = NOTE_BASE + 7;
	p8->notes[3][0] = NOTE_BASE;
	p8->notes[4][0] = STEP_TIE;
	p8->notes[5][0] = STEP_EMPTY;
	p8->notes[6][0] = NOTE_BASE + 12;
	p8->notes[7][0] = STEP_EMPTY;

// POLY	PATTERNS
// Pattern 9
	Pattern * p9 = &patterns[8];
	p9->root = NOTE_BASE;
	p9->length = 4;
	p9->notes[0][0] = NOTE_BASE;
	p9->notes[1][0] = NOTE_BASE;
	p9->notes[1][1] = NOTE_BASE + 7;
	p9->notes[1][2] = NOTE_BASE + 15;
	p9->notes[2][0] = NOTE_BASE;
	p9->notes[3][0] = NOTE_BASE;
	p9->notes[3][1] = NOTE_BASE + 7;
	p9->notes[3][2] = NOTE_BASE + 15;

// Pattern 10
	Pattern * p10 = &patterns[9];
	p10->root = NOTE_BASE;
	p10->length = 4;
	p10->notes[0][0] = NOTE_BASE;
	p10->notes[1][0] = NOTE_BASE;
	p10->notes[1][1] = NOTE_BASE + 7;
	p10->notes[1][2] = NOTE_BASE + 16;
	p10->notes[2][0] = NOTE_BASE;
	p10->notes[3][0] = NOTE_BASE;
	p10->notes[3][1] = NOTE_BASE + 7;
	p10->notes[3][2] = NOTE_BASE + 16;

// Pattern 11
	Pattern * p11 = &patterns[10];
	p11->root = NOTE_BASE;
	p11->length = 4;
	p11->notes[0][0] = NOTE_BASE;
	p11->notes[1][0] = NOTE_BASE + 12;
	p11->notes[2][0] = NOTE_BASE;
	p11->notes[2][1] = NOTE_BASE + 7;
	p11->notes[3][0] = NOTE_BASE + 12;

// Pattern 12
	Pattern * p12 = &patterns[11];
	p12->root = NOTE_BASE;
	p12->length = 4;
	p12->notes[0][0] = NOTE_BASE;
	p12->notes[1][0] = NOTE_BASE + 12;
	p12->notes[2][0] = NOTE_BASE;
	p12->notes[2][1] = NOTE_BASE + 10;
	p12->notes[3][0] = NOTE_BASE + 12;

// Pattern 13
	Pattern * p13 = &patterns[12];
	p13->root = NOTE_BASE;
	p13->length = 8;
	p13->notes[0][0] = NOTE_BASE;
	p13->notes[0][1] = NOTE_BASE + 12;
	p13->notes[1][0] = NOTE_BASE;
	p13->notes[2][0] = STEP_EMPTY;
	p13->notes[3][0] = STEP_EMPTY;
	p13->notes[4][0] = NOTE_BASE;
	p13->notes[4][1] = NOTE_BASE + 5;
	p13->notes[5][0] = NOTE_BASE;
	p13->notes[5][1] = NOTE_BASE + 12;
	p13->notes[6][0] = STEP_EMPTY;
	p13->notes[7][0] = STEP_EMPTY;

// Pattern 14
	Pattern * p14 = &patterns[13];
	p14->root = NOTE_BASE;
	p14->length = 8;
	p14->notes[0][0] = NOTE_BASE;
	p14->notes[0][1] = NOTE_BASE + 12;
	p14->notes[1][0] = STEP_EMPTY;
	p14->notes[2][0] = NOTE_BASE;
	p14->notes[3][0] = STEP_EMPTY;
	p14->notes[4][0] = NOTE_BASE;
	p14->notes[4][1] = NOTE_BASE + 7;
	p14->notes[5][0] = NOTE_BASE;
	p14->notes[5][1] = NOTE_BASE + 12;
	p14->notes[6][0] = STEP_EMPTY;
	p14->notes[7][0] = NOTE_BASE;

// Pattern 15
	Pattern * p15 = &patterns[14];
	p15->root = NOTE_BASE;
	p15->length = 8;
	p15->notes[0][0] = NOTE_BASE;
	p15->notes[0][1] = NOTE_BASE + 7;
	p15->notes[1][0] = STEP_TIE;
	p15->notes[1][1] = STEP_TIE;
	p15->notes[2][0] = NOTE_BASE + 12;
	p15->notes[3][0] = STEP_EMPTY;
	p15->notes[4][0] = NOTE_BASE + 5;
	p15->notes[5][0] = STEP_EMPTY;
	p15->notes[6][0] = NOTE_BASE + 5;
	p15->notes[7][0] = NOTE_BASE + 1;
	p15->notes[7][1] = NOTE_BASE + 5;

// Pattern 16
	Pattern * p16 = &patterns[15];
	p16->root = NOTE_BASE;
	p16->length = 8;
	p16->notes[0][0] = NOTE_BASE;
	p16->notes[1][0] = NOTE_BASE + 7;
	p16->notes[1][1] = NOTE_BASE + 12;
	p16->notes[2][0] = NOTE_BASE;
	p16->notes[3][0] = STEP_EMPTY;
	p16->notes[4][0] = NOTE_BASE;
	p16->notes[5][0] = NOTE_BASE + 10;
	p16->notes[5][1] = NOTE_BASE + 12;
	p16->notes[6][0] = STEP_TIE;
	p16->notes[6][1] = STEP_TIE;
	p16->notes[7][0] = NOTE_BASE + 5;
}

/*****************************************************************************/
void seqPatternsLoad()
{
// Browse all patterns records
	uint32_t addr = PATTERNS_ADDR;
	for (int s = 0; s < STORE_PATTERNS_PER_PAGE * SEQ_PATTERNS_MAX; s++) {
	// Load a pattern header
		uint32_t header;
		uint8_t * bytes = (uint8_t *) &header;
		storeRead32(addr, &header);
		if (header == 0xFFFFFFFF ||
			header == 0x00000000) {
			addr += STORE_PATTERN_SIZE;
			continue;
		}

	// Check pattern content
		uint8_t len = bytes[1];
		uint8_t id	= bytes[2];
		if (len > 0 &&
			len < SEQ_STEPS_MAX &&
			id < SEQ_PATTERNS_MAX)
			patternLoad(addr, &patterns[id]);
		addr += STORE_PATTERN_SIZE;
	}
}

void seqPatternsSave(int id)
{
// Save on a free record
	uint32_t addr = STORE_PATTERNS_ADDR(id);
	for (int s = 0; s < STORE_PATTERNS_PER_PAGE; s++) {
		uint32_t header;
		storeRead32(addr, &header);
		if (header == 0xFFFFFFFF) {
			patternSave(addr, &patterns[id]);
			return;
		}
		addr += STORE_PATTERN_SIZE;
	}

// Clear the whole page and save
	addr = STORE_PATTERNS_ADDR(id);
	storeErasePage(addr);
	patternSave(addr, &patterns[id]);
}

/*****************************************************************************/
void patternClear(Pattern * p)
{
	p->root = NOTE_BASE;
	p->length = 0;
	p->flags = 0;
	for (int s = 0; s < SEQ_STEPS_MAX; s++)
	for (int n = 0; n < SEQ_NOTES_MAX; n++)
		p->notes[s][n] = STEP_EMPTY;
}

void patternInsert(Pattern * p, uint8_t note)
{
// Insert one note
	if (note < STEP_EMPTY) {
		if (!p->length)	p->root = note;
		uint8_t * notes = p->notes[p->length];
		for (int n = 0; n < SEQ_NOTES_MAX; n++) {
			if (notes[n] != STEP_EMPTY) continue;
			notes[n] = note;
			break;
		}
		return;
	}

// Insert an event
	if (!p->length)	p->root = NOTE_BASE;
	uint8_t * notes = p->notes[p->length];
	for (int n = 0; n < SEQ_NOTES_MAX; n++)
		notes[n] = note;
}

void patternAdvance(Pattern * p)
{
	seqClean();
	if (p->length < SEQ_STEPS_MAX-1)
		p->length++;
	else{
		seq.step = 0;
		seq.halfStep = false;
		seq.state = SEQ_PLAY;
	}
}
void patternLoad(uint32_t addr, Pattern * p)
{
	uint32_t src = addr;
	uint32_t * dst = (uint32_t *) p;
	for (int i = 0; i < sizeof(Pattern) / sizeof(uint32_t); i++) {
		storeRead32(src, dst++);
		src += sizeof(uint32_t);
	}
}

void patternSave(uint32_t addr, const Pattern * p)
{
	uint32_t * src = (uint32_t *) p;
	uint32_t dst = addr;
	for (int i = 0; i < sizeof(Pattern) / sizeof(uint32_t); i++) {
		storeWrite32(dst, src++);
		dst += sizeof(uint32_t);
	}
}

/*****************************************************************************/
inline void seqPlayBlinkFlash()
{
	seqPlayBlinkStamp = uwTick;
	seqPlayBlink = true;
}

inline void seqRecBlinkFlash()
{
	seqRecBlinkStamp = uwTick;
	seqRecBlink = true;
}

inline void seqTapBlinkFlash()
{
	seqTapBlinkStamp = uwTick;
	seqTapBlink = true;
}

inline void seqSaveBlinkFlash()
{
	seqSaveBlinkStamp = uwTick;
	seqSaveBlink = true;
}
