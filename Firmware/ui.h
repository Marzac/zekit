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

#ifndef UI_H
#define UI_H

	#include <stdint.h>

	typedef enum{
		TACT_PATTERN	= 0x01,
		TACT_SYSTEM		= 0x02,
		TACT_WAVE		= 0x04,
		TACT_PLAY		= 0x08,
		TACT_REC		= 0x10,
		TACT_TAP		= 0x20,
		TACT_SAVE		= 0x40,
	}TACT_FLAGS;

	typedef enum{
		SYSTEM_ENV_RETRIG	= 0x01,
		SYSTEM_ENV_LOOP		= 0x02,
		SYSTEM_FILTER_TRACK	= 0x04,
		SYSTEM_PITCH_GLIDE	= 0x08,
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
