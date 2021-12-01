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

#ifndef WAVES_H
#define WAVES_H


#include <stdint.h>
/******************************************************************************/
	#define MAX_WAVES				8
	#define IS_WAVEFORM_MONO(w)		((w) < MAX_WAVES)
	#define IS_WAVEFORM_POLY(w)		((w) >= MAX_WAVES)

	typedef struct {
		int32_t phase;	// Start phase
		int16_t rate;	// Pitch rate (multiplier)
		int16_t shift;	// Level shift (volume)
	}Sawer;

	extern const Sawer wavesMono[MAX_WAVES][4];
	extern const Sawer wavesPara[MAX_WAVES][2];

#endif
