/*
 * sens.h
 *
 *  Created on: 30/mar/2014
 *      Author: robotis
 */

#ifndef SENS_H_
#define SENS_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"


/*  contiene le impostazioni per la gestione del sensore di colore: sia hw (pin) sia sw */


/// struttura dati del sensore di colore

typedef struct _colore{
	int rosso;
	int verde;
	int blu;
	int luminanza;
	int bianco;
} colore;



typedef struct _survivor{
	///contatore dei sopravvissuti trovati
	uint8_t isSurvivor;
} survivor;

typedef struct _temp{
	float 	Temp;
	int 	tempRaw;
	int 	T_tar;
	int 	Tcase;
	int		ok;
} temperatura;



/////
#define			COLOR_ADDR			0x29
#define			DEVICE_ID			0x12
#define			TEMP_ADDR			0x00
#define			TEMP_REG			0x07

#ifdef __cplusplus
extern "C" {
#endif


void readColourSens(colore *);
void readTempSens(temperatura *tPtr);
void taraturaTemp(temperatura *tempPtr);
void readTemp(temperatura *tempPtr);


void initLightSens(void);
void initTimer4(uint8_t);
void whiteBal(colore *);

#ifdef __cplusplus
}
#endif

#define 	SOGLIAFERITO 			600
#define		SOGLIA_DARK_TILE		-100

///
/// messaggi
#define			IS_WALL				0x80
#define			IS_DARK_TILE		0x08
#define			IS_SURVIOR			0x40
#define			NO_DARK_TILE		0x04
#define			NO_SURVIVOR			0x20
#define			GET_WALL			0x81
#define			GET_DARK_TILE		0x09
#define			GET_SURVIVOR		0x41



#endif /* SENS_H_ */
