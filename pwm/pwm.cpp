/*
 * pwm.c
 *
 *  Created on: 09/mar/2015
 *      Author: robotics
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/pwm.h"
#include "pwm.h"
#include "motpwm.h"




#define IN1 	GPIO_PIN_6		//PB6
#define IN2 	GPIO_PIN_7		//PB7
#define IN3 	GPIO_PIN_3		//PA3
#define IN4 	GPIO_PIN_2		//PA2

///
/// inizializza il numero di PWM usati ed il numero di servi usati
int PWM_MOTORI::numPwm = 0;
int PWM_SERVI::numServi = 0;

void PWM_MOTORI::Init(){

	if(numMot == 1){
		/// per ottenre una frequenza di 50 Hz necessaria ai servi, partendo da 80MHz di clock si deve dividere pe la costante 1.6e6
		/// che è ottenuta come prodotto tra 50000 (contenuto del registro di fine conteggio a 16 bit della cpu) e 32 (divisore del clock di sistema)
		/// 80000000 / (50 * 32) = 50000. Infatti il tempo di raggiungere 1600000 conteggi  e' di 20 ms
		/// Per ottenre una frequenza di 8kHz occorre invece dividere il clock per la costante 10000 = 32 * 312.5 e quindi il fine conteggio
		/// dovra' essere 312. Ci'o significa che la granularità del PWM e' di 1/312 = 0.32% inferiore a quanto riloevabile dai motori
		/// I valori sono quindi: 80000000 / (8000 * 32) = 80000000 / 256000 e quindi il fine conteggio e' 312.5
	    NCont = ROM_SysCtlClockGet() / 256000;
	    delta = 0;

		/// Abilita le porte usate dal PWM, e per il comando della direzione dei motori,
		/// anche se potrebbero gia' esserlo stato inaltre parti del programma
	    /// il primo pwm motori deve abilitare PE5
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
		/// mentre PE4 e' una uscita digitale che combianta con il segnale PWM stabilisce la potenza al motore ed il verso di rotazione
		/// In pratica se PE4 = '0' e PE5 ha un PWM, supponendo che il motore giri in senso orario, allora
		/// quando PE4 = '1' e PE5 ha lo stesso PWM ma con polarita' invertita, il motore ruota in senso orario.

		ROM_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4);

		/// azzeramento delle uscite dei pin
		HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + (GPIO_PIN_4) << 2)) =  0;
		//HWREG(GPIO_PORTA_BASE + (GPIO_O_DATA + (GPIO_PIN_3 | GPIO_PIN_2) << 2)) =  0;

	    ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_32);  //divisore per 32
	    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0); //modulo pwm numero 0

	    //Configure PE5 Pins as PWM
	    ROM_GPIOPinConfigure(GPIO_PE5_M0PWM5);  //generatore 2
	    //ROM_GPIOPinConfigure(GPIO_PB4_M0PWM2); // generatore 1
	    ///PE5 e' il modulo 1, mentre PB4 e' il modulo 2 del L298
	    ROM_GPIOPinTypePWM(GPIO_PORTE_BASE, GPIO_PIN_5);


	    //il generatore e' il numero 2
	    ROM_PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

	    //Imposta il periodo del PWM rispetto al clock
	    ROM_PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, NCont);

	    // Enable the PWM generator
	    ROM_PWMGenEnable(PWM0_BASE, PWM_GEN_2);

	    // Turn on the Output pins (quello di M0PWM5)
	    ROM_PWMOutputState(PWM0_BASE, PWM_OUT_5_BIT, true);
	    /// PWM non invertito
	    ROM_PWMOutputInvert(PWM0_BASE, PWM_OUT_5_BIT, false);

	    /// spegne i PWM
	    numPin = PWM_OUT_5;
	    MotorStop();

	}
	else{
		/// per ottenre una frequenza di 50 Hz necessaria ai servi, partendo da 80MHz di clock si deve dividere pe la costante 1.6e6
		/// che è ottenuta come prodotto tra 50000 (contenuto del registro di fine conteggio a 16 bit della cpu) e 32 (divisore del clock di sistema)
		/// 80000000 / (50000 * 32) = 50. Infatti il tempo di raggiungere 1600000 conteggi  e' di 20 ms
		/// Per ottenre una frequenza di 8kHz occorre invece dividere il clock per la costante 10000 = 32 * 312.5 e quindi il fine conteggio
		/// dovra' essere 312. Ci'o significa che la granularità del PWM e' di 1/312 = 0.32% inferiore a quanto riloevabile dai motori
		/// I valori sono quindi: 80000000 / (8000 * 32) = 80000000 / 256000 e quindi il fine conteggio e' 312.5
		NCont = ROM_SysCtlClockGet() / 256000;
		delta = 0;

		/// Abilita le porte usate dal PWM, e per il comando della direzione dei motori,
		/// anche se potrebbero gia' esserlo stato inaltre parti del programma
		/// il secondo pwm motori deve abilitare PB4
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		/// mentre PE4 e' una uscita digitale che combianta con il segnale PWM stabilisce la potenza al motore ed il verso di rotazione
		/// In pratica se PA5 = '0' e PB4 ha un PWM, supponendo che il motore giri in senso orario, allora
		/// quando PA5 = '1' e PB4 ha lo stesso PWM ma con polarita' invertita, il motore ruota in senso orario.

		ROM_GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_5);

		/// azzeramento delle uscite dei pin
		HWREG(GPIO_PORTA_BASE + (GPIO_O_DATA + (GPIO_PIN_5) << 2)) =  0;
		//HWREG(GPIO_PORTA_BASE + (GPIO_O_DATA + (GPIO_PIN_3 | GPIO_PIN_2) << 2)) =  0;

		ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_32);  //divisore per 32
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0); //modulo pwm numero 0

		//Configure PB4 Pins as PWM
		//ROM_GPIOPinConfigure(GPIO_PB4_M0PWM5);  //generatore 1
		ROM_GPIOPinConfigure(GPIO_PB4_M0PWM2); // generatore 2
		///PE5 e' il modulo 1, mentre PB4 e' il modulo 2 del L298
		ROM_GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_4);


		//il generatore e' il numero 1
		ROM_PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

		//Imposta il periodo del PWM rispetto al clock
		ROM_PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, NCont);

		// Enable the PWM generator
		ROM_PWMGenEnable(PWM0_BASE, PWM_GEN_1);

		// Turn on the Output pins (quello di M0PWM2)
		ROM_PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true);
		/// VA FATTO L'OR CON L'ALTRO PIN????
		/// PWM non invertito
		ROM_PWMOutputInvert(PWM0_BASE, PWM_OUT_2_BIT, false);

		/// spegne i PWM
		numPin = PWM_OUT_2;
		MotorStop();

	}
}


///
/// spegne il motore associato al PWM
void PWM_MOTORI::MotorStop(){
	delta = 0;
	direction = 1;
	MotorGo();
	//pwmPower();
}


////
//// questa funzione modifica il valore del PWM per l'oggetto associato al motore.
void PWM_MOTORI::pwmPower(){
	PWMPulseWidthSet(PWM0_BASE, numPin, NCont * delta / 100);
}


#include "inc/hw_pwm.h"
///
/// muove il motore
void PWM_MOTORI::MotorGo(){
	/// FATTORI DI CORREZIONE DEI MOTORI. SI NOTA CHE IL N.1 VA UN PO' MENO DEL NUMERO 2
	float f, k;
	int valPwm;
	switch (numMot){
	case 1:
		if (direction == 1){
			//PWMOutputInvert(PWM0_BASE, PWM_OUT_5_BIT, false);
			/// imposta il pwm in modalita' affermata
			HWREG(PWM0_BASE + PWM_O_INVERT) &= ~(PWM_OUT_5_BIT);
			}
		else{
			//PWMOutputInvert(PWM0_BASE, PWM_OUT_5_BIT, true);
			/// imposta il pwm in modalita' negata
			HWREG(PWM0_BASE + PWM_O_INVERT) |= PWM_OUT_5_BIT;
		}
		/// correzione
		k = 1.076;
		f = NCont * delta * k / 100;
		valPwm = (int) f;
		PWMPulseWidthSet(PWM0_BASE, numPin, valPwm);
	break;
	case 2:
		if (direction == 1){
			//PWMOutputInvert(PWM0_BASE, PWM_OUT_5_BIT, false);
			/// imposta il pwm in modalita' affermata
			HWREG(PWM0_BASE + PWM_O_INVERT) &= ~(PWM_OUT_2_BIT);
			}
		else{
			//PWMOutputInvert(PWM0_BASE, PWM_OUT_5_BIT, true);
			/// imposta il pwm in modalita' negata
			HWREG(PWM0_BASE + PWM_O_INVERT) |= PWM_OUT_2_BIT;
		}
		k = 1.0;
		f = NCont * delta * k / 100;
		valPwm = (int) f;
		PWMPulseWidthSet(PWM0_BASE, numPin, valPwm);
	break;
	}
}

///
/// inizializzazione per i  servi
/// vengono posti sui pin PD0 e PD1, col generatore PWM n. 1 con periodo di 20ms
void PWM_SERVI::Init(){

	NCont = ROM_SysCtlClockGet() / 1600;
    delta = 0;

    if(numS == 1){
		/// per ottenre una frequenza di 50 Hz necessaria ai servi, partendo da 80MHz di clock si deve dividere pe la costante 1.6e6
		/// che è ottenuta come prodotto tra 50000 (contenuto del registro di fine conteggio a 16 bit della cpu) e 32 (divisore del clock di sistema)
		/// 80000000 / (50 * 32) = 50000. Infatti il tempo di raggiungere 1600000 conteggi  e' di 20 ms

		/// Abilita le porte usate dal PWM, e per il comando della direzione dei motori,
		/// anche se potrebbero gia' esserlo stato inaltre parti del programma
	    /// il primo pwm motori deve abilitare PD0
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
		/// mentre PE4 e' una uscita digitale che combianta con il segnale PWM stabilisce la potenza al motore ed il verso di rotazione
		/// In pratica se PE4 = '0' e PE5 ha un PWM, supponendo che il motore giri in senso orario, allora
		/// quando PE4 = '1' e PE5 ha lo stesso PWM ma con polarita' invertita, il motore ruota in senso orario.
		ROM_GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0);

		/// azzeramento delle uscite dei pin
		HWREG(GPIO_PORTD_BASE + (GPIO_O_DATA + (GPIO_PIN_0) << 2)) =  0;

	    ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_32);  //divisore per 32
	    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1); //modulo pwm numero 1

	    //Configure PD0 Pins as PWM
	    ROM_GPIOPinConfigure(GPIO_PD0_M1PWM0);  //modulo 1 generatore 0

	    ROM_GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);

	    //il generatore e' il numero 0
	    ROM_PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

	    //Imposta il periodo del PWM rispetto al clock
	    ROM_PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, NCont);

	    // Enable the PWM generator
	    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0);

	    // Turn on the Output pins (quello di M1PWM6)
	    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
	    /// PWM non invertito
	    ROM_PWMOutputInvert(PWM1_BASE, PWM_OUT_0_BIT, false);

	    /// spegne i PWM
	    numPin = PWM_OUT_0;
	    MotorStop();

	}
	else{
		/// per ottenre una frequenza di 50 Hz necessaria ai servi, partendo da 80MHz di clock si deve dividere pe la costante 1.6e6
		/// che è ottenuta come prodotto tra 50000 (contenuto del registro di fine conteggio a 16 bit della cpu) e 32 (divisore del clock di sistema)
		/// 80000000 / (50 * 32) = 50000. Infatti il tempo di raggiungere 1600000 conteggi  e' di 20 ms

		ROM_GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_1);

		/// azzeramento delle uscite dei pin
		HWREG(GPIO_PORTD_BASE + (GPIO_O_DATA + (GPIO_PIN_1) << 2)) =  0;
		//ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_32);  //divisore per 32
		//ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1); //modulo pwm numero 1

		//Configure PF3 Pins as PWM
		//ROM_GPIOPinConfigure(GPIO_PB4_M0PWM5);  //generatore 1
		ROM_GPIOPinConfigure(GPIO_PD1_M1PWM1); // generatore 1

		ROM_GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_1);

	    //il generatore e' il numero 1
	    ROM_PWMGenConfigure(PWM1_BASE, PWM_GEN_1, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

	    //Imposta il periodo del PWM rispetto al clock
	    ROM_PWMGenPeriodSet(PWM1_BASE, PWM_GEN_1, NCont);

	    // Enable the PWM generator
	    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_1);

		// Turn on the Output pins (quello di M0PWM2)
		ROM_PWMOutputState(PWM1_BASE, PWM_OUT_1_BIT, true);
		/// VA FATTO L'OR CON L'ALTRO PIN????
		/// PWM non invertito
		ROM_PWMOutputInvert(PWM1_BASE, PWM_OUT_1_BIT, false);

		/// spegne i PWM
		numPin = PWM_OUT_1;
		MotorStop();

	}
}


void PWM_SERVI::MotorGo(int16_t gradi){
	switch (numS){
	///
	/// attiva il motore su PA6 lo scarico del kit
	uint32_t valFin;
	case 1:

		valFin = convertDeg2Pwm(gradi);
		ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, valFin / 100);
	break;

	case 2:

		valFin = convertDeg2Pwm(gradi);
		ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_1, valFin / 100);
	}
}


void PWM_SERVI::MotorStop(){
	switch (numServi){
	///
	/// Spegne il pwm corrispondente
	case 1:
		ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 0);
	break;

	case 2:
		ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_1, 0);
	}
}

///
/// funzione che converte i gradi in sewgnale pwm per i servomotori
uint32_t PWM_SERVI::convertDeg2Pwm(int16_t gradi){

	/// -90° -> delta_2 = 5; +90° -> delta_2 = 10
	float rot, delta;
	uint32_t valFin;
	rot = gradi / 36.0 + 7.50;
	delta = (float) NCont * rot;
	valFin = (uint32_t) delta;
	return valFin;
}

void PWM_SERVI::scarico(){
	MotorGo(-30);
	for (int i = 2000000; i > 0; i--);
	MotorGo(80);
}
