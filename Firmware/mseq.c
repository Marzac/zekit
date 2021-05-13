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
 
#include "mseq.h"
#include "audio.h"
#include "ui.h"
#include "pins.h"

#include <stdint.h>
#include <stdlib.h>
#include <xc.h>

/*****************************************************************************/
#define PATTERNS_MAX	16
#define STEPS_MAX		16
#define NOTE_BASE		48

/*****************************************************************************/
typedef enum {
	SEQ_RESET = 0,
	SEQ_PLAY,
	SEQ_RECORD,
}State;
	
static uint8_t  seqState;
static uint16_t seqDT;

static uint8_t  seqPattern;
static uint8_t  seqStep;
static uint8_t  seqHalf;
static uint8_t  seqKey;

static uint16_t seqTapStamps[3];
static uint16_t seqTapCount;
static uint16_t seqPlayStamp;

static uint8_t  seqBlink;
static uint16_t seqBlinkStamp;

static int8_t   seqLastNote;
    
/*****************************************************************************/
#define STEP_EMPTY	0x80
#define STEP_TIE	0x81
typedef struct {
	uint8_t root;
	uint8_t notes[STEPS_MAX];
	uint8_t velos[STEPS_MAX];
	uint8_t length;
} Pattern;

Pattern patterns[PATTERNS_MAX];

void patternInit(Pattern * pattern);
void patternClear(Pattern * pattern);
void patternInsert(Pattern * pattern, uint8_t note, uint8_t velo);

/*****************************************************************************/
void seqInit()
{
    seqState = SEQ_RESET;
	seqDT = 300000UL / 1200;
	
    seqPattern = 0;
    seqStep = 0;
	seqHalf = 0;
    seqKey = 48;

    seqTapStamps[0] = uwTick;
    seqTapStamps[1] = uwTick;
    seqTapStamps[2] = uwTick;
    seqPlayStamp = uwTick;
	
	seqBlink = 0;
	seqBlinkStamp = 0;
    seqLastNote = -1;

    for (int s = 0; s < PATTERNS_MAX; s++)
        patternInit(&patterns[s]);
}

void seqUpdate()
{
	ledsState &= ~(BUT_PLAY | BUT_TAP | BUT_RECORD | BUT_SEQ);
	if (seqBlink) {
		uint16_t dt = uwTick - seqBlinkStamp;
		if (dt > 125) seqBlink = 0;
	}else{
		if (seqState == SEQ_PLAY) ledsState |= BUT_PLAY;
		else if (seqState == SEQ_RECORD) ledsState |= BUT_RECORD;
	}
	
    if (seqTapCount) {
        uint16_t dt = uwTick - seqTapStamps[seqTapCount-1];
        if (dt >= 4000) seqTapCount = 0;
    }

    if (seqState != SEQ_PLAY) return;
	
	uint16_t dt = uwTick - seqPlayStamp;
	if (dt < seqDT) return;
	seqPlayStamp = uwTick;
	
	Pattern * p = &patterns[seqPattern];
	int note = p->notes[seqStep];
	int velo = p->velos[seqStep];

	if (note == STEP_EMPTY) {
		if (seqLastNote >= 0)
			audioNoteOff(seqLastNote);
		seqLastNote = -1;
	}else if (note != STEP_TIE) {
		if (seqHalf) {
			if (seqLastNote >= 0)
				audioNoteOff(seqLastNote);
		}else{
			note += seqKey - p->root;
			if (note < 0) note = 0;
			if (note > 127) note = 127;
			audioNoteOn(note, velo);
			seqLastNote = note;
		}
	}
	
	if (seqHalf) {
		if (++seqStep >= p->length)
			seqStep = 0;
		seqHalf = 0;
	}else seqHalf = 1;
}

/*****************************************************************************/
void seqNoteOn(uint8_t note, uint8_t velo)
{
    if (seqState == SEQ_PLAY) {
		seqBlinkStamp = uwTick;
		seqBlink = 1;
        seqKey = note;
    }else if (seqState == SEQ_RECORD) {
		seqBlinkStamp = uwTick;
		seqBlink = 1;
        patternInsert(&patterns[seqPattern], note, velo);
		audioNoteOn(note, velo);
	}else if (seqState == SEQ_RESET) {
        audioNoteOn(note, velo);
	}
}

void seqNoteOff(uint8_t note)
{
    if (seqState == SEQ_RESET ||
		seqState == SEQ_RECORD);
        audioNoteOff(note);
}

/*****************************************************************************/
void seqPressTap()
{
    if (seqState == SEQ_PLAY) {
        seqTapStamps[seqTapCount++] = uwTick;
        if (seqTapCount != 3) return;
        uint16_t dt1 = seqTapStamps[1] - seqTapStamps[0];
        uint16_t dt2 = seqTapStamps[2] - seqTapStamps[1];
		seqDT = (dt1 + dt2) >> 3;
        seqTapCount = 0;
    } else if (seqState == SEQ_RECORD)
        patternInsert(&patterns[seqPattern], STEP_TIE, 0);
}

void seqPressNext()
{
    if (seqState == SEQ_PLAY) {
        seqStep = seqHalf = 0;
        seqPattern = (seqPattern + 1) & (PATTERNS_MAX - 1);
    }else if (seqState == SEQ_RECORD)
        patternInsert(&patterns[seqPattern], STEP_EMPTY, 0);
}

void seqPressRecord()
{
    if (seqState == SEQ_RESET ||
		seqState == SEQ_PLAY) {
        patternClear(&patterns[seqPattern]);
        seqStep = 0;
        seqState = SEQ_RECORD;
    }else if (seqState == SEQ_RECORD) {
        seqStep = 0;
        seqState = SEQ_RESET;
    }
}

void seqPressPlay()
{
    if (seqState == SEQ_RESET) {
        seqPlayStamp = uwTick - seqDT;
        seqStep = seqHalf = 0;
        seqState = SEQ_PLAY;
    }else if (seqState == SEQ_PLAY) {
        seqState = SEQ_RESET;
    }else if (seqState == SEQ_RECORD) {
		seqKey = patterns[seqPattern].root;
        seqPlayStamp = uwTick - seqDT;
		seqStep = seqHalf = 0;
        seqState = SEQ_PLAY;
    }
}

/*****************************************************************************/
void patternInit(Pattern * pattern)
{
    pattern->root = NOTE_BASE;
    pattern->length = 1;
    pattern->notes[0] = NOTE_BASE;
    pattern->velos[0] = 60;
}

void patternInsert(Pattern * pattern, uint8_t note, uint8_t velo)
{
    if (!pattern->length)
		pattern->root = note < 128 ? note : NOTE_BASE;
	
    pattern->notes[pattern->length] = note;
    pattern->velos[pattern->length] = velo;
    if (++pattern->length != STEPS_MAX)
		return;

    seqStep = 0;
	seqHalf = 0;
    seqState = SEQ_PLAY;
}

void patternClear(Pattern * pattern)
{
    pattern->root = NOTE_BASE;
    pattern->length = 0;
    for (int s = 0; s < STEPS_MAX; s++) {
        pattern->notes[s] = STEP_EMPTY;
        pattern->velos[s] = STEP_TIE;
    }
}
