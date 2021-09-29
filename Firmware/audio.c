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

#include "audio.h"
#include "waves.h"
#include "midi.h"
#include "ui.h"

#include "pins.h"
#include "config.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************/
int16_t audioBuffer[AUDIO_BUFFER_LEN * 2];

static int waveform;
static int16_t pitchBend;
static int16_t cutoffMIDI;
static int16_t cutoffTrack;

static int voicesCount;
static int voicesMIDI[MAX_VOICES];
static uint32_t voicesPitch[MAX_VOICES];
static uint32_t voicesInc[MAX_VOICES];

static Sawer oscs[MAX_OSCS];

/******************************************************************************/
static void audioMuteOscs();
static void audioMonoNoteOn(uint8_t note, int wave);
static void audioParaNoteOn(uint8_t note, int wave);

static void audioComputePitch(int voice, int note);
static void audioUpdateTracking();
static void audioUpdateWaveforms();

static void audioRenderMono(int16_t * buffer, uint16_t cutoff);
static void audioRenderPara(int16_t * buffer, uint16_t cutoff);

/******************************************************************************/
void audioInit()
{
	waveform = 0;
	pitchBend = 0;
	cutoffMIDI = 0;
	cutoffTrack = 0;

	voicesCount = 0;
	audioMuteOscs();

	for (int i = 0; i < MAX_OSCS; i++) {
		Sawer * o = &oscs[i];
		o->phase = 0;
		o->rate = 0;
		o->shift = 0;
	}
}

void audioUpdate()
{
	static uint16_t stamp = 0;
	if (uwTick == stamp) return;
	stamp = uwTick;

	static int robin = 0;
	audioComputePitch(robin, voicesMIDI[robin]);
	robin = (robin + 1) & (MAX_VOICES - 1);
}

/******************************************************************************/
void audioSetWave(int wave)
{
	bool oldMono = IS_WAVEFORM_MONO(waveform);
	bool newMono = IS_WAVEFORM_MONO(wave);
	if (oldMono != newMono) audioAllSoundsOff();

	waveform = wave;
	audioUpdateWaveforms();
}

int audioGetWave() {return waveform;}

void audioSetBend(int16_t bend)
{
	pitchBend = ((int32_t) (bend - 0x2000) * BEND_RANGE) >> (13 - 8);
}

void audioSetCutoff(int16_t cutoff)
{
	cutoffMIDI = (cutoff - 64) << 9;
}

/******************************************************************************/
inline void audioTrigger()
{
	CVRCONbits.CVR = 24;		// Vref = 2.475V
	CM1CONbits.CPOL = 1;		// Inverse polarity
	VCA_ENV_SetHigh();
	VCF_ENV_SetHigh();
}

inline void audioRelease()
{
	VCA_ENV_SetLow();
	VCF_ENV_SetLow();
}

/******************************************************************************/
inline void audioMuteOscs()
{
	for (int i = 0; i < MAX_VOICES; i++) {
		voicesInc[i] = 0;
		voicesMIDI[i] = -1;
	}
}

/******************************************************************************/
void audioNoteOn(uint8_t note)
{
	if (IS_WAVEFORM_MONO(waveform))
		audioMonoNoteOn(note, waveform);
	else audioParaNoteOn(note, waveform - MAX_WAVES);
	audioUpdateTracking();
}

void audioNoteOff(uint8_t note)
{
	for (int i = 0; i < MAX_VOICES; i++) {
		if (voicesMIDI[i] != note) continue;
		voicesMIDI[i] = -1;
		if (voicesCount) voicesCount--;
	}
	if (!voicesCount) audioRelease();
}

void audioAllNotesOff()
{
	for (int i = 0; i < MAX_VOICES; i++)
		voicesMIDI[i] = -1;
	voicesCount = 0;
	audioRelease();
}

void audioAllSoundsOff()
{
	for (int i = 0; i < MAX_VOICES; i++) {
		voicesMIDI[i] = -1;
		voicesInc[i] = 0;
	}
	voicesCount = 0;
	audioRelease();
}

