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

#ifndef AUDIO_H
#define AUDIO_H

	#include "pins.h"
	#include "config.h"

	#include <stdint.h>

/******************************************************************************/
	#define MAX_VOICES		4
	#define MAX_OSCS		(MAX_VOICES * 2)
	#define GLIDE_SHIFT		2
	#define BEND_RANGE		7

/******************************************************************************/
	extern int16_t audioBuffer[AUDIO_BUFFER_LEN * 2];

/******************************************************************************/
	void audioInit();
	void audioUpdate();
	void audioRender(int16_t * buffer);

	void audioNoteOn(uint8_t note);
	void audioNoteOff(uint8_t note);
	void audioAllNotesOff();
	void audioAllSoundsOff();
	void audioResetCtrls();

	void audioSetWave(int wave);
	int	 audioGetWave();
	void audioSetBend(int16_t bend);
	void audioSetCutoff(int16_t cutoff);

#endif
