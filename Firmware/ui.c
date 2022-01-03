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
	PAGE_NONE,
	PAGE_HOME,
	PAGE_WAVEFORM_SELECT,
	PAGE_SYSTEM_SELECT,
	PAGE_PATTERN_SELECT,
	PAGE_MIDI_SELECT,
	PAGE_CLOCKING_SELECT,
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

static int uiPageFromSwitch(int pressed);

/******************************************************************************/
void uiInit()
{
	LED_MODES_Blank();
	LED_SEQ_Blank();

	uiPage = PAGE_HOME;
	uiSystem = SYSTEM_ENV_RETRIG | SYSTEM_FILTER_TRACK;

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
// Cache the switch states
	uiSwitchesLast = uiSwitches;
	uiSwitches = 0;
	
// Translate the states	
	if (uiSwitchPortA & PORTA_TACT_PATTERN) 
		uiSwitches |= TACT_PATTERN;
	if (uiSwitchPortA & PORTA_TACT_SYSTEM) 
		uiSwitches |= TACT_SYSTEM;
	if (uiSwitchPortA & PORTA_TACT_WAVE) 
		uiSwitches |= TACT_WAVE;
	if (uiSwitchPortB & PORTB_TACT_PLAY) 
		uiSwitches |= TACT_PLAY;
	if (uiSwitchPortB & PORTB_TACT_REC) 
		uiSwitches |= TACT_REC;
	if (uiSwitchPortB & PORTB_TACT_TAP) 
		uiSwitches |= TACT_TAP;
	if (uiSwitchPortB & PORTB_TACT_SAVE) 
		uiSwitches |= TACT_SAVE;
}

/******************************************************************************/
inline void uiEvents()
{
	uint16_t toggle = uiSwitches ^ uiSwitchesLast;
	uint16_t pressed = (~uiSwitches) & toggle;
	uint16_t code = pressed >> 3;

// Trigger functions
	int value;
	if (uiPage == PAGE_HOME) {
		if (pressed & TACT_SAVE) mseqPressSave();
		if (pressed & TACT_TAP)	 mseqPressTap();
		if (pressed & TACT_REC)	 mseqPressRec();
		if (pressed & TACT_PLAY) mseqPressPlay();
	}else if (code) {
		switch (uiPage) {
		case PAGE_WAVEFORM_SELECT:
			value = audioGetWave();
			audioSetWave(value ^ code);
			break;

		case PAGE_SYSTEM_SELECT:
			uiSystem ^= code;
			break;

		case PAGE_PATTERN_SELECT:
			value = mseqGetPattern();
			mseqSetPattern(value ^ code);
			break;

		case PAGE_MIDI_SELECT:
			value = midiGetChannel();
			midiSetChannel(value ^ code);
			break;
		
		case PAGE_CLOCKING_SELECT:
			value = mseqGetClocking();
			mseqSetClocking(value ^ code);
			break;

		default: break;
		}
	}

// Exit when recording
	if (mseqGetState() == MSEQ_STATE_RECORD)
		return;

// Manage the pages
	int newPage = uiPageFromSwitch(pressed);
	if (newPage == PAGE_NONE) return;
	if (newPage == PAGE_MIDI_SELECT) 
		audioSetWheel(0);	// Stop the vibrato
	if (newPage == uiPage) newPage = PAGE_HOME;
	else uiBlinkStamp = uwTick;

	int lastPage = uiPage;
	uiPage = newPage;
	
// Save globals on leaving
	if (lastPage == PAGE_MIDI_SELECT ||
		lastPage == PAGE_CLOCKING_SELECT)
		uiSaveGlobals();
}


int uiPageFromSwitch(int pressed)
{
	if (pressed & TACT_WAVE) {
		if (!(uiSwitches & TACT_PATTERN)) return PAGE_MIDI_SELECT;
		if (!(uiSwitches & TACT_SYSTEM)) return PAGE_CLOCKING_SELECT;
		return PAGE_WAVEFORM_SELECT;
	}
	
	if (pressed & TACT_SYSTEM) {
		if (!(uiSwitches & TACT_WAVE)) return PAGE_CLOCKING_SELECT;
		return PAGE_SYSTEM_SELECT;
	}
	
	if (pressed & TACT_PATTERN) {
		if (!(uiSwitches & TACT_WAVE)) return PAGE_MIDI_SELECT;
		return PAGE_PATTERN_SELECT;
	}
	
	return PAGE_NONE;
}

/******************************************************************************/
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
		display = midiGetChannel();
		break;

	case PAGE_CLOCKING_SELECT:
		if (blink) {
			LED_WAVE_SetHigh();
			LED_SYSTEM_SetHigh();
		}
		display = mseqGetClocking();
		break;
		
	default: {
		MSEQ_STATES state = mseqGetState();
		if (seqPlayBlink ^ (state == MSEQ_STATE_PLAY)) display |= 0x01;
		if (seqRecBlink ^ (state == MSEQ_STATE_RECORD)) display |= 0x02;
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
	uint32_t channel, clocking, tuning;
	storeRead32(GLOBAL_MIDICHANNEL_ADDR, &channel);
	storeRead32(GLOBAL_CLOCKING_ADDR, &clocking);
	storeRead32(GLOBAL_FRCTUNING_ADDR, &tuning);
	if (channel > 15) channel = 0;
	if (clocking > 15) clocking = 3;
	if (tuning > 63) tuning = 32;

// Apply the globals
	midiSetChannel(channel);
	mseqSetClocking(clocking);
	OSCTUN = (tuning - 32) & 0x1F;
}

void uiSaveGlobals()
{
// Load globals content
	uint32_t oldChannel, oldClocking, oldTuning;
	storeRead32(GLOBAL_MIDICHANNEL_ADDR, &oldChannel);
	storeRead32(GLOBAL_CLOCKING_ADDR, &oldClocking);
	storeRead32(GLOBAL_FRCTUNING_ADDR, &oldTuning);

// Compare to new configuration
	uint32_t tuning = ((int16_t) (OSCTUN << 10) >> 10) + 32;
	uint32_t channel = midiGetChannel();
	uint32_t clocking = mseqGetClocking();
	
	if (channel == oldChannel &&
		clocking == oldClocking &&
		tuning == oldTuning) return;

// Update the globals
	storeErasePage(GLOBALS_ADDR);
	storeWrite32(GLOBAL_MIDICHANNEL_ADDR, &channel);
	storeWrite32(GLOBAL_CLOCKING_ADDR, &clocking);
	storeWrite32(GLOBAL_FRCTUNING_ADDR, &tuning);
}

void uiFRCTuning(int value)
{
// Adjust MCU FRC frequency
	if (uiPage != PAGE_MIDI_SELECT)	return;
	audioSetWheel(0);
	OSCTUN = (value >> 1) - 32;
}