void audioResetCtrls()
{
	cutoffMIDI = 0;
}

/******************************************************************************/
void audioMonoNoteOn(uint8_t note, int wave)
{
	oscs[0] = wavesMono[wave][0];
	oscs[1] = wavesMono[wave][1];
	oscs[2] = wavesMono[wave][2];
	oscs[3] = wavesMono[wave][3];
	audioComputePitch(0, note);
	voicesCount = 1;

	audioTrigger();
}

void audioParaNoteOn(uint8_t note, int wave)
{
	bool trigger = !voicesCount;
	if (trigger) audioMuteOscs();

	for (int i = 0; i < MAX_VOICES; i++) {
		if (voicesMIDI[i] > 0) continue;
		oscs[i*2+0] = wavesPara[wave][0];
		oscs[i*2+1] = wavesPara[wave][1];
		audioComputePitch(i, note);
		voicesCount++;
		break;
	}

	bool always = uiSystem & SYSTEM_ENVF_RETRIG;
	if (trigger || always) audioTrigger();
}

/******************************************************************************/
static const uint32_t pt[] = {
	(uint32_t) (0x1p24f * 4186.009044809578f / FRQ_SAMPLE),
	(uint32_t) (0x1p24f * 4434.922095629953f / FRQ_SAMPLE),
	(uint32_t) (0x1p24f * 4698.636286678520f / FRQ_SAMPLE),
	(uint32_t) (0x1p24f * 4978.031739553295f / FRQ_SAMPLE),
	(uint32_t) (0x1p24f * 5274.040910605920f / FRQ_SAMPLE),
	(uint32_t) (0x1p24f * 5587.651702928062f / FRQ_SAMPLE),
	(uint32_t) (0x1p24f * 5919.910763386150f / FRQ_SAMPLE),
	(uint32_t) (0x1p24f * 6271.926975707989f / FRQ_SAMPLE),
	(uint32_t) (0x1p24f * 6644.875161279122f / FRQ_SAMPLE),
	(uint32_t) (0x1p24f * 7040.000000000000f / FRQ_SAMPLE),
	(uint32_t) (0x1p24f * 7458.620184289437f / FRQ_SAMPLE),
	(uint32_t) (0x1p24f * 7902.132820097988f / FRQ_SAMPLE),
	(uint32_t) (0x1p24f * 8372.018089619156f / FRQ_SAMPLE),
};

void audioUpdateTracking()
{
	int16_t newTrack = 0;

	for (int i = 0; i < MAX_VOICES; i++) {
		int note = voicesMIDI[i];
		if (note < 0) continue;
		if (note > 91) note = 91;	// G6 = ~1568Hz
		if (note < 28) note = 28;	// E1 = ~41Hz

		int16_t track = (note - TRACK_REF) * 990; // ~100% tracking
		if (track < newTrack) continue;
		newTrack = track;
	}

	cutoffTrack = newTrack;
}

void audioUpdateWaveforms()
{
	if (IS_WAVEFORM_MONO(waveform)) {
		int wave = waveform;
		oscs[0] = wavesMono[wave][0];
		oscs[1] = wavesMono[wave][1];
		oscs[2] = wavesMono[wave][2];
		oscs[3] = wavesMono[wave][3];
	}else{
		int wave = waveform - MAX_WAVES;
		for (int i = 0; i < MAX_VOICES; i++) {
			oscs[i*2+0] = wavesPara[wave][0];
			oscs[i*2+1] = wavesPara[wave][1];
		}
	}
}

