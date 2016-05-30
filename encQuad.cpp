/*
 * encQuad.cpp
 *
 *  Created on: 23/feb/2016
 *      Author: massimo
 */

#include "encQuad.h"

#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "driverlib/qei.h"

#include "pwm/pwm.h"
#include "uartp/uartstdio.h"
#include "uartp/uart.h"

extern "C" void IntEnc0(void);
extern "C" void IntGPIOf(void);
extern encQuad ENC0;

void UnlockPD7_01()
{
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) 	 = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTD_BASE + GPIO_O_CR)		|= 0x80;
	HWREG(GPIO_PORTD_BASE + GPIO_O_AFSEL) 	&= 0x80;
	//HWREG(GPIO_PORTD_BASE + GPIO_O_DEN) 	&= Pin;
	HWREG(GPIO_PORTD_BASE + GPIO_O_PCTL) 	|= 0x60000000;
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) 	 = 0;

}

encQuad::encQuad() {
	// TODO Auto-generated constructor stub
	address 	= 0;

	fscala 		= 40000;
	zero_pos 	= 0;
	vel_period = ROM_SysCtlClockGet()/10;
	vel = 0;
	indice = 0;
	contIDX = 0;

}

encQuad::~encQuad() {
	// TODO Auto-generated destructor stub
}

void encQuad::qeiInit(){

	///TODO: tarare i due valori: periodo e fondoscala
	//vel_period = ROM_SysCtlClockGet()/10;
	//fscala = 80000;
	//zero_pos = 0;

	// *************************************************************************************************** Lx4F232H5QD
	//    Initialize right side QEI (Quadrature Encoder Interface) for use by the right side drive.
	//    Uses GPIO Port D bits PD6 & PD7 (There is no Index wire from the encoder, perhaps we could install one on the axle).
	//    Note: Port D pin 7 defaults to a NMI input at reset. That functionality is protected by a mechanism
	//    which must be "unlocked" before it can be made available to another module (like QEI0).
	// ***************************************************************************************************

	if (address == QEI0_BASE || address == QEI1_BASE){

		switch(address){
		case QEI0_BASE:
			// Enable Port D module so we can work with it
			SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
			SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
			// Make pin direction of bits 6 and 7 INPUTS (this may be unnecessary?)
			UnlockPD7_01();
			GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_7 | GPIO_PIN_6, GPIO_DIR_MODE_IN);
			//GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);
			// Enable programming access to QEI Module 0
			SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);
			QEIDisable(address);
			// Tell the mux which particular QEI function to connect to specified pin
			GPIOPinConfigure(GPIO_PD6_PHA0);
			GPIOPinConfigure(GPIO_PD7_PHB0);    // now redundant
			//GPIOPinConfigure(GPIO_PF4_IDX0);
			// Tell the GPIO module which pins will be QEI type pins
			GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_7 | GPIO_PIN_6);
			//GPIOPinTypeQEI(GPIO_PORTF_BASE, GPIO_PIN_4);
			/// imposta PF4 a ricevere interruzioni (segnale IDX)
		    ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
		    //GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_BOTH_EDGES);
		    ROM_GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);
		    // abilita l'interruzione a livello di periferica

		    //GPIOIntEnable(GPIO_PORTD_BASE, GPIO_INT_PIN_1);
		    //verificare se la funzione � necessaria
		    //GPIOIntRegister(GPIO_PORTD_BASE, *IntGPIOd);
		    //
			// Set the interrupt priorities so they are all equal.
			//
			ROM_IntPrioritySet(INT_GPIOF, 0x00);
			/// disabilita la generazione di interruzioni da parte del pin PD1
			GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
		    //
		    // Enable the interrupts at CPU controller level.
		    //
		    ROM_IntEnable(INT_GPIOF);
		break;

		case QEI1_BASE:
			SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
			GPIODirModeSet(GPIO_PORTC_BASE, GPIO_PIN_5 | GPIO_PIN_6, GPIO_DIR_MODE_IN);
			// Enable programming access to QEI Module 1
			SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI1);
			QEIDisable(address);
			// Tell the mux which particular QEI function to connect to specified pin
			GPIOPinConfigure(GPIO_PC5_PHA1);
			GPIOPinConfigure(GPIO_PC6_PHB1);
			// Tell the GPIO module which pins will be QEI type pins
			GPIOPinTypeQEI(GPIO_PORTC_BASE, GPIO_PIN_5 | GPIO_PIN_6);
		break;
		}

		//    // Write this 'key' 0x4C4F434B into Port D's Lock register to enable access to Port D's Commit register
		//
		//    //HWREG(GPIO_0_LOCK) |= GPIO_LOCK_KEY;
		//    GPIO_O_LOCK = GPIO_LOCK_KEY;
		//
		//    // Flip only bit 7 ON to ALLOW Port D bit 7 to be switched from NMI use to QEI use
		//    GPIO_O_CR |= 0x80;
		//
		//
		//    // Switch pin usage
		//    GPIO_O_AFSEL |= 0x80;            // Selects alternative usage for the pin
		//    //GPIO_PORTD_PCTL_R  |= 0x60000000;    // Selects QEI0 PHB0 in particular (pages 722 & 1405 in LM4F232H5QD manual)
		//    GPIO_O_PCTL |= 0x60000000;
		//
		//    // Flip only bit 7 OFF to Re-lock
		//    GPIO_O_CR &= !0x08;

		//
		//! Configures the quadrature encoder.
		//!
		//! \param ui32Base is the base address of the quadrature encoder module.
		//! \param ui32Config is the configuration for the quadrature encoder.  See
		//! below for a description of this parameter.
		//! \param ui32MaxPosition specifies the maximum position value.
		//!
		//! This function configures the operation of the quadrature encoder.  The
		//! \e ui32Config parameter provides the configuration of the encoder and is
		//! the logical OR of several values:
		//!
		//! - \b QEI_CONFIG_CAPTURE_A or \b QEI_CONFIG_CAPTURE_A_B specify if edges
		//!   on channel A or on both channels A and B should be counted by the
		//!   position integrator and velocity accumulator.
		//! - \b QEI_CONFIG_NO_RESET or \b QEI_CONFIG_RESET_IDX specify if the
		//!   position integrator should be reset when the index pulse is detected.
		//! - \b QEI_CONFIG_QUADRATURE or \b QEI_CONFIG_CLOCK_DIR specify if
		//!   quadrature signals are being provided on ChA and ChB, or if a direction
		//!   signal and a clock are being provided instead.
		//! - \b QEI_CONFIG_NO_SWAP or \b QEI_CONFIG_SWAP to specify if the signals
		//!   provided on ChA and ChB should be swapped before being processed.
		//!
		//! \e ui32MaxPosition is the maximum value of the position integrator and is
		//! the value used to reset the position capture when in index reset mode and
		//! moving in the reverse (negative) direction.
		//!
		//! \return None.

		//configurazione qei
		QEIConfigure(address,(QEI_CONFIG_CAPTURE_A | QEI_CONFIG_RESET_IDX | QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), fscala);
		//QEIConfigure(QEI1_BASE,(QEI_CONFIG_CAPTURE_A | QEI_CONFIG_NO_RESET | QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), fscala);

		QEIPositionSet(address, zero_pos);
		//QEIPositionSet(QEI0_BASE, zero_pos);

		//configurazione lettore velocit� del qei
		QEIVelocityDisable(address);
		//QEIVelocityDisable(QEI1_BASE);

		QEIVelocityConfigure(address, QEI_VELDIV_2, vel_period);  //periferica, divisore, periodo
		//QEIVelocityConfigure(QEI1_BASE, QEI_VELDIV_2, vel_period);  //periferica, divisore, periodo

		QEIVelocityEnable(address);
		//QEIVelocityEnable(QEI1_BASE);


	//	///TODO: se si vuole usare l'interrupt del qei vanno decommentate le due righe che seguono
		//IntEnable(INT_QEI0);
	//	//IntEnable(INT_QEI1);
	//
	//	//QEIIntEnable(QEI0_BASE, QEI_INTDIR | QEI_INTTIMER); //interruzione abilitata al cambio di direzione e al timer della velocit� finito
	//	QEIIntEnable(QEI0_BASE, QEI_INTDIR);
	//	QEIIntEnable(QEI0_BASE, QEI_INTINDEX);

	//	QEIIntEnable(QEI1_BASE, QEI_INTDIR);
		QEIEnable(address);
		//QEIEnable(QEI0_BASE);
	}
}


