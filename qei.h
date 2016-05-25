/*
 * qei.h
 *
 *  Created on: 27/mar/2015
 *      Author: Nico
 */

#ifndef QEI_H_
#define QEI_H_


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _qei{

	uint32_t 	pos_1; //posizione encoder 1
	uint32_t 	pos_2; //posizione encoder 2
	uint32_t 	vel_1; //velocità motore 1 (encoder 1)
	uint32_t  	vel_2; //velocità motore 2 (encoder 2)
	uint32_t	dir_1; //direzione motore 1
	uint32_t	dir_2; //direzione motore 2
	uint32_t 	fscala; //fondoscala posizione
	uint32_t 	vel_period; //periodo della velocità
	uint32_t	zero_pos; //posizione iniziale

} qei;

////////////////////////////////////////

//// cfr. l classe
//class QEI{
//public:
//	QEI(){;}
//	void qeiInit();
//	void qeiData();
//	void qeiReset();
//
//	uint32_t 	pos_1; //posizione encoder 1
//	uint32_t 	pos_2; //posizione encoder 2
//	uint32_t 	vel_1; //velocità motore 1 (encoder 1)
//	uint32_t  	vel_2; //velocità motore 2 (encoder 2)
//	uint32_t	dir_1; //direzione motore 1
//	uint32_t	dir_2; //direzione motore 2
//	uint32_t 	fscala; //fondoscala posizione
//	uint32_t 	vel_period; //periodo della velocità
//	uint32_t	zero_pos; //posizione iniziale
//
//};



void QEI0IntHandler();
void QEI1IntHandler();
void UnlockPD7();
void qei_init(qei *p);

void qei_test(qei *);
void qei_data(qei *);
void qei_reset(qei *);

#ifdef __cplusplus
}
#endif


#endif /* QEI_H_ */
