/*
 * main.c
 */

/* Progetti di test per il bus I2C
	Created on: 20/apr/2015
 *
 *
 *
 *  Versione che sviluppa i controlli PID e e che si occupa di definire
 *  l'automa dei tasks.
 *  VERSIONE COMPLETA DI TEST: prove sui componenti in modalita' completa
 *  VERSIONE IN C++
 *  Ripresa dalla release precedente
 *  alcune prove
 *  In questa release si testa sia il giroscopio che l'accelerometro.
 *  Si testano anche i sensori di distanza (5 sensori)
 *
 *  INSERIMENTO DEL CICLO PRINCIPALE con PID SEMPLIFICATO
 *  RELEASE CON AGGIUNTA DELL'USO DELL'ACCELEROMETRO.
 */


#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "uartp/uartstdio.h"

#include "gyro_init.h"

#include "gen_def.h"

#include "uartp/uart.h"
#include "init.h"


#include "xbee/xbee.h"

#include "qei.h"

#include "parse.h"
#include "encQuad.h"




/// variabili globali
volatile int procCom = 0, tick10, tick100, millis10 = 0;
volatile int procCom4 = 0;
volatile int ADCDataReadyFlag = 0;
/// buffer per la seriale che riceve i dati dalla raspPi
extern volatile uint8_t uart1buffer[16], RX_PTR1, READ_PTR1;

/// MODULI ENCODER ///
encQuad ENC0, ENC1;



