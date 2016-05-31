/*
 * encQuad.h
 *
 *  Created on: 23/feb/2016
 *      Author: itismattei
 */

#ifndef ENCQUAD_H_
#define ENCQUAD_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define		TRUE	1
#define		FALSE	0

class encQuad {
public:
	encQuad();
	~encQuad();
	void qeiInit();
	void readPos();
	void readVel();
	void readDir();
	void setAddr(uint32_t add){address = add;}
	void update();
	void intIDXEnable();
	void fixPos(){ posFix = pos; }

	/// prorpieta'
	int 		vel;
	int 		dir;
	int 		posIDX;
	/// posizione aggiornata dalla routine di interruzione
	int 		pos;
	/// posiizone fissata per la trasmissione
	int 		posFix;
	int 		vel_period;
	int 		fscala;
	int 		zero_pos;
	uint32_t 	address;
	/// distanza percorsa in mm
	int			dist_mm;
	int 		posV[64];
	int			indice;
	int			contIDX;
	int			setIDX;
};

#endif /* ENCQUAD_H_ */
