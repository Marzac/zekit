/**
	PIC24 / dsPIC33 / PIC32MM MCUs Source File
	Microchip Technology Inc.
	(c) 2020 Microchip Technology Inc. and its subsidiaries.
	You may use this software and any derivatives exclusively with Microchip products.

	THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
	EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
	WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
	PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
	WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

	IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
	INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
	WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
	BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
	FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
	ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
	THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

	MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
	TERMS.
*/
/******************************************************************************/

// FSEC
#pragma config BWRP = OFF			// Boot Segment Write-Protect bit->Boot Segment may be written
#pragma config BSS = DISABLED		// Boot Segment Code-Protect Level bits->No Protection (other than BWRP)
#pragma config BSEN = OFF			// Boot Segment Control bit->No Boot Segment
#pragma config GWRP = OFF			// General Segment Write-Protect bit->General Segment may be written
#pragma config GSS = DISABLED		// General Segment Code-Protect Level bits->No Protection (other than GWRP)
#pragma config CWRP = OFF			// Configuration Segment Write-Protect bit->Configuration Segment may be written
#pragma config CSS = DISABLED		// Configuration Segment Code-Protect Level bits->No Protection (other than CWRP)
#pragma config AIVTDIS = OFF		// Alternate Interrupt Vector Table bit->Disabled AIVT

// FBSLIM
#pragma config BSLIM = 8191			// Boot Segment Flash Page Address Limit bits->8191

// FOSCSEL
#pragma config FNOSC = FRC			// Oscillator Source Selection->FRC
#pragma config PLLMODE = PLL96DIV2	// PLL Mode Selection->96 MHz PLL. Oscillator input is divided by 2 (8 MHz input)
#pragma config IESO = OFF			// Two-speed Oscillator Start-up Enable bit->Start up with user-selected oscillator source

// FOSC
#pragma config POSCMD = NONE		// Primary Oscillator Mode Select bits->Primary Oscillator disabled
#pragma config OSCIOFCN = ON		// OSC2 Pin Function bit->OSC2 is general purpose digital I/O pin
#pragma config SOSCSEL = OFF		// SOSC Power Selection Configuration bits->Digital (SCLKI) mode
#pragma config PLLSS = PLL_FRC		// PLL Secondary Selection Configuration bit->PLL is fed by the on-chip Fast RC (FRC) oscillator
#pragma config IOL1WAY = ON			// Peripheral pin select configuration bit->Allow only one reconfiguration
#pragma config FCKSM = CSECMD		// Clock Switching Mode bits->Clock switching is enabled,Fail-safe Clock Monitor is disabled

// FWDT
#pragma config WDTPS = PS32768		// Watchdog Timer Postscaler bits->1:32768
#pragma config FWPSA = PR128		// Watchdog Timer Prescaler bit->1:128
#pragma config FWDTEN = OFF			// Watchdog Timer Enable bits->WDT and SWDTEN disabled
#pragma config WINDIS = OFF			// Watchdog Timer Window Enable bit->Watchdog Timer in Non-Window mode
#pragma config WDTWIN = WIN25		// Watchdog Timer Window Select bits->WDT Window is 25% of WDT period
#pragma config WDTCMX = WDTCLK		// WDT MUX Source Select bits->WDT clock source is determined by the WDTCLK Configuration bits
#pragma config WDTCLK = LPRC		// WDT Clock Source Select bits->WDT uses LPRC

// FPOR
#pragma config BOREN = ON			// Brown Out Enable bit->Brown Out Enable Bit
#pragma config LPCFG = OFF			// Low power regulator control->No Retention Sleep
#pragma config DNVPEN = ENABLE		// Downside Voltage Protection Enable bit->Downside protection enabled using ZPBOR when BOR is inactive

// FICD
#pragma config ICS = PGD2			// ICD Communication Channel Select bits->Communicate on PGEC2 and PGED2
#pragma config JTAGEN = OFF			// JTAG Enable bit->JTAG is disabled

// FDEVOPT1
#pragma config ALTCMPI = DISABLE	// Alternate Comparator Input Enable bit->C1INC, C2INC, and C3INC are on their standard pin locations
#pragma config TMPRPIN = OFF		// Tamper Pin Enable bit->TMPRN pin function is disabled
#pragma config SOSCHP = ON			// SOSC High Power Enable bit (valid only when SOSCSEL = 1->Enable SOSC high power mode (default)
#pragma config ALTI2C1 = ALTI2CEN	// Alternate I2C pin Location->SDA1 and SCL1 on RB9 and RB8
