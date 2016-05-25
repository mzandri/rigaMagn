/*
 * Giroscopio.h
 *
 *  Created on: 02/mar/2016
 *      Author: massimo
 */

#ifndef GIROSCOPIO_H_
#define GIROSCOPIO_H_

#include <stdint.h>
#include "I2C/i2cTiva.h"

#define		maxDimBuff		256
#define		numSampleBias	128

class Giroscopio {
public:
	Giroscopio();
	virtual ~Giroscopio();

	void initGyro(char);
	void setupAssi(char stato);
	void misuraAngoli();
	void azzeraAssi(void);
	void printAsseZ(int);
	int getTemp();
	void primoAzzeramento(void);
	void attachI2C(I2C *, uint8_t sa);

	int16_t buffValori[maxDimBuff];
	int16_t buffX[maxDimBuff];
	int16_t tempReg;
	float  media;
	float m, q;


//private:
	char IsPresent;
	char IsOn;
	/// tiene il setup dell'asse
	int16_t x0, y0, z0;
	/// indica quali assi sono attivi
	char asseOn;
	/// valore degli angoli misurati da integrazione della rotazione
	/// i valori sono in gradi in modo da non dover memorizzare dei float
	int16_t yaw; 	/// azze z
	int16_t roll; 	/// asse y
	int16_t pitch;	/// asse x

	/// valori attuali degli assi
	float yawF, yawF0;
	float rollF;
	float pitchF;
	/// valori precedenti degli assi
	float yawP, rollP, pitchP;
	/// valori di gradi al secondo dedotti dal manuale del giroscopio
	int16_t gradiSec;
	/// fondo scala (°/s)
	float FS;
	/// intervallo di integrazione in ms
	float tick;
	/// fattori di scala per ciscun asse
	float kz, ky, kx;
	/// temepratura
	int temp;
	/// viene attiato dal pid durante la rotazione in modo da non resettare il giroscopio.
	char IsRotating;
	char offsetRequest;
private:
	uint16_t posizione;
	I2C *i2cPtr;

};

uint8_t printFloat(double number, uint8_t digits);

#endif /* GIROSCOPIO_H_ */