void audioComputePitch(int voice, int note)
{
	if (note < 0) return;
	voicesMIDI[voice] = note;

// Clamp the pitch
	int16_t pitch = (note << 8) + pitchBend;
	if (pitch < 0) pitch = 0;
	if (pitch > 0x6000) {
		voicesInc[voice] = 0;
		voicesPitch[voice] = 0x6000l << GLIDE_SHIFT;
		return;
	}

// Compute glide effect
	voicesPitch[voice] += pitch - (voicesPitch[voice] >> GLIDE_SHIFT);
	if (uiSystem & SYSTEM_ENVF_GLIDE)
		pitch = voicesPitch[voice] >> GLIDE_SHIFT;

// Compute the final pitch
	int16_t coarse = pitch >> 8;
	int16_t fine = pitch & 0xFF;
	uint16_t octave = coarse / 12;
	uint16_t degree = coarse % 12;
	uint32_t i1 = pt[degree];
	uint32_t i2 = pt[degree+1];
	uint32_t inc = (((i2 - i1) * fine) >> 8) + i1;
	voicesInc[voice] = inc >> (9 - octave);
}

/******************************************************************************/
void audioRender(int16_t * buffer)
{
	//TRISA &= ~PORTA_TACT_WAVE;
	//LED_WAVE_SetHigh();

// Render digital oscillators
	int16_t cutoff = (uiSystem & SYSTEM_ENVF_TRACK) ? cutoffTrack : cutoffMIDI;
	if (IS_WAVEFORM_MONO(waveform))
		audioRenderMono(buffer, cutoff);
	else audioRenderPara(buffer, cutoff);

// Manage the analog envelopes
	if (CM1CONbits.COUT) {
		if (!CM1CONbits.CPOL) {			// Filter env. reached bottom
			if (uiSystem & SYSTEM_ENVF_LOOP) {
				CVRCONbits.CVR = 24;	// Vref = 2.475V
				CM1CONbits.CPOL = 1;	// Normal polarity
				VCF_ENV_SetHigh();		// Trigger the env.
			}
		}else{							// Filter env. reached top
			CVRCONbits.CVR = 1;			// Vref = 0.103V
			CM1CONbits.CPOL = 0;		// Inverted polarity
			VCF_ENV_SetLow();			// Release the env.
		}
	}

// Read switches state (in sync)
	TRISA |= PORTA_TACTS;
	TRISB |= PORTB_TACTS;
	__asm__ volatile ("repeat #10\n nop\n");
	uiSwitchPortA = PORTA;
	uiSwitchPortB = PORTB;
	TRISA &= ~PORTA_TACTS;
	TRISB &= ~PORTB_TACTS;

	//LED_WAVE_SetLow();
}

