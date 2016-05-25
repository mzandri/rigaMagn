/*
 * pwm.h
 *
 *  Created on: 09/mar/2015
 *      Author: robotics
 */



#ifndef PWM_H_
#define PWM_H_



#define 	AVANTI_PWM	 	10
#define 	INDIETRO_PWM	20
#define 	FERMO_PWM		30

/*#define A 65	//avanti
#define I 73 	//indietro
#define S 83	//sinistra
#define D 68	//destra
#define F 70	//fermo
*/

typedef struct _pwm{

	uint32_t 	NCont;
	uint32_t 	delta_1;
	uint32_t 	delta_2;
	uint8_t  	dir_1;
	uint8_t 	dir_2;

} pwm;

#ifdef __cplusplus
extern "C" {
#endif

void pwmMotInit(pwm *);
//void pwm_power(int delta_1, int delta_2, pwm*);
void pwm_power(pwm *);

void pwm_dir(pwm *);
void pwmServoInit (pwm *p);
void pwmPowerA6(pwm *p, int16_t gradi);
void pwmPowerA7(pwm *p, int16_t gradi);

#ifdef __cplusplus
}
#endif

#endif /* PWM_H_ */
