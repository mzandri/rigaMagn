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
#include "pid.h"

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
	/// Lo stato pu� quindi diventare NON_VALIDO solo a seguito di time out o perche' gia' lo era.
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


///
/// analizza il comando che e' arrivato
///
void parse(syn_stat *STATO, comando *cmdPtr, syntaxStatus *synPtr){


	STATO->cmd[STATO->ST] = uart1buffer[READ_PTR1];

	/// La ricezione di un comando errato non produce il cambio di stato del mezzo.
	/// Infatti STATO->valid cambia a seguito di un comando corretto in questa funzione ma NON a seguito di un comando errato
	/// Lo stato pu� quindi diventare NON_VALIDO solo a seguito di time out o perche' gia' lo era.
	switch(STATO->ST){
	case 0:
		STATO->check = 0;
		if (STATO->cmd[0] >64 && STATO->cmd[0] < 91 ){
			/// una lettera MAIUSCOLA e quindi un comando di azione da raspberry
			STATO->l_cmd = 4;
			STATO->ST = 1;
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
			convertToToken(STATO, cmdPtr);
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


///
/// converte l'indicatore di un comando in un token
void convertToToken(syn_stat *STATO, comando *cmdPtr){

	switch(STATO->cmd[0]){
	case 'F':
		STATO->token = AVANTI;
		cmdPtr->azione = true;
		cmdPtr->numPid = 0;
	break;
	case 'B':
		STATO->token = INDIETRO;
		cmdPtr->azione = true;
		cmdPtr->numPid = 0;
	break;
	case 'S':
		STATO->token = STOP;
		cmdPtr->azione = true;
		cmdPtr->numPid = -1;
	break;
	case 'R':
		STATO->token = DESTRA;
		cmdPtr->azione = true;
		cmdPtr->numPid = 1;
		cmdPtr->valFin -= 90;
	break;
	case 'L':
		STATO->token = SINISTRA;
		cmdPtr->azione = true;
		cmdPtr->numPid = 2;
		cmdPtr->valFin += 90;
	break;
	case 'I':
		STATO->token = GIRA_INDIETRO;
		cmdPtr->azione = true;
		/// potrebbe essere un "DESTRA" con valFIn = -180
	break;
	case 'G':
		//// non ancora implementato
		STATO->token = MISURA_GRADI;
		cmdPtr->azione = false;
	break;
	case 'D':
		/// lettura sensore
		STATO->token = LETTURA_SENSORE;
		cmdPtr->azione = false;
	break;
	case 'P':
		//rilascio rescue pack
		STATO->token = RILASCIO_PACK;
		cmdPtr->azione = true;
		cmdPtr->avvia = true;
	default:
		/// se nessun comando e'giusto produce un errore.
		STATO->token = ERRORE;
		cmdPtr->numPid = -1;
	break;
	}

	/// potrebbe essere un comando nnuerico di richiesta dati
	/*if (STATO->cmd[0] <= 16){
		STATO->token = LETTURA_DATI;
	}*/
	/// azzera il contatore della persistenza del comando
	/// cio' vuol di re che c'e' un nuovo comando da servire
	STATO->tick = 0;
	/// resetta il contatore di timeout per permettere al comando di essere eseguito per almeno
	/// 1.5 s
	cmdPtr->tick = 0;

}


void rispondiComando(syn_stat *sSTAT, glb *colletedD){
	/// controlla se la sintazzi e' valida
	sSTAT->check = 0;
	/// controllo ridondante gia' effettuato
	if (sSTAT->valid == VALIDO){
		/// analizza il token e per il momento risposnde alle richieste di dati
		/// i tokens sono: MISURA_GRADI LETTURA_SENSORE (con nuero di sensore in sSTAT.cmd[1])
		switch(sSTAT->token){

//		case MISURA_GRADI:
//			sSTAT->buff_reply[0] = 'G';
//			sSTAT->buff_reply[1] = (data->gPtr->yaw  & 0xFF00) >> 8;
//			sSTAT->buff_reply[2] = data->gPtr->yaw  & 0x00FF;
//
//		break;

		/// fornisce la risposta alla lettura di un sensore
		case LETTURA_SENSORE:
			/// prepara solamente il buffer di risposta
			inviaSensore(sSTAT, colletedD);
		break;
		}

		/// se il dato e' valido viene trasmesso al richiedente (raspberry)
		if (sSTAT->dato_valido == 1){
			sSTAT->check  = 0;
		/// calcolo checksum
			for(int i = 0; i < 3; i++)
				/// calcola il checksum
				sSTAT->check ^= sSTAT->buff_reply[i];

			/// aggiunge la chiave 0xA9
			sSTAT->check ^= CHECK_SUM;
			sSTAT->buff_reply[3] = sSTAT->check;
			sSTAT->buff_reply[4] = '*';
		}
		else
			for (int  i = 0; i < 5; i++)
				sSTAT->buff_reply[i] = 0;

		/// invia i 4 byte su seriale. L'ultimo e' invato dalla funzione sendReply
		sendReply(sSTAT, 4);
	}
}

///
/// invia i bytes di risposta sulla uart 1
void sendReply(syn_stat *sSTAT, uint8_t numChar){
	uint8_t i;
	/// invia tutti i caratterei nella stringa
	for (i= 0; i < numChar; i++)
		UARTCharPut(UART1_BASE, sSTAT->buff_reply[i]);

	/// infine invia il terminatore di stringa '*'
	UARTCharPut(UART1_BASE, '*');
}

///
/// risponde alla richiesta di un sensore
void inviaSensore(syn_stat *sSTAT, glb * collectedD){

	//TODO: ho aggiunto il puntatore alla struttura del colore, va aggiunto anche quello della temperatura?
	uint8_t  datoValido = 1;
	//analizzo il secondo byte, che � quello contentente l'ID del sensore
	//se ci dovesse essere un sensore in pi� quello va come case (0)
	switch(sSTAT->cmd[1]){
		// i sensori vengono numerati da quello davanti in senso antiorario
		// la misura dfornita e' in millimetri
		// sensore di distanza DD1
		case(1):
			//risposta con ID del sensore
			sSTAT->buff_reply[0] = 1;
			sSTAT->buff_reply[1] = (collectedD->DSTptr->d_mm[0]  & 0xFF00) >> 8;
			sSTAT->buff_reply[2] = collectedD->DSTptr->d_mm[0]  & 0x00FF;

			break;

		//sensore di distanza DD2
		case(2):
			sSTAT->buff_reply[0] = 2;
			sSTAT->buff_reply[1] = (collectedD->DSTptr->d_mm[1]  & 0xFF00) >> 8;
			sSTAT->buff_reply[2] = collectedD->DSTptr->d_mm[1]  & 0x00FF;

			break;

		/// sensore di distanza anteriore
		case(3):
			sSTAT->buff_reply[0] = 3;
			sSTAT->buff_reply[1] = (collectedD->DSTptr->d_mm[2]  & 0xFF00) >> 8;
			sSTAT->buff_reply[2] = collectedD->DSTptr->d_mm[2]  & 0x00FF;

			break;

		case(4):
			sSTAT->buff_reply[0] = 4;
			sSTAT->buff_reply[1] = (collectedD->DSTptr->d_mm[3]  & 0xFF00) >> 8;
			sSTAT->buff_reply[2] = collectedD->DSTptr->d_mm[3]  & 0x00FF;

			break;

		case(5):
			sSTAT->buff_reply[0] = 5;
			sSTAT->buff_reply[1] = (collectedD->DSTptr->d_mm[4] & 0xFF00) >> 8;
			sSTAT->buff_reply[2] = collectedD->DSTptr->d_mm[4]  & 0x00FF;

			break;

		//se fosse presente anche il sensore nr 6
			//TODO: allargare il vettore d_mm di 1
//		case(0):
//			sSTAT->buff_reply[0] = 0;
//			sSTAT->buff_reply[1] = D->d_mm[5] ;
//			break;

		//lettura giroscopio (angolo tra 0 e 360�)
		case(6):
				sSTAT->buff_reply[0] = 6;
				sSTAT->buff_reply[1] = (collectedD->gyro->yaw & 0xFF00) >> 8;
				sSTAT->buff_reply[2] = collectedD->gyro->yaw & 0x00FF;
				break;

		//lettura luminosit� (valore tra 0 e 255)
		case(7):
					sSTAT->buff_reply[0] = 8;
					sSTAT->buff_reply[1] = (collectedD->colorClass->luminanza & 0xFF00) >> 8;
					sSTAT->buff_reply[2] = collectedD->colorClass->luminanza & 0x00FF;
				break;

		//lettura temperatura (valore tra 20 e 40)
		case(8):
					sSTAT->buff_reply[0] = 8;
					sSTAT->buff_reply[1] = ((int)collectedD->temperat->Tint & 0xFF00) >> 8;
					sSTAT->buff_reply[2] = (int)collectedD->temperat->Tint & 0x00FF;
				break;

		//velocit� (cm/s)
		case(9):
				sSTAT->buff_reply[0] = 9;
				//cast necessario, bisogna passare un intero
//				sSTAT->buff_reply[1] = ((int)collectedD->cinPtr->vel  & 0xFF00) >> 8;
//				sSTAT->buff_reply[2] = (int)collectedD->cinPtr->vel  & 0x00FF;
				sSTAT->buff_reply[1] = 1;
				sSTAT->buff_reply[2] = 0;
				break;

		//distanza percorsa (cm)
		case(10):
				sSTAT->buff_reply[0] = 10;
				//cast necessario, bisogna passare un intero
				sSTAT->buff_reply[1] = ((int)collectedD->encoder->dist_mm  & 0xFF00) >> 8;
				sSTAT->buff_reply[2] = (int)collectedD->encoder->dist_mm  & 0x00FF;
				break;

		/// inserire qui il codice per il senspre n. 11: il valore di angolo dell'accelerometro
		case(11):
				sSTAT->buff_reply[0] = 11;
				//cast necessario, bisogna passare un intero. Il valore restituito e' in
				// milli g cioe' in piano l'asse z da' 1.00xx
				// mentre in aInt[2] fornisce 1000
				//sSTAT->buff_reply[1] = ((int)collectedD->acc->aInt[2]  & 0xFF00) >> 8;
				//sSTAT->buff_reply[2] = (int)collectedD->acc->aInt[2]  & 0x00FF;
		break;

		default:
			sSTAT->buff_reply[0] = 0;
			sSTAT->buff_reply[1] = 0;
			sSTAT->buff_reply[2] = 0;
			datoValido = 0;
		break;
	}

	/// tiene nella struttura della sintassi se il dato richiesto e' valido oppure non lo e'
	if (datoValido == 1)
		sSTAT->dato_valido = 1;
	else
		sSTAT->dato_valido = 0;

}

///
/// invia la risposta, in comandi di tipo rotazione
//void rispostaRotazione(pid *pidPtr, syn_stat *synPtr){
//	uint8_t checksum = 0;
//	pidPtr->rispondi = FALSE;
//	synPtr->buff_reply[1] = '0';
//	synPtr->buff_reply[2] = TRUE;
//	int i;
//	for (i = 0 ; i < 3; i++)
//		checksum ^= synPtr->buff_reply[i];
//	checksum ^= CHECK_SUM;
//	synPtr->buff_reply[3] = checksum;
//	sendReply(synPtr, 4);
//}
