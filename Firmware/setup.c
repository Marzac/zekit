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
#include "audio.h"

#include "pins.h"
#include "config.h"

#include <stdint.h>
#include <stdlib.h>
#include <xc.h>

/******************************************************************************/
static void setupClock();
static void setupGPIOs();
static void setupDMA();
static void setupSPI();
static void setupUART();
static void setupTicks();
static void setupComp();
static void setupChargePump();

/******************************************************************************/
void setup()
{
// Complete setup
	setupClock();
	setupGPIOs();
	setupChargePump();
	setupDMA();
	setupTicks();
	setupSPI();
	setupUART();
	setupComp();
}

/******************************************************************************/
void setupClock()
{
// Internal clock settings
	CLKDIV = 0x3000;		// Default clock setting
	OSCTUN = 0x00;			// Default FRC trimming
	OSCDIV = 0x00;
	OSCFDIV = 0x00;
	REFOCONL = 0x00;
	REFOCONH = 0x00;
	DCOTUN = 0x00;
	DCOCON = 0x700;

// Peripherals power enable
	PMD1 = 0xFF;
	PMD2 = 0xFF;
	PMD3 = 0xFF;
	PMD4 = 0xFF;
	PMD5 = 0xFF;
	PMD6 = 0xFF;
	PMD7 = 0xFF;
	PMD8 = 0xFF;
	PMD1bits.T1MD = 0;		// Timer1 enabled;
	PMD1bits.U1MD = 0;		// UART1 enabled;
	PMD1bits.SPI1MD = 0;	// SPI1 enabled;
	PMD2bits.OC1MD = 0;		// OC1 enabled;
	PMD7bits.DMA0MD = 0;	// DMA0 enabled
	PMD7bits.DMA1MD = 0;	// DMA1 enabled

// Switch clock to FRCPLL
	__builtin_write_OSCCONH((uint8_t) (0x01));
	__builtin_write_OSCCONL((uint8_t) (0x01));

// Wait for clock switch
	while (OSCCONbits.OSWEN != 0);
	while (OSCCONbits.LOCK != 1);
}

void setupGPIOs()
{
// Ports pin direction and mode
	TRISA = 0xFFFF;
	TRISB = ~(PORTB_GATE_VCF | PORTB_GATE_VCA);
	ODCA = PORTA_TACT_PATTERN | PORTA_TACT_SYSTEM | PORTA_TACT_WAVE;
	ODCB = PORTB_TACT_PLAY | PORTB_TACT_REC | PORTB_TACT_TAP | PORTB_TACT_SAVE;
	LATA = 0x0000;
	LATB = 0x0000;
	ANSA = 0x0000;
	ANSB = 0x0000;

// Ports pin options (pull up & down, interrupts)
	IOCFA = 0;
	IOCFB = 0;
	IOCPUA = PORTA_CLOCK_START | PORTA_CLOCK_INPUT | PORTA_TACT_PATTERN | PORTA_TACT_SYSTEM | PORTA_TACT_WAVE;
	IOCPUB = PORTB_TACT_PLAY | PORTB_TACT_REC | PORTB_TACT_TAP | PORTB_TACT_SAVE;
	IOCPDA = 0;
	IOCPDB = 0;
	IOCNA = 0;
	IOCNB = 0;
	IOCPA = PORTA_CLOCK_START | PORTA_CLOCK_INPUT;
	IOCPB = 0;

	PADCONbits.IOCON = 1;
	IEC1bits.IOCIE = 1;

// Peripherals to pins mapping
	__builtin_write_OSCCONL(OSCCON & 0xbf); // Unlock PPS

	RPOR7bits.RP15R = 0x0008;		// RB15->SPI1:SCK1OUT
	RPOR7bits.RP14R = 0x0009;		// RB14->SPI1:CS
	RPOR6bits.RP13R = 0x0007;		// RB13->SPI1:SDO1
	RPOR2bits.RP5R	= 0x000D;		// RB5->OC1:OC1
	RPINR18bits.U1RXR = 0x0004;		// RB4->UART1:U1RX

	__builtin_write_OSCCONL(OSCCON | 0x40); // Lock back PPS
}

void setupTicks()
{
// Configure the tick timer
	T1CON = 0;
	TMR1 = 0;
	PR1 = (FRQ_FCY / (FRQ_TICK * 64)) - 1;

	T1CONbits.TCKPS = 2;
	T1CONbits.TON = 1;			// Start the timer

	IFS0bits.T1IF = 0;
	IPC0bits.T1IP = 3;			// Set interrupt priority
	IEC0bits.T1IE = 1;			// Enable the interrupt
}

