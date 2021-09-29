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

#include "waves.h"

#include <stdint.h>

/******************************************************************************/
const Sawer wavesMono[MAX_WAVES][4] = {
{
// Basic sawtooth
	{.phase = 0x00000000, .rate = 128, .shift = 1},
	{.phase = 0x00000000, .rate = 0,   .shift = 16},
	{.phase = 0x00000000, .rate = 0,   .shift = 16},
	{.phase = 0x00000000, .rate = 0,   .shift = 16}},

// Detuned sawtooth
{
	{.phase = 0x00000000, .rate = 127, .shift = 2},
	{.phase = 0x00000000, .rate = 129, .shift = 2},
	{.phase = 0x00000000, .rate = 128, .shift = 2},
	{.phase = 0x00000000, .rate = 0,   .shift = 16}},

// Basic square
{
	{.phase = 0x00000000, .rate = 128, .shift = 2},
	{.phase = 0x80000000, .rate =-128, .shift = 2},
	{.phase = 0x00000000, .rate =	0, .shift = 16},
	{.phase = 0x00000000, .rate =	0, .shift = 16}},

// Detuned square
{
	{.phase = 0x00000000, .rate = 127, .shift = 2},
	{.phase = 0x80000000, .rate =-127, .shift = 2},
	{.phase = 0x00000000, .rate = 129, .shift = 2},
	{.phase = 0x80000000, .rate =-129, .shift = 2}},

// Pulse 25%
{
	{.phase = 0x00000000, .rate = 128, .shift = 1},
	{.phase = 0x40000000, .rate =-128, .shift = 1},
	{.phase = 0x00000000, .rate =	0, .shift = 16},
	{.phase = 0x00000000, .rate =	0, .shift = 16}},

// Double-sawtooth
{
	{.phase = 0x00000000, .rate = 128, .shift = 1},
	{.phase = 0x00000000, .rate = 256, .shift = 2},
	{.phase = 0x00000000, .rate =	0, .shift = 16},
	{.phase = 0x00000000, .rate =	0, .shift = 16}},

// Pulse 10%
{
	{.phase = 0x00000000, .rate = 128, .shift = 1},
	{.phase = 0x18000000, .rate =-128, .shift = 1},
	{.phase = 0x00000000, .rate =	0, .shift = 16},
	{.phase = 0x00000000, .rate =	0, .shift = 16}},

// Ultrasaw
{
	{.phase = 0x00000000, .rate = 127, .shift = 2},
	{.phase = 0x00000000, .rate = 130, .shift = 2},
	{.phase = 0x00000000, .rate = 128, .shift = 2},
	{.phase = 0x80000000, .rate = 256, .shift = 2}},
};

/******************************************************************************/
const Sawer wavesPara[MAX_WAVES][2] = {
{
// Basic sawtooth
	{.phase = 0x00000000, .rate = 128, .shift = 3},
	{.phase = 0x00000000, .rate =	0, .shift = 16}},

// Detuned sawtooth 1
{
	{.phase = 0x00000000, .rate = 255, .shift = 3},
	{.phase = 0x80000000, .rate = 257, .shift = 3}},

// Basic square
{
	{.phase = 0x00000000, .rate = 128, .shift = 4},
	{.phase = 0x80000000, .rate =-128, .shift = 4}},

// Double sawtooth
{
	{.phase = 0x00000000, .rate = 128, .shift = 3},
	{.phase = 0x80000000, .rate = 64,  .shift = 4}},

// Pulse 25%
{
	{.phase = 0x00000000, .rate = 128, .shift = 3},
	{.phase = 0x40000000, .rate =-128, .shift = 3}},

// Detuned sawtooth 2
{
	{.phase = 0x00000000, .rate = 256, .shift = 3},
	{.phase = 0x80000000, .rate = 127, .shift = 4}},

// Pulse 10%
{
	{.phase = 0x00000000, .rate = 128, .shift = 3},
	{.phase = 0x18000000, .rate =-128, .shift = 3}},

// Detuned sawtooth 2
{
	{.phase = 0x00000000, .rate = 129, .shift = 3},
	{.phase = 0x80000000, .rate = 127, .shift = 3}},
};
