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

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "store.h"

/******************************************************************************/
/* Reserved flash memory */
const int8_t __attribute__ ((section(".globals"),  noload, address(GLOBALS_ADDR))) flashGlobals[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x0000))) flashPatternsPage1[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x0800))) flashPatternsPage2[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x1000))) flashPatternsPage3[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x1800))) flashPatternsPage4[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x2000))) flashPatternsPage5[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x2800))) flashPatternsPage6[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x3000))) flashPatternsPage7[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x3800))) flashPatternsPage8[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x4000))) flashPatternsPage9[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x4800))) flashPatternsPage10[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x5000))) flashPatternsPage11[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x5800))) flashPatternsPage12[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x6000))) flashPatternsPage13[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x6800))) flashPatternsPage14[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x7000))) flashPatternsPage15[FLASH_PAGE_SIZE];
const int8_t __attribute__ ((section(".patterns"), noload, address(PATTERNS_ADDR+0x7800))) flashPatternsPage16[FLASH_PAGE_SIZE];

/******************************************************************************/
void storeErasePage(uint32_t addr)
{
	NVMCON = 0x4003; // Erase full page
	TBLPAG = 0x00;
	NVMADRU = (uint16_t) (addr >> 16);
	NVMADR	= (uint16_t) (addr);

	__asm volatile("disi #5\n");
	__builtin_write_NVM();
	while(NVMCONbits.WR);
}

void storeRead32(uint32_t addr, uint32_t * dword)
{
	uint16_t addrHigh = (uint16_t) (addr >> 16);
	uint16_t addrLow  = (uint16_t) (addr);
	TBLPAG = addrHigh;

	uint16_t * data = (uint16_t *) dword;
	data[0] = __builtin_tblrdl(addrLow);
	data[1] = __builtin_tblrdl(addrLow + 2);
}

void storeWrite32(uint32_t addr, const uint32_t * dword)
{
	NVMCON = 0x4001; // Write double word
	TBLPAG = 0xFA;
	NVMADRU = (uint16_t) (addr >> 16);
	NVMADR	= (uint16_t) (addr);

	uint16_t * data = (uint16_t *) dword;
	__builtin_tblwtl(0, data[0]);
	__builtin_tblwth(0, 0xFF);
	__builtin_tblwtl(2, data[1]);
	__builtin_tblwth(2, 0xFF);

	__asm volatile("disi #5\n");
	__builtin_write_NVM();
	while(NVMCONbits.WR);
}