void setupSPI()
{
// Configure the audio SPI
	SPI1CON1L = 0;
	SPI1CON1H = 0;
	SPI1CON2L = 0;
	SPI1STATL = 0;
	SPI1STATH = 0;
	SPI1IMSKL = 0;

	SPI1CON1Hbits.AUDEN = 1;	// Audio mode
	SPI1CON1Hbits.AUDMOD = 2;	// Right-justified data
	SPI1CON1Lbits.MSTEN = 1;	// SPI Master mode
	SPI1CON1Lbits.CKE = 1;
	SPI1CON1Lbits.CKP = 0;

	SPI1BRGL = (FRQ_FCY / (2 * FRQ_SAMPLE * 32)) - 1;

	SPI1IMSKLbits.SPITBEN = 1;	// Enable interrupt generation
	SPI1CON1Lbits.ENHBUF = 1;	//
	SPI1CON1Lbits.SPIEN = 1;	// Enable SPI

// Configure the audio DMA
	DMACH0 = 0;
	DMAINT0 = 0;
	DMASRC0 = (uint16_t) audioBuffer;
	DMADST0 = (uint16_t) &SPI1BUFL;
	DMACNT0 = AUDIO_BUFFER_LEN * 2;

	DMAINT0bits.HALFEN = 1;		// Half transfer interrupts
	DMAINT0bits.CHSEL = 0x27;	// SPI1 TX request

	DMACH0bits.RELOAD = 1;		// Circular mode
	DMACH0bits.SAMODE = 1;		// Source address => increment
	DMACH0bits.DAMODE = 0;		// Destination address => not increment
	DMACH0bits.TRMODE = 1;		// Repeated one-shot mode
	DMACH0bits.CHEN = 1;		// Enable DMA channel

	IPC1bits.DMA0IP = 4;		// Set interrupt priority
	IEC0bits.DMA0IE = 1;		// Enable DMA interrupt

// Start the continuous transfer
	SPI1BUFL = 0;
	SPI1BUFL = 0;
}

void setupUART()
{
// Configure the MIDI UART
	U1MODE = 0;					// Reset the UART mode
	U1MODEbits.BRGH = 1;		// Fast baudrate mode
	U1MODEbits.UARTEN = 1;		// Enable the UART
	U1BRG = (FRQ_FCY / FRQ_MIDI / 4) - 1;
	U1ADMD = 0x00;
	U1STA = 0x00;				// Clear the status
	U1STAbits.URXEN = 1;		// Enable reception

// Configure the DMA
	DMACH1 = 0;					// Reset the DMA channel
	DMAINT1 = 0;
	DMASRC1 = (uint16_t) &U1RXREG;
	DMADST1 = (uint16_t) midiBuffer;
	DMACNT1 = MIDIRX_BUFFER_LEN;

	DMAINT1bits.CHSEL = 0x45;	// UART1 RX
	DMACH1bits.RELOAD = 1;		// Circular mode
	DMACH1bits.SAMODE = 0;		// Source address => not increment
	DMACH1bits.DAMODE = 1;		// Destination address => Increment
	DMACH1bits.TRMODE = 1;		// Repeated one-shot mode
	DMACH1bits.CHEN = 1;		// Enable DMA channel
}

void setupComp()
{
// Configurate the analog comparator
	CVRCON = 0;					// Reset the voltage reference
	CVRCONbits.CVREN = 1;		// Enable voltage reference
	CVRCONbits.CVR = 24;		// Vref = 2.475V

// Comparator 1 - ENVF
	CM1CON = 0;					// Reset the comparator module
	CM1CONbits.CON = 1;			// Enable voltage comparator
	CM1CONbits.CPOL = 1;		// Inverse polarity
	CM1CONbits.EVPOL = 0;		// No event generation
	CM1CONbits.CREF = 1;		// Use the voltage reference
	CM1CONbits.CCH = 0;			// Monitor C1INB pin
}

void setupDMA()
{
// Configurate the DMA controller
	DMACON = 0;					// Reset the DMA controller
	DMACONbits.DMAEN = 1;		// Enable DMA
	DMACONbits.PRSSEL = 1;
	DMAH = 0xFFFF;
	DMAL = 0x0000;
}

void setupChargePump()
{
// Configurate the charge pump
	const int dt = FRQ_FCY / FRQ_PUMP;
	OC1R = dt / 2 - 1;			// Set pump frequency and PW ratio
	OC1RS = dt - 1;				// 50%
	OC1CON2 = 0x001F;			// Auto-sync
	OC1CON1 = 0x1C06;			// FCY clock / edge-aligned PWM mode
}
