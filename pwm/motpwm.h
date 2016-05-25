/*
 * motpwm.h
 *
 *  Created on: 10/apr/2016
 *      Author: itismattei
 */

#ifndef MOTPWM_H_
#define MOTPWM_H_

#include "stdint.h"

class PWM_MOTORI{

public:
	/// static properties
	static int numPwm;

public:
	PWM_MOTORI(){numPwm ++; direction = 1; numMot = numPwm;}
	~PWM_MOTORI(){if (numPwm > 0) numPwm --;}

	void Init();
	void pwmPower();
	void MotorGo();
	void MotorStop();
	inline void setDir(int dir){ direction = dir;}

	///properties
	int NCont;
	int delta;
	/// 1 avanti; -1 indietro
	int direction;
	uint32_t numPin;
	uint32_t numMot;
};


///
///
/// classe che gestisce i due motori servi
/// vengono posti sui pin PD0 e PD1, col generatore PWM n. 1 con periodo di 20ms
class PWM_SERVI {

public:
	/// static properties
	static int numServi;


public:
	PWM_SERVI(){numServi++; numS = numServi;}
	~PWM_SERVI(){if (numServi > 0) numServi--;}
	void Init();
	void MotorGo(int16_t);
	void MotorStop(void);
	void scarico(void);

	uint32_t convertDeg2Pwm(int16_t gradi);
	///properties
	int NCont;
	int delta;
	uint32_t numS;
	uint32_t numPin;
};


#endif /* MOTPWM_H_ */
