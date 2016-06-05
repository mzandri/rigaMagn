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

/// buffer per la seriale che riceve i dati dalla raspPi
extern volatile uint8_t uart1buffer[DIM_READ_BUFF], RX_PTR1, READ_PTR1;
extern volatile uint8_t uart0buffer[DIM_READ_BUFF], RX_PTR0, READ_PTR0;
extern uint8_t releaseHW[], releaseSW[];

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
	/// inizializzati i moduli encoder
	ENC0.qeiInit();
	ENC1.qeiInit();

	/// descrittore della sintassi dei comandi
	syn_stat synSTATO;
	/// modulo zigbee per telemetria
	xBee XB;

	/// disabilita le interruzioni
	DI();

	//comando CMD1;

	/// setup di base
	setupMCU();

	/// imposta le UART e setta la PRINTF sulla 1 in modo da trasmettere la telemetria
	//setupUART(1);
	/// imposta le UART e setta la PRINTF sulla 0
	setupUART(0);
	PRINTF("impostata UART0 per debug\n");
	/// messaggio d'inizio
	PRINT_WELCOME();
	PRINTF("%s", releaseSW);

	tick10 = tick100 = 0;
	/// inizializza il timer 0 e genera un tick da 10 ms
	initTimer0(INT_STEP_10_MS);
	PRINTF("inizializzato TIMER0\n");
	/// imposta il passo di integrazione per il calcolo dell'angolo

	/// inizializza il timer 1
	//initTimer1(100);
	/// inizializza il contatore della persistenza del comando
	synSTATO.tick = 0;
	resetAutoma(&synSTATO);
	PRINTF("inizializzato automa comandi\n");

	/// abilita le interruzioni
	EI();
	PRINTF("abilitate interruzioni\n");

	contatore = 0;

	XB.test();

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

		// controllo di messaggio sulla seriale 0 (ricevuto comando pc
		if (READ_PTR0 != RX_PTR0){
			 PARSE(&synSTATO);
			 if(synSTATO.valid == VALIDO){
				 /// SOLO PER SCOPI DI DEBUG E TARATURA SENSORE
				 /// per scopi di debug, stama i valori letti ad IDX
				 if (synSTATO.cmd[0] == 'a'){
					 /// stampa i valori
					 PRINTF("ENC0\n");
					 for (int i = 0; i < (MAX_NUM_PT / 8); i++){
						 for (int j = 0; j < 8; j++)
							 PRINTF("%d\t", ENC0.posV[i * 8 + j]);
						 /// va a capo
						 PRINTF("\n");
					 }
					 PRINTF("****\nFine taratura\n****\n");
				 }
				 /// legge la posizione e la fissa in modo da poter essere trasmessa.
				 ENC0.fixPos();
				 if (ENC0.posFix > ENC0.fscala / 2){
					/// posizione negativa
					ENC0.posFix -= ENC0.fscala;
				}
				 char buffer[4];
				 /// invia la lettura
				 for(int i = 0; i < 4; i++){
					 /// invio dei 4 byte dell'intero little endian

					 buffer[i] = (ENC0.posFix >> (8 * i)) & 0xff;
					 ////PRINTF ("%d", valore);
					 /// al posto di PRINTF() si invia il carattere direttamente
				 }
				 UARTwrite(buffer, 4);

				 //PRINTF("%d\t%d(%d)\n", ENC0.posFix, ENC0.contIDX, ENC0.posIDX);
				 for(int i = 0; i < 4; i++){
					 /// invio dei 4 byte dell'intero little endian
					 buffer[i] = (ENC0.posV[i] >> (8 * i)) & 0xff;
				 }
				 UARTwrite(buffer, 4);

				 for(int i = 0; i < 4; i++){
					 /// invio dei 4 byte dell'intero little endian

					 buffer[i] = (ENC0.posV[i + 8] >> (8 * i)) & 0xff;
 					 PRINTF ("%d", valore);
				 }
				 UARTwrite(buffer, 4);

				 synSTATO.valid = NON_VALIDO;
			 }
			 /// aggiorna il buffer
			 READ_PTR0++;
			 READ_PTR0 &= DIM_READ_BUFF - 1;

		}

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

		}
		/// effettua i calcoli solo se il giroscopio e' presente
		/// TODO: il PID viene calcolato ongi 10ms oppure ogni 20ms? Come è meglio?

		/////////////////////////////////////
		/// AZIONI DA COMPIERE OGNI 100ms ///
		if (tick10 >= 10){
			tick10 = 0;

		}
		/* misura gli encoder e calcola spostamenti e velocità */
		//////////////////////////////////
		/// AZIONI DA COMPIERE OGNI 1s ///
		if (tick100 >= 100){


			HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + (GPIO_PIN_3 << 2))) ^=  GPIO_PIN_3;
#ifndef _DEBUG_
			ENC0.readPos();
			ENC0.readDir();
			PRINTF("POS: %d\t%d\n", ENC0.pos, ENC0.dir);
#endif //_DEBUG_
			//// reset del contatore
			tick100 = 0;
		}
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
