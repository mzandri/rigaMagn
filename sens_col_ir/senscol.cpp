/*
 * senscol.c
 *
 *  Created on: 30/mar/2014
 *      Author: robotis
 */



#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "driverlib/rom.h"
#include "../i2c/tiva_i2c.h"
#include "sens.h"
#include "sens1.h"
#include "../uartp\uartstdio.h"

extern volatile bool scansione, letturaCampioni;
extern volatile int contLightPwm;
extern volatile int procCom4;

///
/// normalizza i colori sencondo norma 2
void normalizzaColori(colore *colPtr){

	float v[3], n = 0.0;
	int i;

	v[0] = (float) colPtr->rosso;
	v[1] = (float) colPtr->verde;
	v[2] = (float) colPtr->blu;

	for ( i = 0; i < 3; i++)
		n += v[i] * v[i];
	n = sqrt(n);

	/// normalizzazione dei colori
	colPtr->rosso /= n;
	colPtr->verde /= n;
	colPtr->blu   /= n;
}


///
/// taratura del sensore

int readCol(void){
	volatile uint32_t i = 0, tmp;
	/// accende il dispositivo
	HWREG(GPIO_PORTA_BASE + (GPIO_O_DATA + (GPIO_PIN_2 << 2))) = GPIO_PIN_2;
	//breve attesa
	for (i = 0; i < 1000; i++);
	/// avvia il timer che genera la finestra da 10 ms
	TimerEnable(TIMER4_BASE, TIMER_A);
	/// abilita le interruzioni sul pin PD1, che verrano contate
	GPIOIntEnable(GPIO_PORTA_BASE, GPIO_INT_PIN_3);
	/// attende la fine del campionamento.
	while(procCom4 == 0);
	/// ricarica per prossimo campionamento
	procCom4 = 0;
	tmp = contLightPwm;
	contLightPwm = 0;
	/// in contLiggthPwm c'e' la lettura dei conteggio del sensore:
	return tmp;
	/*contatore = 1;
	while (contatore == 1);
	/// entrato qui vuol dire che sto leggendo il sensore di colore
	while (letturaCampioni == false);
	/// ha termitato la finestra ed il dato e' pronto.
	colPtr->bianco = contaImpulsi;*/
}


///
/// taratura in un punto noto della temepratura
//void taraturaTemp(temperatura *tempPtr){
//	/// taratura del sensore
//	uint8_t buff[8];
//	uint32_t i;
//	uint16_t valore;
//	float temp;
//	/// temperatura rilevata dal case
//	I2CReceiveN(TEMP_ADDR, 0x06 | 0x20, 3, buff);
//	valore = (buff[1] << 8) + buff[0];
//	temp = (float)valore * 0.02 - 273.15;
//	tempPtr->Tcase = valore;
//	/// breve attesa
//	for (i=0; i < 1000; i++);
//	/// temp rilevata dal sensore
//	I2CReceiveN(TEMP_ADDR, TEMP_REG | 0x20, 3, buff);
//	valore = (buff[1] << 8) + buff[0];
//	temp = (float)valore * 0.02 - 273.15;
//	tempPtr->T_tar = temp;
//	tempPtr->tempRaw = valore;
//	tempPtr->ok = 1;
//}

///
/// lettura della temperatura
//void readTemp(temperatura *tempPtr){
//
//	unsigned char buffer[4];
//	//char valore;
//	/// per leggere la temperatura deve chiamare il sensore su I2C
//	I2CReceiveN(TEMP_ADDR, TEMP_REG | 0x20, 3, buffer);
//	tempPtr->tempRaw = (buffer[1] << 8) + buffer[0];
//	tempPtr->Temp = 0.02 * tempPtr->tempRaw  + 273.15;
//	tempPtr->ok = 0;
//}



