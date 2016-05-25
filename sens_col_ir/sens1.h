/*
 * sens1.h
 *
 *  Created on: 20/apr/2015
 *      Author: massimo
 */

#ifndef SENS1_H_
#define SENS1_H_

#include "../I2C/i2cTiva.h"
#include <stdint.h>

#define			IS_WALL				0x80
#define			IS_DARK_TILE		0x08
#define			IS_SURVIVOR			0x40
#define			NO_DARK_TILE		0x04
#define			NO_SURVIVOR			0x20
#define			GET_WALL			0x81
#define			GET_DARK_TILE		0x09
#define			GET_SURVIVOR		0x41



class TEMPER{
public:

	TEMPER(){ok = 0; isSurvivor = NO_SURVIVOR;}
	~TEMPER(){;}
	float readTemp();
	float getTemp(){ return Tint; }
	void taraturaTemp();
	void attachI2C(I2C *, uint8_t sa);

	float 		Temp;			/// lettura dal sensore in °C
	int 		tempRaw;		/// lettura dal sensore
	float 		T_tar;			/// temperatura dell'ambiente letta dal pirometro
	int 		Tcase;			///	temperatura del corpo del pirometro
	int 		Tint;			/// lettura del sensore in °C come intero.
	int			ok;				/// pirometro tarato
	uint16_t 	isSurvivor;		/// ha individuato un corpo caldo
	I2C* 		i2cPtr;			/// puntatore al canale di trasmissione I2C

};


#define		IS_DARK		1
#define		ISNT_DARK	0

void initLightSens1(void);
int readCol(void);

/// indica le mattonelle scure da evitare
class TILE{
public:
	TILE(){;}

	uint32_t isDark;
};


class COLORE{
public:
	COLORE();

	inline void Init(){ initLightSens1(); initTimer4(10);}
	/// legge il valore di luminanza
	inline int read(){ return readCol() ; }
	/// legge il valore di luminanza
	inline int get(void){ return luminanza; }
	/// restituisce il valore del bianco
	inline int getWhite(void){ return bianco; }
	inline void set(int v){ luminanza = v; }
	//Imposta il livello di bianco, ipotizzando che all'inizio la paistrella sia bianca.
	inline void WhiteBalance(){ bianco = read(); }

	void Run();

	/// proprieta'
	int 	rosso;
	int 	verde;
	int 	blu;
	int 	luminanza;
	int 	bianco;
	TILE	piastra;
};



#endif /* SENS1_H_ */
