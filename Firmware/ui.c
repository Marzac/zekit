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

#include "ui.h"
#include "audio.h"
#include "midi.h"
#include "mseq.h"
#include "store.h"

#include "pins.h"
#include "config.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>


/******************************************************************************/
typedef enum {
	PAGE_HOME,
	PAGE_WAVEFORM_SELECT,
	PAGE_SYSTEM_SELECT,
	PAGE_PATTERN_SELECT,
	PAGE_MIDI_SELECT,
} UI_MODE;
UI_MODE uiPage;

uint16_t uiSystem;
uint16_t uiSwitches;
uint16_t uiSwitchesLast;
uint16_t uiBlinkStamp;

uint16_t uiSwitchPortA;
uint16_t uiSwitchPortB;

static void uiScan();
static void uiEvents();
static void uiDisplay();
static void uiLoadGlobals();
static void uiSaveGlobals();

/******************************************************************************/
void uiInit()
{
	LED_MODES_Blank();
	LED_SEQ_Blank();

	uiPage = PAGE_HOME;
	uiSystem = SYSTEM_ENVF_RETRIG | SYSTEM_ENVF_TRACK;

	uiSwitches = 0xFFFF;
	uiSwitchesLast = 0xFFFF;
	uiBlinkStamp = uwTick;

	uiSwitchPortA = 0xFFFF;
	uiSwitchPortB = 0xFFFF;

	uiLoadGlobals();
}

void uiUpdate()
{
	uiScan();
	uiEvents();
	uiDisplay();
}

/******************************************************************************/
inline void uiScan()
{
// Translate the states
	uiSwitchesLast = uiSwitches;
	uiSwitches = 0;
	if (uiSwitchPortA & PORTA_TACT_PATTERN) uiSwitches |= TACT_PATTERN;
	if (uiSwitchPortA & PORTA_TACT_SYSTEM) uiSwitches |= TACT_SYSTEM;
	if (uiSwitchPortA & PORTA_TACT_WAVE) uiSwitches |= TACT_WAVE;
	if (uiSwitchPortB & PORTB_TACT_PLAY) uiSwitches |= TACT_PLAY;
	if (uiSwitchPortB & PORTB_TACT_REC) uiSwitches |= TACT_REC;
	if (uiSwitchPortB & PORTB_TACT_TAP) uiSwitches |= TACT_TAP;
	if (uiSwitchPortB & PORTB_TACT_SAVE) uiSwitches |= TACT_SAVE;
}

/******************************************************************************/
inline void uiEvents()
{
	int selection;
	uint16_t toggle = uiSwitches ^ uiSwitchesLast;
	uint16_t pressed = (~uiSwitches) & toggle;
	uint16_t code = pressed >> 3;

// Trigger special functions
	switch (uiPage) {
	case PAGE_HOME:
		if (pressed & TACT_SAVE) mseqPressSave();
		if (pressed & TACT_TAP)	 mseqPressTap();
		if (pressed & TACT_REC)	 mseqPressRec();
		if (pressed & TACT_PLAY) mseqPressPlay();
		break;

	case PAGE_WAVEFORM_SELECT:
		if (!code) break;
		selection = audioGetWave();
		audioSetWave(selection ^ code);
		break;

	case PAGE_SYSTEM_SELECT:
		uiSystem ^= code;
		break;

	case PAGE_PATTERN_SELECT:
		if (!code) break;
		selection = mseqGetPattern();
		mseqSetPattern(selection ^ code);
		break;

	case PAGE_MIDI_SELECT:
		midiChannel ^= code;
		break;
	}

// Exit if recording mode
	if (mseqGetState() == SEQ_RECORD)
		return;

// Manage the pages
	int lastUIPage = uiPage;
	if (pressed & TACT_WAVE) {
		if ((uiSwitches & TACT_PATTERN) == 0)
			uiPage = PAGE_MIDI_SELECT;
		else {
			if (uiPage != PAGE_HOME)
				uiPage = PAGE_HOME;
			else uiPage = PAGE_WAVEFORM_SELECT;
		}
		uiBlinkStamp = uwTick;
	}

	if (pressed & TACT_SYSTEM) {
		if (uiPage != PAGE_HOME)
			uiPage = PAGE_HOME;
		else uiPage = PAGE_SYSTEM_SELECT;
		uiBlinkStamp = uwTick;
	}

	if (pressed & TACT_PATTERN) {
		if ((uiSwitches & TACT_WAVE) == 0)
			uiPage = PAGE_MIDI_SELECT;
		else{
			if (uiPage != PAGE_HOME)
				uiPage = PAGE_HOME;
			else uiPage = PAGE_PATTERN_SELECT;
		}
		uiBlinkStamp = uwTick;
	}

// Save globals on leaving
	if (lastUIPage == uiPage) return;
	if (lastUIPage == PAGE_MIDI_SELECT)
		uiSaveGlobals();
}

