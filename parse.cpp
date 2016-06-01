/*
 * parse.c
 *
 *  Created on: 05/mar/2015
 *      Author: robotics
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/uart.h"
#include "init.h"
#include "pid.h"

#include "sens_col_ir/sens.h"
#include "distMis.h"
#include "parse.h"
#include "uartp/uartstdio.h"
//#include "pid.h"
#include "encQuad.h"

extern volatile uint8_t uart1buffer[], RX_PTR1, READ_PTR1;
extern volatile uint8_t uart0buffer[], RX_PTR0, READ_PTR0;

void resetAutoma(syn_stat * STATO){
	STATO->ST = 0;
	STATO->cmd[0] = STATO->cmd[1] = 0;
	STATO->l_cmd = 0;
	STATO->valid = NON_VALIDO;
}


///
/// analizza il comando che e' arrivato
///
void PARSE(syn_stat *STATO){


	STATO->cmd[STATO->ST] = uart0buffer[READ_PTR0];

	/// La ricezione di un comando errato non produce il cambio di stato del mezzo.
	/// Infatti STATO->valid cambia a seguito di un comando corretto in questa funzione ma NON a seguito di un comando errato
	/// Lo stato può quindi diventare NON_VALIDO solo a seguito di time out o perche' gia' lo era.
	switch(STATO->ST){
	case 0:
		STATO->check = 0;
		if (STATO->cmd[0] > 96 && STATO->cmd[0] < 123 ){
			/// una lettera minuscola e quindi un comando di azione da raspberry
			STATO->l_cmd = 4;
			if (STATO->cmd[STATO->ST] == 'd'){
				resetAutoma(STATO);
				STATO->valid = VALIDO;
			}
			if (STATO->cmd[STATO->ST] == 'a'){

				STATO->valid = VALIDO;
			}
			STATO->ST = 0;
			STATO->check = STATO->cmd[0];
		}
		/*else if(STATO->cmd[0] <= 16){
			/// comando di richiesta dati lungo due caratteri
			STATO->l_cmd = 2;
			STATO->ST = 1;
		}*/
	break;

	case 1:

		STATO->check ^= STATO->cmd[1];
		STATO->ST = 2;

	break;

	case 2:
		STATO->check ^= CHECK_SUM;
		if(STATO->check == STATO->cmd[2]){
			/// ok, il messaggio e' valido
			//convertToToken(STATO, cmdPtr);
			/// il comando e' ora valido
			STATO->valid = VALIDO;
			STATO->ST = 3;
		}
		else
			STATO->ST = 0;

	break;

	case 3:
		// resetta l'automa e rimette lo stato a NON_VALIDO
		/// l'invio del comando e' fatto di 4 bytes e quindi passa di qui quando e' arrivato il IV byte cioe' quello
		/// del terminatore
		resetAutoma(STATO);
	break;

	}
}


/////
///// analizza il comando che e' arrivato
/////
//void parse(syn_stat *STATO, comando *cmdPtr, syntaxStatus *synPtr){
//
//
//	STATO->cmd[STATO->ST] = uart1buffer[READ_PTR1];
//
//	/// La ricezione di un comando errato non produce il cambio di stato del mezzo.
//	/// Infatti STATO->valid cambia a seguito di un comando corretto in questa funzione ma NON a seguito di un comando errato
//	/// Lo stato può quindi diventare NON_VALIDO solo a seguito di time out o perche' gia' lo era.
//	switch(STATO->ST){
//	case 0:
//		STATO->check = 0;
//		if (STATO->cmd[0] >64 && STATO->cmd[0] < 91 ){
//			/// una lettera MAIUSCOLA e quindi un comando di azione da raspberry
//			STATO->l_cmd = 4;
//			STATO->ST = 1;
//			STATO->check = STATO->cmd[0];
//		}
//		/*else if(STATO->cmd[0] <= 16){
//			/// comando di richiesta dati lungo due caratteri
//			STATO->l_cmd = 2;
//			STATO->ST = 1;
//		}*/
//	break;
//
//	case 1:
//
//		STATO->check ^= STATO->cmd[1];
//		STATO->ST = 2;
//
//	break;
//
//	case 2:
//		STATO->check ^= CHECK_SUM;
//		if(STATO->check == STATO->cmd[2]){
//			/// ok, il messaggio e' valido
//			convertToToken(STATO, cmdPtr);
//			/// il comando e' ora valido
//			STATO->valid = VALIDO;
//			STATO->ST = 3;
//		}
//		else
//			STATO->ST = 0;
//
//	break;
//
//	case 3:
//		// resetta l'automa e rimette lo stato a NON_VALIDO
//		/// l'invio del comando e' fatto di 4 bytes e quindi passa di qui quando e' arrivato il IV byte cioe' quello
//		/// del terminatore
//		resetAutoma(STATO);
//	break;
//
//	}
//}

//
/////
///// converte l'indicatore di un comando in un token
//void convertToToken(syn_stat *STATO, comando *cmdPtr){
//
//	switch(STATO->cmd[0]){
//	case 'F':
//		STATO->token = AVANTI;
//		cmdPtr->azione = true;
//		cmdPtr->numPid = 0;
//	break;
//	case 'B':
//		STATO->token = INDIETRO;
//		cmdPtr->azione = true;
//		cmdPtr->numPid = 0;
//	break;
//	case 'S':
//		STATO->token = STOP;
//		cmdPtr->azione = true;
//		cmdPtr->numPid = -1;
//	break;
//	case 'R':
//		STATO->token = DESTRA;
//		cmdPtr->azione = true;
//		cmdPtr->numPid = 1;
//		cmdPtr->valFin -= 90;
//	break;
//	case 'L':
//		STATO->token = SINISTRA;
//		cmdPtr->azione = true;
//		cmdPtr->numPid = 2;
//		cmdPtr->valFin += 90;
//	break;
//	case 'I':
//		STATO->token = GIRA_INDIETRO;
//		cmdPtr->azione = true;
//		/// potrebbe essere un "DESTRA" con valFIn = -180
//	break;
//	case 'G':
//		//// non ancora implementato
//		STATO->token = MISURA_GRADI;
//		cmdPtr->azione = false;
//	break;
//	case 'D':
//		/// lettura sensore
//		STATO->token = LETTURA_SENSORE;
//		cmdPtr->azione = false;
//	break;
//	case 'P':
//		//rilascio rescue pack
//		STATO->token = RILASCIO_PACK;
//		cmdPtr->azione = true;
//		cmdPtr->avvia = true;
//	default:
//		/// se nessun comando e'giusto produce un errore.
//		STATO->token = ERRORE;
//		cmdPtr->numPid = -1;
//	break;
//	}
//
//	/// potrebbe essere un comando nnuerico di richiesta dati
//	/*if (STATO->cmd[0] <= 16){
//		STATO->token = LETTURA_DATI;
//	}*/
//	/// azzera il contatore della persistenza del comando
//	/// cio' vuol di re che c'e' un nuovo comando da servire
//	STATO->tick = 0;
//	/// resetta il contatore di timeout per permettere al comando di essere eseguito per almeno
//	/// 1.5 s
//	cmdPtr->tick = 0;
//
//}

