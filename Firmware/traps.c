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

void __attribute__((interrupt,no_auto_psv)) _OscillatorFail(void)
{
	INTCON1bits.OSCFAIL = 0;		/* Clear the trap flag */
	while(1);
}

void __attribute__((interrupt,no_auto_psv)) _AddressError(void)
{
	INTCON1bits.ADDRERR = 0;		/* Clear the trap flag */
	while (1);
}
void __attribute__((interrupt,no_auto_psv)) _StackError(void)
{
	INTCON1bits.STKERR = 0;			/* Clear the trap flag */
	while (1);
}

void __attribute__((interrupt,no_auto_psv)) _MathError(void)
{
	INTCON1bits.MATHERR = 0;		/* Clear the trap flag */
	while (1);
}

void __attribute__((interrupt,no_auto_psv)) _AltOscillatorFail(void)
{
	INTCON1bits.OSCFAIL = 0;		/* Clear the trap flag */
	while (1);
}

void __attribute__((interrupt,no_auto_psv)) _AltAddressError(void)
{
	INTCON1bits.ADDRERR = 0;		/* Clear the trap flag */
	while (1);
}

void __attribute__((interrupt,no_auto_psv)) _AltStackError(void)
{
	INTCON1bits.STKERR = 0;			/* Clear the trap flag */
	while (1);
}

void __attribute__((interrupt,no_auto_psv)) _AltMathError(void)
{
	INTCON1bits.MATHERR = 0;		/* Clear the trap flag */
	while (1);
}

void __attribute__((interrupt,no_auto_psv)) _DefaultInterrupt(void)
{
	while(1);
}
