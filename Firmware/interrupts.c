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

#include <xc.h>
#include <stdint.h>

#include "audio.h"
#include "mseq.h"

#include "pins.h"
#include "config.h"

/******************************************************************************/
void __attribute__((interrupt,no_auto_psv)) _DMA0Interrupt(void)
{
	if (DMAINT0bits.HALFIF) {
		audioRender(&audioBuffer[0]);
		DMAINT0bits.HALFIF = 0;
		DMAINT0bits.DONEIF = 0;
	}

	if (DMAINT0bits.DONEIF) {
		audioRender(&audioBuffer[AUDIO_BUFFER_LEN]);
		DMAINT0bits.HALFIF = 0;
		DMAINT0bits.DONEIF = 0;
	}

	IFS0bits.DMA0IF = 0;
}

/******************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
	uwTick++;
	IFS0bits.T1IF = 0;
}

void __attribute__((interrupt, no_auto_psv)) _IOCInterrupt(void)
{
	if (IOCFAbits.IOCFA0) mseqExtClockStart();
	if (IOCFAbits.IOCFA1) mseqExtClockTick();
	IOCFA = 0;
	IFS1bits.IOCIF = 0;
}
