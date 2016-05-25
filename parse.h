/*
 * parse.h
 *
 *  Created on: 07/mar/2016
 *      Author: itismattei
 */

#ifndef PARSE_H_
#define PARSE_H_

#include <stdint.h>
#include "pid.h"

#include "sens_col_ir/sens.h"
#include "qei.h"
#include "distMis.h"
#include "sens_col_ir/sens1.h"

#include "Giroscopio.h"
#include "encQuad.h"


typedef struct _glb{
	//gyro 		*gPtr;
	TEMPER			*temperat;
	temperatura 	*tempPtr;
	colore 			*colPtr;
	COLORE			*colorClass;
	//distanza 	*distPtr;
	//cinematica 		*cinPtr;
	qei 			*qeiPtr;
	survivor 		*surPtr;

	/* classe */
	distMis 		*DSTptr;
	Giroscopio  	*gyro;
	encQuad			*encoder;
	//accelerometro 	*acc;
} glb;

class syntaxStatus{
public:
	syntaxStatus(){valore = 0; tick = 0;}

	int valore;
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
};



#ifdef __cplusplus
extern "C" {
#endif

void parse(syn_stat *,  comando *, syntaxStatus *);
void convertToToken(syn_stat *, comando * );
/// reset dell'automa (o inizializzazione)
void resetAutoma(syn_stat * STATO);
void initModule();


void sendReply(syn_stat *, uint8_t numChar);

#ifdef __cplusplus
}
#endif

void rispondiComando(syn_stat *sSTAT, glb *);
/// invia la lettura di un sensore
void inviaSensore(syn_stat *,  glb*);

//void dati_a_struttura(gyro *, distanza *, cinematica *, colore *, temperatura* ,survivor *, dati *);
//void datiRaccolti(accelerometro *ACC, encQuad *ENC, TEMPER *TEMP, COLORE *CL, survivor *SUR, distMis *DIS, Giroscopio *GYRO, glb *GLB );

//pid * leggiComando(syn_stat *sSTAT, pid CTRL[], pid *p, dati *);
void EseguiPID(syn_stat *sSTAT, ControlloPID *);

#endif /* PARSE_H_ */
