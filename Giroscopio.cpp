/*
 * Giroscopio.cpp
 *
 *  Created on: 02/mar/2016
 *      Author: massimo
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "gyro_init.h"
#include "uartp/uartstdio.h"
#include "I2C/tiva_i2c.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "gen_def.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "uartp/uartstdio.h"
#include "uartp/uart.h"

#include "Giroscopio.h"

Giroscopio::Giroscopio() {
	// TODO Auto-generated constructor stub
	tempReg = 0;
	posizione = 0;
	yawF0 = 0.0;
	IsRotating = offsetRequest = 0;
	i2cPtr = NULL;
}

Giroscopio::~Giroscopio() {
	// TODO Auto-generated destructor stubù
	i2cPtr = NULL;
}

void Giroscopio::azzeraAssi(){
	int conteggio = 0;
	int valore;
	uint8_t buffer[2];
	uint32_t i ;
	int16_t x = 0, y = 0, z = 0;
	uint8_t buffer3A[8];

	switch(asseOn){

	case ALL_AXIS:
		/// assi ON
		if (i2cPtr == NULL){
			while(conteggio < 32){
				/// effettua 32 letture e calcola la media
				valore = I2CReceive(GYRO_ADDR,STATUS_REG);
				//PRINTF("REG_STAT 0x%x\n", valore);
				if (valore != 0){
					/// tutti gli assi on
					I2CReceiveN(GYRO_ADDR,OUT_X_L | MUL_READ , 6, buffer);
					x += (int16_t)((buffer3A[1]<< 8) + buffer3A[0]);
					y += (int)((buffer3A[3]<< 8) + buffer3A[2]);
					z += (int)((buffer3A[5]<< 8) + buffer3A[4]);
					conteggio++;
					for (i = 0; i < 50000; i++);
				}
			}
			/// calcola la media: divide  per 32
			x /= 32;
			y /= 32;
			z /= 32;
			/// assegna il valore nella struct G
			x0 = x;
			y0 = y;
			z0 = z;
		}
		else{
			while(conteggio < 32){
				/// effettua 32 letture e calcola la media+
				valore = i2cPtr->I2CGet(STATUS_REG);
				//PRINTF("REG_STAT 0x%x\n", valore);
				if (valore != 0){
					/// tutti gli assi on
					i2cPtr->I2CGetN(OUT_X_L | MUL_READ , 6, buffer);
					x += (int16_t)((buffer3A[1]<< 8) + buffer3A[0]);
					y += (int)((buffer3A[3]<< 8) + buffer3A[2]);
					z += (int)((buffer3A[5]<< 8) + buffer3A[4]);
					conteggio++;
					for (i = 0; i < 50000; i++);
				}
			}
			/// calcola la media: divide  per 32
			x /= 32;
			y /= 32;
			z /= 32;
			/// assegna il valore nella struct G
			x0 = x;
			y0 = y;
			z0 = z;
		}
	break;

	case Z_AXIS:
		/// asse z ON
		media = 0;

		if (i2cPtr == NULL){

			while(conteggio < numSampleBias){
				/// effettua numSampleBias letture e calcola la media
				valore = I2CReceive(GYRO_ADDR,STATUS_REG);
				//PRINTF("REG_STAT 0x%x\n", valore);
				if (valore != 0){
					/// asse z ON
					I2CReceiveN(GYRO_ADDR,OUT_Z_L | MUL_READ , 2, buffer);
					buffValori[conteggio] = (int16_t)((buffer[1]<< 8) + buffer[0]);
					buffX[conteggio] = conteggio;
					media += buffValori[conteggio];
					//z += (int16_t )((buffer[1]<< 8) + buffer[0]);
					conteggio++;
					for (int i = 0; i < 50000; i++);
				}
			}
		}
		else{
			while(conteggio < numSampleBias){
				/// effettua numSampleBias letture e calcola la media
				valore = i2cPtr->I2CGet(STATUS_REG);
				//PRINTF("REG_STAT 0x%x\n", valore);
				if (valore != 0){
					/// asse z ON
					i2cPtr->I2CGetN(OUT_Z_L | MUL_READ , 2, buffer);
					buffValori[conteggio] = (int16_t)((buffer[1]<< 8) + buffer[0]);
					buffX[conteggio] = conteggio;
					media += buffValori[conteggio];
					//z += (int16_t )((buffer[1]<< 8) + buffer[0]);
					conteggio++;
					for (int i = 0; i < 50000; i++);
				}
			}
		}
		media /= numSampleBias;

		//printAsseZ(64);
		minQuad(buffX, buffValori, numSampleBias, &m, &q);
		z0 = (int16_t) media;
		//tempReg = 0;
	break;
	}

}


///
/// stampa i valori dell'asse z del giroscopio

void Giroscopio::printAsseZ(int numS){
	for (int16_t i = 0; i < numS; i++)
		PRINTF("%d\n", buffValori[i]);
}


void Giroscopio::initGyro(char assi){
	volatile uint32_t valore;
	IsOn = OFF;
	yaw = roll = pitch = 0;
	if (i2cPtr == NULL){
		//chiedo il dato presente nel registro WHO_AM_I
		if (I2CReceive(GYRO_ADDR, WHO_AM_I) == 0xD4){
			blinkBlueLed();
			IsPresent = OK;
			/// imposta gli assi
			/// per il drone, stato = ALL (0x7)
			/// per il rover stato = Z (0x4);
			setupAssi(assi);
		}
		else{
			IsPresent = NOT_PRESENT;
			PRINTF("Giroscopio non presente\n");
			return;
		}
	}
	else{
		if (i2cPtr->I2CGet(WHO_AM_I) == 0xD4){
			blinkBlueLed();
			IsPresent = OK;
			/// imposta gli assi
			/// per il drone, stato = ALL (0x7)
			/// per il rover stato = Z (0x4);
			setupAssi(assi);
		}
		else{
			IsPresent = NOT_PRESENT;
			PRINTF("Giroscopio non presente\n");
			return;
		}
	}
}


///
/// impostazione delle funzionalita del giroscopio
/// in questo punto imposto la velocita' 250 gradi/secondo
void Giroscopio::setupAssi(char stato){
	char mask;
	uint32_t valore;
	x0 = y0 = z0 = 32767;
	pitchF = rollF = yawF = pitchP = rollP = yawP = 0.0;
	gradiSec = 0;
	/// trovato empiricamente
	kz = 1.140;
	/// lo stato e' cosi' interpretato: bit0: x; bit1: y; bit2: z.
	/// scrivo nel registro 0x20 il valore 0x0C, cioe' banda minima, modulo on e assi on
	/// sintassi: indirizzo slave, num parm, indirizzo reg, valore da scrivere
	mask = 0x08 | stato;
	if (i2cPtr == NULL){
		I2CSend(GYRO_ADDR, 2, CTRL_REG1, mask);
		if(I2CReceive(GYRO_ADDR, CTRL_REG1) == mask){
			//GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
			IsOn = ON;
			asseOn = stato & 0x7;
		}

		/// set FS to 500 degree per sec.
		I2CSend(GYRO_ADDR, 2, CTRL_REG4, FS_250);
		gradiSec = 250;
		FS = (float) 250.0 / 32768.0;
		valore = I2CReceive(GYRO_ADDR,CTRL_REG4);

	}
	else{
		i2cPtr->I2CPut(2, CTRL_REG1, mask);
		if(i2cPtr->I2CGet(CTRL_REG1) == mask){
			//GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
			IsOn = ON;
			asseOn = stato & 0x7;
		}
		/// set FS to 500 degree per sec.
		i2cPtr->I2CPut(2, CTRL_REG4, FS_250);
		gradiSec = 250;
		FS = (float) 250.0 / 32768.0;
		valore = i2cPtr->I2CGet(CTRL_REG4);

	}
	PRINTF("Lettura dal REG4 %d - deg/s %d \n", valore, gradiSec);
}

///
/// integra la misura del'angolo a partire dalla velocità angolare
///
void Giroscopio::misuraAngoli(){

	static uint32_t tempoDiReset = 0;
	uint32_t valore;
	int16_t z, x, y, tmp;
	uint8_t buffer[8];
	float f, DPS = FS;
	/// controlla se c'e' la classe I2C oppure e' il vecchio metodo,
	/// cioe' la classe Giroscopio gestisce il bus.
	if(i2cPtr == NULL)
		valore = I2CReceive(GYRO_ADDR, STATUS_REG);
	else
		valore = i2cPtr->I2CGet(STATUS_REG);

	if (valore != 0){
		tempoDiReset++;
		/// legge i dati da tutti i registri del giroscopio
		/// stato = readI2C_N_Byte(OUT_X_L_M, 6, buff);			/// compass
		if ((asseOn & Z_AXIS) == Z_AXIS){
			if(i2cPtr == NULL)
				I2CReceiveN(GYRO_ADDR, OUT_Z_L | MUL_READ , 2, buffer);
			else
				i2cPtr->I2CGetN(OUT_Z_L | MUL_READ , 2, buffer);
			z = (int16_t)((buffer[1]<< 8) + buffer[0]);
			tmp = z;
			z = z - z0;
#ifdef _DEBUG_
			/// usa la retta di regressione
			tmp -=  q;
#endif
			/// integrazione rettangolare: valore letto * fondo scala * intervallo di tempo di integrazione
			/// posto a 10ms
			f = z * DPS * kz;
			f *= tick;
			yawF += f;
#ifdef	_DEBUG_
			f = tmp * DPS * kz;
			f *= tick;
			yawF0 += f;
#endif
			if ((tempoDiReset >= 1000 && IsRotating == 0) || offsetRequest){
// TODO: SE NON STA RUOTANDO POTREBBE EFFETTUARE UN AZZERAMENTO ASSI
				/// le richieste a seguito di fine rotazione non possono restare
				/// attive più di una volta.
				offsetRequest = 0;
				/// sono passati 10 secondi e dovrebbe ricalcolare l'offset degli assi
				/// dovrebbe calcolare se G->yawF è cambiato di almeno 1 grado rispetto ai 5 secondi precedenti
				/// se non è cambiato dovrebbe azzerare la parte frazionaria
				if ((yawF - yawP < 1.0) && (yawF - yawP > -1.0)){
					/// dopo 5 secondi non ho avuto variazioni significative e quindi ho integrato l'errore del
					/// sensore
					yawF = yawP;
					PRINTF("\nazzeramento\n");
				}
				else{
					tmp = (int16_t)yawF;
					yawP = (float) tmp;
					PRINTF("\noffset\n");
					azzeraAssi();
				}
				tempoDiReset = 0;
			}
			/// riporta il valore ad intero
			yaw = (int16_t) yawF;
		}

		if((asseOn & ALL_AXIS) == ALL_AXIS){
			if(i2cPtr == NULL)
				I2CReceiveN(GYRO_ADDR, OUT_X_L | MUL_READ , 6, buffer);
			else
				i2cPtr->I2CGetN(OUT_X_L | MUL_READ , 6, buffer);
			x = (int16_t)((buffer[1]<< 8) + buffer[0]) - x0;
			y = (int)((buffer[3]<< 8) + buffer[2]) -  y0;
			z = (int)((buffer[5]<< 8) + buffer[4]) -  z0;
			/// integrazione rettangolare: valore letto * fondo scala * intervallo di tempo di integrazione
			/// posto a 10ms
			f = z * DPS;
			yawF = f;
			f *=  tick;
			/// riporta il valore ad intero
			yaw += (int16_t) f;
			f = y * DPS;
			rollF = f;
			f *=  tick;
			/// riporta il valore ad intero
			roll += (int16_t)f;
			f = x * DPS;
			pitchF = f;
			f *=  tick;
			/// riporta il valore ad intero
			pitch += (int16_t) f;
		//valore = readI2CByteFromAddress(STATUS_REG, &stato);

		/*PRINTF("asse x: %d\t",x);
		PRINTF("\tasse y: %d\t",y);
		PRINTF("\tasse z: %i\r\n", z);*/
		}
	}
	else{
		/// integra dal valore precedente con intervallo di tempo di 10ms
		f =  yawF *  tick;
		 yaw += (uint16_t) f;
		if (( asseOn & 0x3) != 0){
			/// occorre integrare anche sugli assi X e Y
			f =  rollF *  tick;
			roll += (uint16_t) f;
			f =  pitchF *  tick;
			pitch += (uint16_t) f;
		}
	}
}

