/*
 * Magnetometer.h
 *
 *  Created on: Jan 17, 2018
 *      Author: xtarke
 */

#ifndef MAGNETOMETER_H_
#define MAGNETOMETER_H_

#include "i2cmodule.h"

class Magnetometer {
	i2cModule *i2cDev;

	int16_t x;
	int16_t y;
	int16_t z;

	int status;

	enum {HMC5883L_i2c_ADDR = 0x1e, MODE_REG = 0x02, CONTINUOS_MODE = 0x00, FIRST_DATA_OUTPUT_REG = 0x03};

	enum gps_return_code {
		MAG_FAILURE = -1,
		MAG_SUCCESS = 0
	};

	/* Magnetic declination: -19° 22' in Florianópolis */
	const float magDec =  -0.338;

public:
	Magnetometer();
	virtual ~Magnetometer();

	int refresh();
};

#endif /* MAGNETOMETER_H_ */
