/**
 * ZeKit Firmware
 * (c) Fred's Lab 2020-2021
 * fred@fredslab.net
 * 
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2021 Frédéric Meslin
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "setup.h"
#include "midi.h"
#include "audio.h"
#include "pins.h"

#include <stdint.h>
#include <stdlib.h>
#include <xc.h>

/******************************************************************************/
void setupClock();
void setupGPIOs();
void setupSPI();
void setupUART();
void setupTicks();
void setupComp();
void setupChargePump();

/******************************************************************************/
// FSEC
#pragma config BWRP = OFF		//Boot Segment Write-Protect bit->Boot Segment may be written
#pragma config BSS = DISABLED	//Boot Segment Code-Protect Level bits->No Protection (other than BWRP)
#pragma config BSEN = OFF		//Boot Segment Control bit->No Boot Segment
#pragma config GWRP = OFF		//General Segment Write-Protect bit->General Segment may be written
#pragma config GSS = DISABLED	//General Segment Code-Protect Level bits->No Protection (other than GWRP)
#pragma config CWRP = OFF		//Configuration Segment Write-Protect bit->Configuration Segment may be written
#pragma config CSS = DISABLED	//Configuration Segment Code-Protect Level bits->No Protection (other than CWRP)
#pragma config AIVTDIS = OFF	//Alternate Interrupt Vector Table bit->Disabled AIVT

// FBSLIM
#pragma config BSLIM = 8191		//Boot Segment Flash Page Address Limit bits->8191

// FOSCSEL
#pragma config FNOSC = FRC		//Oscillator Source Selection->FRC
#pragma config PLLMODE = PLL96DIV2	//PLL Mode Selection->96 MHz PLL. Oscillator input is divided by 2 (8 MHz input)
#pragma config IESO = OFF		//Two-speed Oscillator Start-up Enable bit->Start up with user-selected oscillator source

// FOSC
#pragma config POSCMD = NONE	//Primary Oscillator Mode Select bits->Primary Oscillator disabled
#pragma config OSCIOFCN = ON	//OSC2 Pin Function bit->OSC2 is general purpose digital I/O pin
#pragma config SOSCSEL = OFF	//SOSC Power Selection Configuration bits->Digital (SCLKI) mode
#pragma config PLLSS = PLL_FRC	//PLL Secondary Selection Configuration bit->PLL is fed by the on-chip Fast RC (FRC) oscillator
#pragma config IOL1WAY = ON		//Peripheral pin select configuration bit->Allow only one reconfiguration
#pragma config FCKSM = CSECMD	//Clock Switching Mode bits->Clock switching is enabled,Fail-safe Clock Monitor is disabled

// FWDT
#pragma config WDTPS = PS32768	//Watchdog Timer Postscaler bits->1:32768
#pragma config FWPSA = PR128	//Watchdog Timer Prescaler bit->1:128
#pragma config FWDTEN = OFF		//Watchdog Timer Enable bits->WDT and SWDTEN disabled
#pragma config WINDIS = OFF		//Watchdog Timer Window Enable bit->Watchdog Timer in Non-Window mode
#pragma config WDTWIN = WIN25	//Watchdog Timer Window Select bits->WDT Window is 25% of WDT period
#pragma config WDTCMX = WDTCLK	//WDT MUX Source Select bits->WDT clock source is determined by the WDTCLK Configuration bits
#pragma config WDTCLK = LPRC	//WDT Clock Source Select bits->WDT uses LPRC

// FPOR
#pragma config BOREN = ON		//Brown Out Enable bit->Brown Out Enable Bit
#pragma config LPCFG = OFF		//Low power regulator control->No Retention Sleep
#pragma config DNVPEN = ENABLE	//Downside Voltage Protection Enable bit->Downside protection enabled using ZPBOR when BOR is inactive

// FICD
#pragma config ICS = PGD1		//ICD Communication Channel Select bits->Communicate on PGEC1 and PGED1
#pragma config JTAGEN = OFF		//JTAG Enable bit->JTAG is disabled

