/*
 * distMis.h
 *
 *  Created on: 21/feb/2016
 *      Author: itismattei
 */

#ifndef DISTMIS_H_
#define DISTMIS_H_

#include <stdbool.h>
#include <stdint.h>

class distMis {
public:
	distMis();
	virtual ~distMis();
	void rawTomm1(void);

	void rawTomm(void);

	/// proprieta'
	float 		misSens[6];		/// distanza in millimetri con virgola
	/// NOTA BENE: siccome il 6° sensore di distanza non è presente si ura tale ingresso per
	///            la misura della batteria.
	float 		kf;				/// coefficiente di trasformazione tensione - distanza
	int 		d_mm[6];		/// distanza in millimetri (intera)
	uint32_t	dI[8];
	bool		run;			/// indica se il dispositivo funziona
	int vel;

};

#endif /* DISTMIS_H_ */
