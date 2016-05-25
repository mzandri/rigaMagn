/*
 * init.h
 *
 *  Created on: 05/mar/2015
 *      Author: robtics
 */

#include <stdint.h>
//#include "pid.h"
//#include "adc/adc.h"
#include "sens_col_ir/sens.h"
//#include "qei.h"
#include "gyro_f.h"


#ifndef INIT_H_
#define INIT_H_


#define 	DIM_READ_BUFF		64
#define		CHECK_SUM			0xA9

//#############
//valore dei tokens

#define		ERRORE				255
#define		NON_VALIDO			254
#define		VALIDO				127
#define		AVANTI				1
#define		INDIETRO			2
#define		STOP				0
#define		DESTRA				3		//(+90°)
#define		SINISTRA			4		//(-90°)
#define		GIRA_INDIETRO		5		//(180°)
#define		GIRA_GRADI			6
#define 	LETTURA_SENSORE		7
#define 	RILASCIO_PACK 		8
#define		MISURA_GRADI		10

#define 	TIMEOUT_CMD			150		/// 1.5 s con tic di 10 ms

#define		TRUE				'1'
#define		FALSE				'0'

#define		INT_STEP_10_MS		10




///
/// struttura che analizza la frase e ne mantiene il token del comando
typedef struct _syn_stat{
	int ST;
	uint8_t 	cmd[4];			/// lunghezza del comando fissa: 4 bytes
	uint8_t 	l_cmd;			/// lunghezza in bytes del comando
	uint8_t		check;			/// checksum del comando
	uint8_t 	token;			/// valore numerico del comando
	uint8_t 	tick;			/// tempo di persistenza del comando
	uint16_t	valid;			/// stato di validita' del comando
	uint8_t		buff_reply[8];	/// buffer della risposta
	uint8_t		dato_valido;	/// validita' della risposta del sensore
	uint8_t		suspend_reply;	/// usata per i comandi che necessitano di risposta alla fine
} syn_stat;

///
/// struttura che contiene i puntatori ai dati
typedef struct _dati{
	gyro 		*gPtr;
	temperatura *tempPtr;
	colore 		*colPtr;
	//distanza 	*distPtr;
	//cinematica 	*cinPtr;
	//qei 		*qeiPtr;
	survivor 	*surPtr;
} dati;

#ifdef __cplusplus
extern "C" {
#endif


void setupUART(int);
void PRINT_WELCOME(void);



#ifdef __cplusplus
}
#endif



#endif /* INIT_H_ */