///
/// inizializza il sensore di colore
void initLightSens1(void){

	/// S2 è collegato +5, S3 e' collegato a 0: sensibile al bianco
	/// S0-S1 sono collegati a +5V e abilitano il sensore alla massima potenza
//	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
//    /// settaggio PC7 e PC4 con corrente di 2mA
//
//    ROM_GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_7 | GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
//    //
//    // Make the pin(s) be outputs.
//    //
//    ROM_GPIODirModeSet(GPIO_PORTC_BASE, GPIO_PIN_7 | GPIO_PIN_4, GPIO_DIR_MODE_OUT);
//    /// accende PC4 e PC7.
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + ((GPIO_PIN_7 | GPIO_PIN_4) << 2))) =  GPIO_PIN_7 | GPIO_PIN_4;

    /// abilita i pin della PORTAA
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /// settaggio PA2 con corrente di 12mA

    ROM_GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);
    //
    // Make the pin(s) be outputs.
    //
    GPIODirModeSet(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_DIR_MODE_OUT);
    /// accende PA2
    HWREG(GPIO_PORTA_BASE + (GPIO_O_DATA + (GPIO_PIN_2 << 2))) = GPIO_PIN_2;
    /// Spegne PA2.
    HWREG(GPIO_PORTA_BASE + (GPIO_O_DATA + (GPIO_PIN_2 << 2))) =  0;
    //
    //setup of interrupt on PA3
    //
    //TODO: verificare se serve la resistenza di pull-up, in caso chiamare GPIOPadConfigSet DOPO di aver chiamto GPIOPinTypeInput
    ROM_GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_3);
    //GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_BOTH_EDGES);
    ROM_GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_RISING_EDGE);
    // abilita l'interruzione a livello di periferica

    //GPIOIntEnable(GPIO_PORTD_BASE, GPIO_INT_PIN_1);
    //verificare se la funzione è necessaria
    //GPIOIntRegister(GPIO_PORTD_BASE, *IntGPIOd);
    //
	// Set the interrupt priorities so they are all equal.
	//
	ROM_IntPrioritySet(INT_GPIOA, 0x00);
	/// disabilita la generazione di interruzioni da parte del pin PD1
	GPIOIntDisable(GPIO_PORTA_BASE, GPIO_INT_PIN_3);
    //
    // Enable the interrupts at CPU controller level.
    //
    ROM_IntEnable(INT_GPIOA);
}


///
///  abilitazione timer 4, usato per generare la finestra di lettura del sensore di luminosita'.
void initTimer4(uint8_t ms){
    //
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    volatile int tick = 0;

    tick = (ROM_SysCtlClockGet() / 1000) * ms; //ms sono i ms
    //
    // Enable the peripherals used by this example.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);
    //
    // Configure the two 32-bit periodic timers.
    //
    ROM_TimerConfigure(TIMER4_BASE, TIMER_CFG_PERIODIC);
    /// imposta il time_out
    ROM_TimerLoadSet(TIMER4_BASE, TIMER_A, tick);
    //
    // Setup the interrupts for the timer timeouts.
    //
    ROM_IntEnable(INT_TIMER4A);
    //ROM_IntEnable(INT_TIMER1A);
    ROM_TimerIntEnable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    //ROM_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}

///
/// legge il sensore tramite metodo della classe
float TEMPER::readTemp(){

	unsigned char buffer[4];
	if (i2cPtr == NULL){
		PRINTF("registrare il canale I2c per il pirometro\n");
		return -300.0;
	}
	//char valore;
	/// per leggere la temperatura deve chiamare il sensore su I2C
	i2cPtr->I2CGetN(TEMP_REG | 0x20, 3, buffer);
	//I2CReceiveN(TEMP_ADDR, TEMP_REG | 0x20, 3, buffer);
	tempRaw = (buffer[1] << 8) + buffer[0];
	Temp = 0.02 * tempRaw  - 273.15;
	Tint = (int)Temp;
	ok = 0;
	return Temp;
}

///
/// taratura del sensore di temperatura
void TEMPER::taraturaTemp(){
  	/// taratura del sensore
	uint8_t buff[8];
	uint32_t i;
	uint16_t valore;
	float temp;
	if (i2cPtr == NULL){
		PRINTF("registrare il canale I2c per il pirometro\n");
		return;
	}
	/// temperatura rilevata dal case
	i2cPtr->I2CGetN(0x06 | 0x20, 3 , buff);
	//I2CReceiveN(TEMP_ADDR, 0x06 | 0x20, 3, buff);
	valore = (buff[1] << 8) + buff[0];
	temp = (float)valore * 0.02 - 273.15;
	Tcase = valore;
	/// breve attesa
	for (i=0; i < 1000; i++);
	/// temp rilevata dal sensore
	i2cPtr->I2CGetN(TEMP_REG | 0x20, 3 , buff);
	//I2CReceiveN(TEMP_ADDR, TEMP_REG | 0x20, 3, buff);
	valore = (buff[1] << 8) + buff[0];
	temp = (float)valore * 0.02 - 273.15;
	T_tar = temp;
	tempRaw = valore;
	ok = 1;
}

/// collega la porta I2C
void TEMPER::attachI2C(I2C * p, uint8_t sa){
	i2cPtr = p;
	i2cPtr->I2CSetSlave_Add(sa);
}


///////////////////////////////////////////////////////////////////////////


COLORE::COLORE(){
	 luminanza = 0; bianco = 0; piastra.isDark = ISNT_DARK;
}

void COLORE::Run(){
	uint32_t lum = read();
	/// imposta la proprieta' luminanza
	set(lum);
	if(lum < getWhite() / 4){
		piastra.isDark = IS_DARK;
	}
	else
		piastra.isDark = ISNT_DARK;
}