/******************************************************************************/
inline void audioRenderMono(int16_t * buffer, uint16_t cutoff)
{
	__asm volatile (" \
	; Process OSC1 and OSC2 (mono mode) \n \
		1:\n \
		mov _voicesInc, w0,\n \
		mov _voicesInc+2, w1\n \
		2: ;Compute OSCs increments\n \
		mov _oscs+4, w2\n \
		mul.us w0, w2, w4\n \
		mul.us w1, w2, w6\n \
		add w5, w6, w5\n \
		mov _oscs+12, w2\n \
		mul.us w0, w2, w6\n \
		mul.us w1, w2, w8\n \
		add w7, w8, w7\n \
		3: ;Load OSCs counters\n \
		mov _oscs+0, w8\n \
		mov _oscs+2, w9\n \
		mov _oscs+8, w10\n \
		mov _oscs+10, w11\n \
		mov %0, w2\n \
		add #256, w2\n \
		4: ;Update OSCs counters\n \
		add w8, w4, w8\n \
		addc w9, w5, w9\n \
		add w10, w6, w10\n \
		addc w11, w7, w11\n \
		5: ;Compute OSCs waveforms\n \
		mov _oscs+6, w0\n \
		mov _oscs+14, w1\n \
		asr w9, w0, w0\n \
		asr w11, w1, w1\n \
		inc2 %0, %0\n \
		add w0, w1, w0\n \
		asr w0, #1, w1\n \
		sub w0, w1, [%0++]\n \
		6: ; Loop over\n \
		cp %0, w2\n \
		bra nz, 4b\n \
		6: ; Write back counters\n \
		sub #256, %0\n \
		mov w8, _oscs+0\n \
		mov w9, _oscs+2\n \
		mov w10, _oscs+8\n \
		mov w11, _oscs+10\n \
	; Process OSC3 and OSC4 (mono mode) \n \
		1:\n \
		mov _voicesInc, w0,\n \
		mov _voicesInc+2, w1\n \
		2: ;Compute OSCs increments\n \
		mov _oscs+20, w2\n \
		mul.us w0, w2, w4\n \
		mul.us w1, w2, w6\n \
		add w5, w6, w5\n \
		mov _oscs+28, w2\n \
		mul.us w0, w2, w6\n \
		mul.us w1, w2, w8\n \
		add w7, w8, w7\n \
		3: ;Load OSCs counters\n \
		mov _oscs+16, w8\n \
		mov _oscs+18, w9\n \
		mov _oscs+24, w10\n \
		mov _oscs+26, w11\n \
		mov %0, w2\n \
		add #256, w2\n \
		4: ;Update OSCs counters\n \
		add w8, w4, w8\n \
		addc w9, w5, w9\n \
		add w10, w6, w10\n \
		addc w11, w7, w11\n \
		5: ;Compute OSCs waveforms\n \
		mov _oscs+22, w0\n \
		mov _oscs+30, w1\n \
		asr w9, w0, w0\n \
		asr w11, w1, w1\n \
		mov %1, [%0++]\n \
		add w0, w1, w0\n \
		asr w0, #1, w1\n \
		sub w0, w1, w0\n \
		add w0, [%0], [%0++]\n \
		6: ; Loop over\n \
		cp %0, w2\n \
		bra nz, 4b\n \
		6: ; Write back counters\n \
		mov w8, _oscs+16\n \
		mov w9, _oscs+18\n \
		mov w10, _oscs+24\n \
		mov w11, _oscs+26\n \
	"
	: "+r" (buffer)
	: "r" (cutoff)
	: "w0", "w1", "w2", "w3", "w4", "w5", "w6", "w7", "w8", "w9", "w10", "w11", "memory");
}

