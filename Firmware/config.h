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

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/** Frequencies in Hertz */
#define FRQ_FCY				16000000UL
#define FRQ_TICK			1000UL
#define FRQ_PUMP			150000UL
#define FRQ_SAMPLE			250000UL
#define FRQ_MIDI			31250UL

/** Buffer lengths */
#define AUDIO_BUFFER_LEN	128
#define MIDIRX_BUFFER_LEN	64
#define MIDIRX_BUFFER_MASK	(MIDIRX_BUFFER_LEN - 1)

/** Other constants */
#define NOTE_BASE			48
#define TRACK_REF			60

/** System ticks (milliseconds) */
extern uint16_t uwTick;

#endif
