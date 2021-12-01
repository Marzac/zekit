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

#ifndef UI_H
#define UI_H

	#include <stdint.h>

	typedef enum{
		TACT_PATTERN	= 0x0001,
		TACT_SYSTEM		= 0x0002,
		TACT_WAVE		= 0x0004,
		TACT_PLAY		= 0x0008,
		TACT_REC		= 0x0010,
		TACT_TAP		= 0x0020,
		TACT_SAVE		= 0x0040,
	}TACT_FLAGS;

	typedef enum{
		SYSTEM_ENVF_RETRIG	= 0x0001,
		SYSTEM_ENVF_LOOP	= 0x0002,
		SYSTEM_ENVF_TRACK	= 0x0004,
		SYSTEM_ENVF_GLIDE	= 0x0008,
	}SYSTEM_FLAGS;

	extern uint16_t uiSystem;
	extern uint16_t uiSwitches;
	extern uint16_t uiSwitchesLast;

	extern uint16_t uiSwitchPortA;
	extern uint16_t uiSwitchPortB;

/******************************************************************************/
	void uiInit();
	void uiUpdate();

	void uiFRCTuning(int value);

#endif