inline void audioRenderPara(int16_t * buffer, uint16_t cutoff)
{
	__asm volatile (" \
	; Process Voice 1 (OSC1 & OSC2) \n \
		1:\n \
		mov _voicesInc, w0,\n \
		mov _voicesInc+2, w1\n \
		2: ;Compute OSCs increments\n \
		mov _oscs+4, w2\n \
		mul.us w0, w2, w4\n \
		mul.us w1, w2, w6\n \
		add w5, w6, w5\n \
		mov _oscs+12, w2\n \
		mul.us w0, w2, w6\n \
		mul.us w1, w2, w8\n \
		add w7, w8, w7\n \
		3: ;Load OSCs counters\n \
		mov _oscs+0, w8\n \
		mov _oscs+2, w9\n \
		mov _oscs+8, w10\n \
		mov _oscs+10, w11\n \
		mov %0, w2\n \
		add #256, w2\n \
		4: ;Update OSCs counters\n \
		add w8, w4, w8\n \
		addc w9, w5, w9\n \
		add w10, w6, w10\n \
		addc w11, w7, w11\n \
		5: ;Compute OSCs waveforms\n \
		mov _oscs+6, w0\n \
		mov _oscs+14, w1\n \
		asr w9, w0, w0\n \
		asr w11, w1, w1\n \
		inc2 %0, %0\n \
		add w0, w1, [%0++]\n \
		6: ; Loop over\n \
		cp %0, w2\n \
		bra nz, 4b\n \
		6: ; Write back counters\n \n \
		sub #256, %0\n \
		mov w8, _oscs+0\n \
		mov w9, _oscs+2\n \
		mov w10, _oscs+8\n \
		mov w11, _oscs+10\n \
	; Process Voice 2 (OSC3 & OSC4) \n \
		1:\n \
		mov _voicesInc+4, w0,\n \
		mov _voicesInc+6, w1\n \
		2: ;Compute OSCs increments\n \
		mov _oscs+20, w2\n \
		mul.us w0, w2, w4\n \
		mul.us w1, w2, w6\n \
		add w5, w6, w5\n \
		mov _oscs+28, w2\n \
		mul.us w0, w2, w6\n \
		mul.us w1, w2, w8\n \
		add w7, w8, w7\n \
		3: ;Load OSCs counters\n \
		mov _oscs+16, w8\n \
		mov _oscs+18, w9\n \
		mov _oscs+24, w10\n \
		mov _oscs+26, w11\n \
		mov %0, w2\n \
		add #256, w2\n \
		4: ;Update OSCs counters\n \
		add w8, w4, w8\n \
		addc w9, w5, w9\n \
		add w10, w6, w10\n \
		addc w11, w7, w11\n \
		5: ;Compute OSCs waveforms\n \
		mov _oscs+22, w0\n \
		mov _oscs+30, w1\n \
		asr w9, w0, w0\n \
		asr w11, w1, w1\n \
		inc2 %0, %0\n \
		add w0, w1, w0\n \
		add w0, [%0], [%0++]\n \
		6: ; Loop over\n \
		cp %0, w2\n \
		bra nz, 4b\n \
		6: ; Write back counters\n \
		sub #256, %0\n \
		mov w8, _oscs+16\n \
		mov w9, _oscs+18\n \
		mov w10, _oscs+24\n \
		mov w11, _oscs+26\n \
	; Process Voice 3 (OSC5 & OSC6) \n \
		1:\n \
		mov _voicesInc+8, w0,\n \
		mov _voicesInc+10, w1\n \
		2: ;Compute OSCs increments\n \
		mov _oscs+36, w2\n \
		mul.us w0, w2, w4\n \
		mul.us w1, w2, w6\n \
		add w5, w6, w5\n \
		mov _oscs+44, w2\n \
		mul.us w0, w2, w6\n \
		mul.us w1, w2, w8\n \
		add w7, w8, w7\n \
		3: ;Load OSCs counters\n \
		mov _oscs+32, w8\n \
		mov _oscs+34, w9\n \
		mov _oscs+40, w10\n \
		mov _oscs+42, w11\n \
		mov %0, w2\n \
		add #256, w2\n \
		4: ;Update OSCs counters\n \
		add w8, w4, w8\n \
		addc w9, w5, w9\n \
		add w10, w6, w10\n \
		addc w11, w7, w11\n \
		5: ;Compute OSCs waveforms\n \
		mov _oscs+38, w0\n \
		mov _oscs+46, w1\n \
		asr w9, w0, w0\n \
		asr w11, w1, w1\n \
		inc2 %0, %0\n \
		add w0, w1, w0\n \
		add w0, [%0], [%0++]\n \
		6: ; Loop over\n \
		cp %0, w2\n \
		bra nz, 4b\n \
		6: ; Write back counters\n \
		sub #256, %0\n \
		mov w8, _oscs+32\n \
		mov w9, _oscs+34\n \
		mov w10, _oscs+40\n \
		mov w11, _oscs+42\n \
	; Process Voice 4 (OSC7 & OSC8) \n \
		1:\n \
		mov _voicesInc+12, w0,\n \
		mov _voicesInc+14, w1\n \
		2: ;Compute OSCs increments\n \
		mov _oscs+52, w2\n \
		mul.us w0, w2, w4\n \
		mul.us w1, w2, w6\n \
		add w5, w6, w5\n \
		mov _oscs+60, w2\n \
		mul.us w0, w2, w6\n \
		mul.us w1, w2, w8\n \
		add w7, w8, w7\n \
		3: ;Load OSCs counters\n \
		mov _oscs+48, w8\n \
		mov _oscs+50, w9\n \
		mov _oscs+56, w10\n \
		mov _oscs+58, w11\n \
		mov %0, w2\n \
		add #256, w2\n \
		4: ;Update OSCs counters\n \
		add w8, w4, w8\n \
		addc w9, w5, w9\n \
		add w10, w6, w10\n \
		addc w11, w7, w11\n \
		5: ;Compute OSCs waveforms\n \
		mov _oscs+54, w0\n \
		mov _oscs+62, w1\n \
		asr w9, w0, w0\n \
		asr w11, w1, w1\n \
		mov %1, [%0++]\n \
		add w0, w1, w0\n \
		add w0, [%0], [%0++]\n \
		6: ; Loop over\n \
		cp %0, w2\n \
		bra nz, 4b\n \
		6: ; Write back counters\n \n \
		mov w8, _oscs+48\n \
		mov w9, _oscs+50\n \
		mov w10, _oscs+56\n \
		mov w11, _oscs+58\n \
	"
	: "+r" (buffer)
	: "r" (cutoff)
	: "w0", "w1", "w2", "w3", "w4", "w5", "w6", "w7", "w8", "w9", "w10", "w11", "memory");
}