inline void uiDisplay()
{
	int display = 0;
	uint16_t dt = uwTick - uiBlinkStamp;
	bool blink = dt < 250;
	if (dt >= 500) uiBlinkStamp = uwTick;

// Display engine state
	LED_MODES_Blank();
	switch (uiPage) {
	case PAGE_WAVEFORM_SELECT:
		if (blink) LED_WAVE_SetHigh();
		display = audioGetWave();
		break;

	case PAGE_SYSTEM_SELECT:
		if (blink) LED_SYSTEM_SetHigh();
		display = uiSystem;
		break;

	case PAGE_PATTERN_SELECT:
		if (blink) LED_PATTERN_SetHigh();
		display = mseqGetPattern();
		break;

	case PAGE_MIDI_SELECT:
		if (blink) {
			LED_WAVE_SetHigh();
			LED_PATTERN_SetHigh();
		}
		display = midiChannel;
		break;

	default: {
		MSEQ_STATES state = mseqGetState();
		if (seqPlayBlink ^ (state == SEQ_PLAY)) display |= 0x01;
		if (seqRecBlink ^ (state == SEQ_RECORD)) display |= 0x02;
		if (seqTapBlink) display |= 0x04;
		if (seqSaveBlink) display |= 0x08;
	} break;
	}

// Display current selection
	LED_SEQ_Blank();
	if (display & 0x08) LED_SAVE_SetHigh();
	if (display & 0x04) LED_TAP_SetHigh();
	if (display & 0x02) LED_REC_SetHigh();
	if (display & 0x01) LED_PLAY_SetHigh();
}

/******************************************************************************/
void uiLoadGlobals()
{
// Load and sanitize globals
	uint32_t channel, tuning;
	storeRead32(GLOBAL_MIDICHANNEL_ADDR, &channel);
	storeRead32(GLOBAL_FRCTUNING_ADDR, &tuning);
	if (channel > 15) channel = 0;
	if (tuning > 63) tuning = 32;

// Apply the globals
	midiChannel = channel;
	OSCTUN = (tuning - 32) & 0x1F;
}

void uiSaveGlobals()
{
// Check globals content
	uint32_t channel, tuning;
	storeRead32(GLOBAL_MIDICHANNEL_ADDR, &channel);
	storeRead32(GLOBAL_FRCTUNING_ADDR, &tuning);
	uint32_t osctun = ((int16_t) (OSCTUN << 10) >> 10) + 32;

	if (channel == midiChannel &&
		tuning == osctun) return;

// Replace the globals
	channel = midiChannel;
	storeErasePage(GLOBALS_ADDR);
	storeWrite32(GLOBAL_MIDICHANNEL_ADDR, &channel);
	storeWrite32(GLOBAL_FRCTUNING_ADDR, &osctun);
}

void uiFRCTuning(int value)
{
// Adjust MCU FRC frequency
	if (uiPage != PAGE_MIDI_SELECT) return;
	OSCTUN = (value >> 1) - 32;
}
