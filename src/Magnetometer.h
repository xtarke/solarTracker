/*
 * Magnetometer.h
 *
 *  Created on: Jan 17, 2018
 *      Author: xtarke
 */

#ifndef MAGNETOMETER_H_
#define MAGNETOMETER_H_

//#include <GeographicLib/MagneticModel.hpp>
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

	/* See: https://www.codeproject.com/Articles/1112064/Calculating-True-North-for-IoT-Applications */
	struct kalman_t {
		float q; //process noise covariance
		float r; //measurement noise covariance
		float x; //value
		float p; //estimation error covariance
		float k; //kalman gain

		bool init;

	} kalman_state;

    /*
	 * Magnetic Model component, Using emm2015 magnetic model
	 * (emm2015 includes magnetic interactions from Earth's crust)
	 */
	double magnetic_declination;
	double magnetic_inclination;
	double field_strength;

	float filtered_bearing;

	void kalman_init(float q, float r, float p, float x);
	void kalman_update(float m);

	/* Magnetic declination: -19° 22' in Florianópolis */
	const float magDec =  -0.338;

public:
	Magnetometer(float lat, float lon, float alt);
	virtual ~Magnetometer();

	void updateMagneticDeclination(float lat, float lon, float alt);
	int refresh(void);
};

#endif /* MAGNETOMETER_H_ */