/******************************************************************************/
/*
 * C versions of algorithms (more or less)
 * For documentation purpose only
 */
/******************************************************************************/

/*
void audioRenderMono(int16_t * buffer)
{
	uint32_t inc = voicesInc[0];
	uint32_t register incs[4];
	incs[0] = inc * (uint32_t) oscs[0].scale;
	incs[1] = inc * (uint32_t) oscs[1].scale;
	incs[2] = inc * (uint32_t) oscs[2].scale;
	incs[3] = inc * (uint32_t) oscs[3].scale;

	for (int i = 0; i < AUDIO_BUFFER_LEN / 2; i++) {
		oscs[0].count += incs[0];
		oscs[1].count += incs[1];
		oscs[2].count += incs[2];
		oscs[3].count += incs[3];

		int16_t v = 0;
		int16_t a = oscs[0].count >> 16;
		v += a >> oscs[0].shift;
		int16_t b = oscs[1].count >> 16;
		v += b >> oscs[1].shift;
		int16_t c = oscs[2].count >> 16;
		v += c >> oscs[2].shift;
		int16_t d = oscs[3].count >> 16;
		v += d >> oscs[3].shift;
		v -= v >> 1;

		*buffer++ = 0;
		*buffer++ = v;
	}
}

void audioRenderPara(int16_t * buffer)
{
	uint32_t inc;
	uint32_t register incs[8];
	inc = voicesInc[0];
	incs[0] = inc * (int32_t) oscs[0].rate;
	incs[1] = inc * (int32_t) oscs[1].rate;
	inc = voicesInc[1];
	incs[2] = inc * (int32_t) oscs[2].rate;
	incs[3] = inc * (int32_t) oscs[3].rate;
	inc = voicesInc[2];
	incs[4] = inc * (int32_t) oscs[4].rate;
	incs[5] = inc * (int32_t) oscs[5].rate;
	inc = voicesInc[3];
	incs[6] = inc * (int32_t) oscs[6].rate;
	incs[7] = inc * (int32_t) oscs[7].rate;

	for (int i = 0; i < AUDIO_BUFFER_LEN / 2; i++) {
		oscs[0].count += incs[0];
		oscs[1].count += incs[1];
		oscs[2].count += incs[2];
		oscs[3].count += incs[3];
		oscs[4].count += incs[4];
		oscs[5].count += incs[5];
		oscs[6].count += incs[6];
		oscs[7].count += incs[7];

		int16_t v = 0;
		int16_t a = oscs[0].phase >> 16;
		v += a >> oscs[0].shift;
		int16_t b = oscs[1].phase >> 16;
		v += b >> oscs[1].shift;
		int16_t c = oscs[2].phase >> 16;
		v += c >> oscs[2].shift;
		int16_t d = oscs[3].phase >> 16;
		v += d >> oscs[3].shift;
		int16_t e = oscs[4].phase >> 16;
		v += e >> oscs[4].shift;
		int16_t f = oscs[5].phase >> 16;
		v += f >> oscs[5].shift;
		int16_t g = oscs[6].phase >> 16;
		v += g >> oscs[6].shift;
		int16_t h = oscs[7].phase >> 16;
		v += h >> oscs[7].shift;

		*buffer++ = 0;
		*buffer++ = v;
	}
}
*/