// FDEVOPT1
#pragma config ALTCMPI = DISABLE	//Alternate Comparator Input Enable bit->C1INC, C2INC, and C3INC are on their standard pin locations
#pragma config TMPRPIN = OFF		//Tamper Pin Enable bit->TMPRN pin function is disabled
#pragma config SOSCHP = ON			//SOSC High Power Enable bit (valid only when SOSCSEL = 1->Enable SOSC high power mode (default)
#pragma config ALTI2C1 = ALTI2CEN	//Alternate I2C pin Location->SDA1 and SCL1 on RB9 and RB8
	
/******************************************************************************/
void setupInit()
{
	setupClock();
	setupGPIOs();
	setupChargePump();
	
	DMACON = 0;
	DMACONbits.DMAEN = 1;
	DMACONbits.PRSSEL = 1;
	DMAH = 0xFFFF;
	DMAL = 0x0000;
	
	setupTicks();
	setupSPI();
	setupUART();
	setupComp();
}

/******************************************************************************/
void setupClock()
{
	// CPDIV 1:1; PLLEN disabled; DOZE 1:8; RCDIV FRC; DOZEN disabled; ROI disabled; 
	CLKDIV = 0x3000;
	// STOR disabled; STORPOL Interrupt when STOR is 1; STSIDL disabled; STLPOL Interrupt when STLOCK is 1; STLOCK disabled; STSRC SOSC; STEN disabled; TUN Center frequency; 
	OSCTUN = 0x00;
	// ROEN disabled; ROSWEN disabled; ROSEL FOSC; ROOUT disabled; ROSIDL disabled; ROSLP disabled; 
	REFOCONL = 0x00;
	// RODIV 0; 
	REFOCONH = 0x00;
	// DCOTUN 0; 
	DCOTUN = 0x00;
	// DCOFSEL 8; DCOEN disabled; 
	DCOCON = 0x700;
	// DIV 0; 
	OSCDIV = 0x00;
	// TRIM 0; 
	OSCFDIV = 0x00;
	// AD1MD enabled; T3MD enabled; T1MD enabled; U2MD enabled; T2MD enabled; U1MD enabled; SPI2MD enabled; SPI1MD enabled; I2C1MD enabled; 
	PMD1 = 0x00;
	// IC3MD enabled; OC1MD enabled; IC2MD enabled; OC2MD enabled; IC1MD enabled; OC3MD enabled; 
	PMD2 = 0x00;
	// PMPMD enabled; RTCCMD enabled; CMPMD enabled; CRCMD enabled; I2C2MD enabled; 
	PMD3 = 0x00;
	// CTMUMD enabled; REFOMD enabled; LVDMD enabled; 
	PMD4 = 0x00;
	// CCP2MD enabled; CCP1MD enabled; CCP4MD enabled; CCP3MD enabled; CCP5MD enabled; 
	PMD5 = 0x00;
	// SPI3MD enabled; 
	PMD6 = 0x00;
	// DMA1MD enabled; DMA0MD enabled; 
	PMD7 = 0x00;
	// CLC1MD enabled; CLC2MD enabled; 
	PMD8 = 0x00;
	// CF no clock failure; NOSC FRCPLL; SOSCEN disabled; POSCEN disabled; CLKLOCK unlocked; OSWEN Switch is Complete; IOLOCK not-active; 
	__builtin_write_OSCCONH((uint8_t) (0x01));
	__builtin_write_OSCCONL((uint8_t) (0x01));
	
	// Wait for Clock switch to occur
	while (OSCCONbits.OSWEN != 0);
	while (OSCCONbits.LOCK != 1);
}

void setupGPIOs()
{
	LATA = 0x0000;
	LATB = 0x0000;
	TRISA = 0x0013;
	TRISB = 0x101F;
	IOCPDA = 0x0000;
	IOCPDB = 0x0000;
	IOCPUA = 0x0000;
	IOCPUB = 0x0000;
	ODCA = 0x0000;
	ODCB = 0x0000;
	ANSA = 0x0003;
	ANSB = 0x100C;
	
	__builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS

	RPOR7bits.RP15R = 0x0007;	//RB15->SPI1:SDO1
	RPOR7bits.RP14R = 0x0009;	//RB14->SPI1:CS
	RPINR18bits.U1RXR = 0x0004;	//RB4->UART1:U1RX
	RPOR6bits.RP13R = 0x0008;	//RB13->SPI1:SCK1OUT
	RPOR2bits.RP5R = 0x000D;	//RB5->OC1:OC1
	
	__builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
}