void encQuad::readPos(){
	float tmp, k = 1.0;
	pos = QEIPositionGet(address);
	tmp = pos * k;
	dist_mm = (int) tmp;

}

void encQuad::readVel(){

	;

}

void encQuad::readDir(){

	dir = QEIDirectionGet(address);

}

///
/// aggiorna i parametri dell'encoder
void encQuad::update(){

	pos =  QEIPositionGet(address);
	vel =  QEIVelocityGet(address);
	dir =  QEIDirectionGet(address);

}


///
/// funziona solo se si imposta in modalita' reset alla ricezione di IDX
void encQuad::intIDXEnable(){
	QEIIntEnable(address, QEI_INTINDEX);
}


///
///
void IntEnc0(void){
	QEIIntClear(QEI0_BASE, QEI_INTINDEX | QEI_INTDIR);

	///occorrerebbe contare il numero di IDX
}

///
/// interruzione che scatta alla ricezione di IDX
void IntGPIOf(void){
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
	ENC0.pos = QEIPositionGet(QEI0_BASE);
	if (ENC0.pos > ENC0.fscala / 2){
		/// posizione negativa
		ENC0.pos -= ENC0.fscala / 2;
	}
	ENC0.posV[ENC0.indice++] = ENC0.pos;
	ENC0.indice &= 63;
	if (QEIDirectionGet(QEI0_BASE) == 1)
		ENC0.contIDX++;
	else
		ENC0.contIDX--;
}

