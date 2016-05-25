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

void resetAutoma(syn_stat * STATO){
	STATO->ST = 0;
	STATO->cmd[0] = STATO->cmd[1] = 0;
	STATO->l_cmd = 0;
	STATO->valid = NON_VALIDO;
}


///
/// analizza il comando che e' arrivato
///
void parse(syn_stat *STATO, comando *cmdPtr, syntaxStatus *synPtr){


	STATO->cmd[STATO->ST] = uart1buffer[READ_PTR1];

	/// La ricezione di un comando errato non produce il cambio di stato del mezzo.
	/// Infatti STATO->valid cambia a seguito di un comando corretto in questa funzione ma NON a seguito di un comando errato
	/// Lo stato può quindi diventare NON_VALIDO solo a seguito di time out o perche' gia' lo era.
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
		/// si analizza il checksum e poi si esegue il comando
		/*if (STATO->cmd[0] ^ CHECK_SUM == STATO->cmd[1]){
			// ok
			convertToToken(STATO);
			// resetta l'automa
			resetAutoma(STATO);
				if(STATO->token != ERRORE)
			/// il comando e' ora valido
			STATO->valid = VALIDO;
		}
*/
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


///
/// legge il comando e restituisce, quando il comando e' valido il puntatore al pid di interesse
/// in caso contrario il puntatore e' NULL oppure il valore che gia' aveva.
//pid * leggiComando(syn_stat *sSTAT, pid CTRL[], pid *p, dati *data){
//
//	uint8_t checksum = 0;
//	//pid *p = NULL;
//	/// controlla se ci sono caratteri da processare
//	if (RX_PTR1 != READ_PTR1){
//		/// e se si', li invia al parser, che restituisce in synSTATO il token del comando
//		//parse(sSTAT);
//		READ_PTR1++;
//		READ_PTR1 &= DIM_READ_BUFF - 1;
//	}
//	/// controlla il time out del comando e se scaduto si ferma
//	if (sSTAT->tick > TIMEOUT_CMD){
//		/// in caso di timeout nella persistenza del comando si deve fermare
//		/// quale era o erano i pid attivo/i?
//		sSTAT->token = STOP;
//		sSTAT->valid = NON_VALIDO;
//		p = NULL;
//		/// deve anche mettere i pid in stato disattivo (.attivo = false)
//	}
//	/// agggiorna il contatore di persistenza.
//	sSTAT->tick++;
//
//	/// dal token si deve estrarre il valore finale da inserire nel PID al prossimo ciclo  e restituire l'indirizzo del
//	/// del PID su cui si andra' ad integrare.
//	if (sSTAT->valid == VALIDO){
//
//		switch(sSTAT->token){
//		case AVANTI:
//			/// imposta la velocita'
//			CTRL[0].valFin = 50;		/// velocita' in cm/s
//			CTRL[0].attivo = TRUE;
//			p = &CTRL[0];
//			sSTAT->buff_reply[0] = 'F';
//		break;
//
//		case INDIETRO:
//			/// imposta la velocita'
//			CTRL[0].valFin = -50;		/// velocita' in cm/s
//			CTRL[0].attivo = TRUE;
//			p = &CTRL[0];
//			sSTAT->buff_reply[0] = 'B';
//		break;
//		//TODO ricordare di impostare la scelta tra ruota e ruota su asse
//		case DESTRA:
//			/// usa il PID ruota e non routa su asse
//			/// non e' detto che la scelta sia ottimale. Come faccio a scegliere tra le due????
//			/// forse è meglio ruotare sull'asse, gli informatici stavano considerando questo
//			/// imposta l'angolo
//			CTRL[1].valFin = 90;		/// angolo in gradi
//			CTRL[1].attivo = TRUE;
//			p = &CTRL[1];
//			sSTAT->buff_reply[0] = 'R';
//			/// necessita di risposta alla fine
//			sSTAT->suspend_reply = TRUE;
//		break;
//
//		case SINISTRA:
//			/// usa il PID ruota e non routa su asse
//			/// non e' detto che la scelta sia ottimale. Come faccio a scegleire tra le due????
//			/// imposta l'angolo
//			CTRL[1].valFin = -90;		/// angolo in gradi
//			CTRL[1].attivo = TRUE;
//			p = &CTRL[1];
//			sSTAT->buff_reply[0] = 'L';
//			/// necessita di risposta alla fine
//			sSTAT->suspend_reply = TRUE;
//		break;
//
//		case GIRA_INDIETRO:
//			/// usa il PID routa su asse
//			/// da che parte e' meglio ruotare? Orario-antiorario? Come faccio a scegleire tra le due????
//			/// imposta l'angolo
//			CTRL[2].valFin = -180;		/// angolo in gradi
//			CTRL[2].attivo = TRUE;
//			p = &CTRL[2];
//			sSTAT->buff_reply[0] = 'I';
//			/// necessita di risposta alla fine
//			sSTAT->suspend_reply = TRUE;
//		break;
//
//		case LETTURA_SENSORE:
//			PRINTF("Sta chiedendo dei dati\n");
//			//rispondiComando(sSTAT, data);
//		break;
//
//		case STOP:
//			sSTAT->buff_reply[0] = 'S';
//		default:
//			/// disattiva tutti i pid al valore attualmente calcolato
//			CTRL[0].attivo = FALSE;
//			CTRL[1].attivo = FALSE;
//			CTRL[2].attivo = FALSE;
//			/// se il puntatore restituito e' NULL allora vuol dire che si e' verificato un errore
//			/// ed i pid devono essere tutti fermati
//			p = NULL;
//
//		break;
//		}
//
//		if(sSTAT->buff_reply[0] != 'R' || sSTAT->buff_reply[0] != 'L' || sSTAT->buff_reply[0] != 'I'){
//			/// risponde solo ai comandi di avanzamento o stop, ma non a quelli di rotazione
//			if (sSTAT->buff_reply[0] > 16 ){
//				/// significa che e' un comando d'azione a cui non e' ancora stato risposto
//				/// e quindi conclude con comando valido
//				/// se fosse stato una richiesta di dati non sarebbe passato
//				/// qui
//				sSTAT->buff_reply[1] = '0';
//				sSTAT->buff_reply[2] = TRUE;
//				int i;
//				for (i = 0 ; i < 3; i++)
//					checksum ^= sSTAT->buff_reply[i];
//				checksum ^= CHECK_SUM;
//				sSTAT->buff_reply[3] = checksum;
//
//			}
//			sendReply(sSTAT, 4);
//		}
//	}
//
//	return p;
//}


///
/// fornisce dati dai sensori a seguito di richiesta


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
	//analizzo il secondo byte, che è quello contentente l'ID del sensore
	//se ci dovesse essere un sensore in più quello va come case (0)
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

		//lettura giroscopio (angolo tra 0 e 360°)
		case(6):
				sSTAT->buff_reply[0] = 6;
				sSTAT->buff_reply[1] = (collectedD->gyro->yaw & 0xFF00) >> 8;
				sSTAT->buff_reply[2] = collectedD->gyro->yaw & 0x00FF;
				break;

		//lettura luminosità (valore tra 0 e 255)
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

		//velocità (cm/s)
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