void setupTicks()
{
	T1CON = 0;
	TMR1 = 0;
	PR1 = (FCY_FRQ / (TICK_FRQ * 64)) - 1;
	
	T1CONbits.TCKPS = 2; 
	T1CONbits.TON = 1;
	
	IPC0bits.T1IP = 3;
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1; 
}

void setupSPI()
{
// Configure the SPI
	SPI1CON1L = 0;
	SPI1CON1H = 0;
	SPI1CON2L = 0;
	SPI1STATL = 0;
	SPI1STATH = 0;
	SPI1IMSKL = 0;
	
	SPI1CON1Hbits.AUDEN = 1;	// Audio mode
	SPI1CON1Hbits.AUDMOD = 2;	// Right-justified
	SPI1CON1Lbits.MSTEN = 1;
	SPI1CON1Lbits.CKE = 1;
	SPI1CON1Lbits.CKP = 0;
	
	SPI1BRGL = (FCY_FRQ / (2 * SAMPLE_FRQ * 32)) - 1;

	SPI1IMSKLbits.SPITBEN = 1;
	SPI1CON1Lbits.ENHBUF = 1;
	SPI1CON1Lbits.SPIEN = 1; 

// Configure the DMA	
	DMACH0 = 0;
	DMAINT0 = 0;

	DMASRC0 = (uint16_t) audioBuffer;
	DMADST0 = (uint16_t) &SPI1BUFL;
	DMACNT0 = BUFFER_LEN * 2;

	DMAINT0bits.HALFEN = 1;		// Half transfer
	DMAINT0bits.CHSEL = 0x27;	// SPI1 TX
	
	DMACH0bits.RELOAD = 1;
	DMACH0bits.SAMODE = 1;		// Increment
	DMACH0bits.DAMODE = 0;		// Do not increment
	DMACH0bits.TRMODE = 1;		// Repeated one-shot
	DMACH0bits.CHEN = 1;

	IPC1bits.DMA0IP = 4;
	IEC0bits.DMA0IE = 1;
	
// Start the transfer
	SPI1BUFL = 0;
	SPI1BUFL = 0;
}

void setupUART()
{
// Configure the UART
	U1MODE = 0;
	U1MODEbits.BRGH = 1;
    U1MODEbits.UARTEN = 1;

    U1BRG = (FCY_FRQ / 31250 / 4) - 1;
    U1ADMD = 0x00;

    U1STA = 0x00;
	U1STAbits.URXEN = 1;
    
// Configure the DMA	
	DMACH1 = 0;
	DMAINT1 = 0;
	DMASRC1 = (uint16_t) &U1RXREG;
	DMADST1 = (uint16_t) midiBuffer;
	DMACNT1 = 64;

	DMAINT1bits.CHSEL = 0x45;	// UART1 RX
	
	DMACH1bits.RELOAD = 1;
	DMACH1bits.SAMODE = 0;		// Do not increment
	DMACH1bits.DAMODE = 1;		// Increment
	DMACH1bits.TRMODE = 1;		// Repeated one-shot
	DMACH1bits.CHEN = 1;
		
}

void setupComp()
{
	CVRCON = 0;
	CVRCONbits.CVREN = 1;
	CVRCONbits.CVR = 24;		// = 2.475V
	
	CM1CON = 0;
	CM1CONbits.CON = 1;			// Enable
	CM1CONbits.CPOL = 1;		// Inverse polarity
	CM1CONbits.EVPOL = 1;		// Trigger on transition
	CM1CONbits.CREF = 1;		// Use reference voltage
	CM1CONbits.CCH = 0;			// C1INB pin
	
	IEC1bits.CMIE = 1;			// Interrupt
}

void setupChargePump()
{
	const int dt = FCY_FRQ / PUMP_FRQ;
	OC1R = dt / 2 - 1;
	OC1RS = dt - 1;
	OC1CON2 = 0x001F;
	OC1CON1 = 0x1C06;			//Edge-aligned PWM
}