int main(void) {
	
	volatile uint32_t valore = 0, i, blink = 0, contatore, lampeggio_led;
	volatile int32_t arrot;
	volatile int16_t val1 = 0, x, y, z;
	syntaxStatus synStat;

	//--------------------------//
	///definizione strutture/////
	//-------------------------//


	/// imposta gli indirizzi dei due moduli
	ENC0.setAddr(QEI0_BASE);
	ENC1.setAddr(QEI1_BASE);

	/// descrittore della sintassi dei comandi
	syn_stat synSTATO;
	/// modulo zigbee per telemetria
	//xbee XB;
	xBee XB;


	/// disabilita le interruzioni
	DI();

	comando CMD1;
	//DATA.distPtr = &DIST;
	//passaggio degli indirizzi delle strutture alla struttura generale
	//dati_a_struttura(&G, &DIST, &CIN, &COL, &TEMP, &SUR, &DATA);
	/// l'oggetto COLLECTDATA (glb) e' una struttara che contiene i puntatori alle strutture e classi del progetto
	//datiRaccolti(&A, &ENC0, &sensIR, &CL, &SUR, &MISURE, &Rot, &COLLECTDATA);

	/// setup di base
	setupMCU();
	/// imposta i parametri del PID
	//setupPID(CTRL);

	/// imposta le UART e setta la PRINTF sulla 1 in modo da trasmettere la telemetria
	//setupUART(1);
	/// imposta le UART e setta la PRINTF sulla 0
	setupUART(0);
	PRINTF("impostata UART0 per debug\n");
	/// messaggio d'inizio
	PRINT_WELCOME();


	tick10 = tick100 = 0;
	/// inizializza il timer 0 e genera un tick da 10 ms
	initTimer0(INT_STEP_10_MS);
	PRINTF("inizializzato TIMER0\n");
	/// imposta il passo di integrazione per il calcolo dell'angolo

	/// inizializza il timer 1
	//initTimer1(100);
	/// inizializza il contatore della persistenza del comando
	synSTATO.tick = 0;


	PRINTF("inizializzato automa comandi\n");
	/// inizializzati i moduli encoder
	ENC0.qeiInit();
	ENC1.qeiInit();
	//servo = (pwm *) &pwmServi;

	/// abilita le interruzioni
	EI();
	PRINTF("abilitate interruzioni\n");


//
//	pwm_power(&PWM);
	contatore = 0;

	//// inizializza l'accelrometro
	//stato =  writeI2CByte(CTRL_REG1_A, ODR1 + ODR0 + ZaxEN + YaxEN + XaxEN);
	// scrivo nel registro 0x20 il valore 0x0F, cioe' banda minima, modulo on e assi on
	/// sintassi: indirizzo slave, num parm, indirizzo reg, valore da scrivere
	//I2CSend(ACCEL_ADDR, 2, CTRL_REG1_A, ODR1 + ODR0 + ZaxEN + YaxEN + XaxEN);
//	A.isPresent = testAccel();
//	if (A.isPresent)
//		impostaAccel(&A);
//
//	/// taratura sul sensore di luminosita'
//	whiteBal(&COL);
//	/// taratura del sensore di temepratura
//	taraturaTemp(&TEMP);
//
//	///
	//qei_test(&QEI);
	/// task principale
	int tempCont = 0;

	XB.test();
	int dir = 1, gradi = 0;
	/////////////////////////////////////////////////////////
	///
	///      TASK PRINCIPALE
	///
	/////////////////////////////////////////////////////////
//	setupUART(1);
//	XB.sendString("Ciao\n", 5);
	PRINTF("Telemetria\n");
	while(1){


/************************************************************/
/*  			ATTIVITA' SVOLTE AD OGNI CICLO				*/
/************************************************************/


		uint8_t valore_carattere = UARTCharGet(UART0_BASE);

		// controllo di messaggio sulla seriale 1 (ricevuto comando da rasp
		if (READ_PTR1 != RX_PTR1){
			 PARSE(&synSTATO);
			 if(synSTATO.valid == VALIDO){
				 /// invia la lettura
				 for(int i = 0; i < 4; i++){
					 /// invio dei 4 byte dell'intero little endian
					 volatile uint8_t valore;
					 valore = (ENC0.pos >> (8 * i)) & 0xff;
					 PRINTF ("%d", valore);
				 }
				 /// spaziatura
				 PRINTF("\n");
				 for(int i = 0; i < 4; i++){
					 /// invio dei 4 byte dell'intero little endian
					 volatile uint8_t valore;
					 valore = (ENC0.posV[i] >> (8 * i)) & 0xff;
					 PRINTF ("%d", valore);
				 }
				 /// spaziatura
				 PRINTF("\n");
				 for(int i = 0; i < 4; i++){
					 /// invio dei 4 byte dell'intero little endian
					 volatile uint8_t valore;
					 valore = (ENC0.posV[i + 8] >> (8 * i)) & 0xff;
 					 PRINTF ("%d", valore);
				 }
				 /// spaziatura
				 PRINTF("\n");

				 synSTATO.valid = NON_VALIDO;
			 }
			 /// aggiorna il buffer
			 READ_PTR1++;
			 READ_PTR1 &= DIM_READ_BUFF - 1;

		}

		/// invia la risposta per i comandi di rotazione, quando sono stati eseguiti
//		if(pidPtr->rispondi == TRUE){
//			rispostaRotazione(pidPtr, &synSTATO);
//			pidPtr->rispondi = FALSE;
//		}
//
		/*********************/
		/* AZIONI CADENZATE  */
		/*********************/

		/////////////////////////////////////
		/// AZIONI DA COMPIERE OGNI 10 ms ///
		/// aggiorna il PID ogni tick del timer che sono 10ms
		if (procCom == 1 ){
			//UARTCharPutNonBlocking(UART1_BASE, 'c');
			procCom = 0;
			contatore++;
			millis10++;



			/// le misure del giroscopio invece sono effettuate solo dall'apposito pid
		}
		/// effettua i calcoli solo se il giroscopio e' presente
		/// TODO: il PID viene calcolato ongi 10ms oppure ogni 20ms? Come è meglio?

		/////////////////////////////////////
		/// AZIONI DA COMPIERE OGNI 100ms ///
		if (tick10 >= 10){
			tick10 = 0;
			ENC0.readPos();
			ENC0.readDir();
			PRINTF("POS: %d\t%d\n", ENC0.pos, ENC0.dir);
		}
		/* misura gli encoder e calcola spostamenti e velocità */
		//////////////////////////////////
		/// AZIONI DA COMPIERE OGNI 1s ///
		if (tick100 >= 100){


			HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + (GPIO_PIN_3 << 2))) ^=  GPIO_PIN_3;



			////
			//// VIENE ESEGUITA QUANDO IL COMANDO E' RILASCIO KIT (comando 'P' da raspberry)
			///  E QUANDO il COMANDO e' SULLO STATO AVVIA. RILASCIATO IL PACK il COMANDO
			///  PONE avvia = false e NON lo RIESEGUIRA' più finché token sarà di nuovo
			///  RILASCIO_PACK e CMD con avvia = true. QUESTO ACCADE IN convertToToken,
			///  nel file parse.cpp
#ifdef _DEBUG_
			if (CMD1.avvia == true && synSTATO.token == RILASCIO_PACK){
				/// rilascio del kit

				CMD1.avvia = false;
				CMD1.isRun = false;
			}
#endif

			/// MISURA IL SENSORE DI ACCELERAZIONE

/// stampe dei valori dei sensori di distanza.
#ifdef _DEBUG_
			for(int i = 0; i < 5; i++){


			}
			PRINTF("\n");
//
#endif
//				/// converte la misure grezza in mm

#ifndef _DEBUG_
//				/// ricopia nella struttare DIST:
			for(int attesa = 0; attesa < 5; attesa++){
//					if (attesa == 3)
//						continue;
				PRINTF("mm(%d): %d \t", attesa, MISURE.d_mm[attesa]);
			}
#endif

#ifndef _DEBUG_

			contatore = 0;
			PRINTF("%d\tasse z: %d\t",tempCont++, Rot.yaw);
			printFloat(Rot.yawF, 4);
			PRINTF("\t");
			printFloat(Rot.yawF0, 4);
//					if (A.isPresent == true){
//						PRINTF("\t");
//						A.misuraAccelerazioni();
//					}
			PRINTF("\n");

#endif

			//// reset del contatore
			tick100 = 0;
		}



			/*if(G.IsPresent == OK)
				if( contatore == 1){
					/// ogni 10 ms effettua il calcolo del PID
					contatore = 0;
					HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + (GPIO_PIN_0 << 2))) |=  GPIO_PIN_0;
					PID(&G, pidPtr, &PWM, &CIN);
					setXPWM(&CTRL[1], &PWM);
					procCom = 0;
					HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + (GPIO_PIN_0 << 2))) &=  ~GPIO_PIN_0;
					blink++;
					/// lampeggio del led con periodo di 2 s.
					if (blink >= 100){
						HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + ((GPIO_PIN_2 | GPIO_PIN_1) << 2))) = 0;
						HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + (GPIO_PIN_3 << 2))) ^= GPIO_PIN_3;
						blink = 0;
					}
				///provvede ad integrare la misura della velcita' angolare ogni 10 ms
				//misuraAngoli(&G);
				//PRINTF("asse x: %d\t", G.pitch);
				//PRINTF("\tasse y: %d\t", G.roll);
				//PRINTF("\tasse z: %d\n", G.yaw);
				//PRINTF("uscita PID: %d\n", C.uscita);
			}*/

			/* RISPOSTA AL COMANDO */
			//inviaSensore(&synSTATO, &DATA);

		//}
	}
}



uint8_t printFloat(double number, uint8_t digits){

  //size_t n = 0;

  // Handle negative numbers
  if (number < 0.0)
  {
     PRINTF("-");
     number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i = 0; i < digits; ++i)
    rounding /= 10.0;

  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  PRINTF("%d", int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0) {
    PRINTF(".");
  }

  // Extract digits from the remainder one at a time
  while (digits-- > 0)
  {
    remainder *= 10.0;
    int toPrint = int(remainder);
    PRINTF("%d", toPrint);
    remainder -= toPrint;
  }


  return 0;
}
