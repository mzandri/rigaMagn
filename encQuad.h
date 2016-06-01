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

#define		VERO		1
#define		FALSO		0
#define		MAX_NUM_PT	512

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
	void fixPos();

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
	int 		posV[MAX_NUM_PT];
	int			indice;
	int			contIDX;
	int			setIDX;
};

#endif /* ENCQUAD_H_ */