///
/// restituisce la temepratura del sensore
int Giroscopio::getTemp(){

	int8_t ris;
	if (i2cPtr == NULL)
		ris = I2CReceive(GYRO_ADDR, OUT_TEMP);
	else
		i2cPtr->I2CGet(OUT_TEMP);
	/// la temperatura dovrebbe avere 25° come punto unito
	/// cioe' 25 gradi = 0x19 nel sensore e poi
	/// la retta ha t = -1 * x + 45  cioe':
	/// t   |   x
	/// ----|-----
	/// 20  |  25
	/// 25  |  20
	/// 45  |   0
	/// 55  | -10
	/// ........
	temp = (int)  45 - ris;
	return temp;
}

extern volatile int procCom;
///
/// primo azzeramento degli assi del giroscopio
void Giroscopio::primoAzzeramento(){
	int blink = 0;
	if (IsPresent == OK){
		PRINTF("\nAzzeramento assi giroscopio\n");
		while (blink < 70){
			if (procCom == 1){
				procCom = 0;
				blink++;
			}
		}

		/// azzeramento degli assi
		//azzeraAssi(&G);
		azzeraAssi();
		PRINTF("media:\t ");
		printFloat(media, 4);
		PRINTF("\nm:\t ");
		printFloat(m, 4);
		PRINTF("\n  q:\t ");
		printFloat(q, 4);
		PRINTF("\n");
	}
}


///
/// collega la porta I2C
void Giroscopio::attachI2C(I2C * p, uint8_t sa){
	i2cPtr = p;
	i2cPtr->I2CSetSlave_Add(sa);
}


