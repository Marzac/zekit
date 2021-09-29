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

#include "setup.h"
#include "midi.h"
#include "mseq.h"
#include "audio.h"
#include "store.h"
#include "ui.h"

#include "pins.h"
#include "config.h"

#include <xc.h>
#include <stdint.h>

/******************************************************************************/
uint16_t uwTick = 0;

/******************************************************************************/
int main(void)
{
// Peripherals setup
	setup();

// Program initialisation
	midiInit();
	audioInit();
	mseqInit();
	uiInit();

// Program main loop
	while (1) {
		uiUpdate();
		midiUpdate();
		mseqUpdate();
		audioUpdate();
	}

	return 1;
}
