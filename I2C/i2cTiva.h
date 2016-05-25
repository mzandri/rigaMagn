

#ifndef I2C_TIVA_H_
#define I2C_TIVA_H_

#include <stdint.h>


#define		IMPOSTATA			1
#define		NON_IMPOSTATA		0

class I2C{
public:
	I2C(){stato = NON_IMPOSTATA;}
	I2C(uint32_t);

	void InitI2C(uint32_t);
	void I2CSetSlave_Add(uint8_t sa){SLAVE_ADD = sa;}
	uint32_t I2CGetN(uint8_t reg, uint8_t numElem, uint8_t buff[]);
	uint32_t I2CGet(uint8_t reg);
	void I2CPut(uint8_t num_of_args, ...);
	void I2CPut_String(char array[]);

	/// proprieta'
	uint32_t 	BASE_ADDR;
	uint8_t		SLAVE_ADD;
	uint16_t	stato;

};
#endif
