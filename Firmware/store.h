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

#ifndef STORE_H
#define STORE_H

	#include <stdint.h>

/******************************************************************************/
	#define FLASH_ROW_SIZE				(128u * 2)
	#define FLASH_ROWS_PER_PAGE			(8u)
	#define FLASH_PAGE_SIZE				(FLASH_ROWS_PER_PAGE * FLASH_ROW_SIZE)

	#define GLOBALS_ADDR				(0x7800u)
	#define GLOBAL_MIDICHANNEL_ADDR		(GLOBALS_ADDR + 0)
	#define GLOBAL_FRCTUNING_ADDR		(GLOBALS_ADDR + 4)
	#define PATTERNS_ADDR				(0x8000u)

	#define STORE_PATTERN_SIZE			(FLASH_ROW_SIZE * 2)
	#define STORE_PATTERNS_PER_PAGE		(FLASH_PAGE_SIZE / STORE_PATTERN_SIZE)
	#define STORE_PATTERNS_ADDR(p)		(PATTERNS_ADDR + (p) * FLASH_PAGE_SIZE)

/******************************************************************************/
	void storeErasePage(uint32_t addr);
	void storeRead32(uint32_t addr, uint32_t * dword);
	void storeWrite32(uint32_t addr, const uint32_t * dword);

#endif
