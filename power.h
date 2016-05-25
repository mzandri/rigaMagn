/*
 * power.h
 *
 *  Created on: 23/apr/2016
 *      Author: massimo
 */

#ifndef POWER_H_
#define POWER_H_

class power {
public:
	power();
	power(int safeLev) { safeLevel = safeLev; }
	virtual ~power();

	int battLevel;
	int safeLevel;
};

#endif /* POWER_H_ */
