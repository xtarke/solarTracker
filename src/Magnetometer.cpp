/*
 * Magnetometer.cpp
 *
 *  Created on: Jan 17, 2018
 *      Author: Renan Augusto Starke
 */

#include "Magnetometer.h"
#include <iostream>
#include <math.h>

using namespace GeographicLib;

Magnetometer::Magnetometer(float lat, float lon, float alt) {

	int ret, localStatus = MAG_SUCCESS;

	i2cDev = new i2cModule();

	ret = i2cDev->openDev(HMC5883L_i2c_ADDR);

	if (ret != 0){
		std::cerr << "Something wrong with i2c Bus\n";
	}
	else{
		/* Prepare magnetometer: continuous mode */
		uint8_t cmds[] = {MODE_REG, CONTINUOS_MODE};
		ret = i2cDev->sendDev(cmds,sizeof(cmds));

		if (ret != sizeof(cmds)){
			std::cerr << "Something wrong with i2c Bus\n";
			localStatus = MAG_FAILURE;
		}
	}

	/* Initialize local axis values */
	x = 0;
	y = 0;
	z = 0;

	/* Current Status */
	status = localStatus;
	kalman_state.init = false;
	filtered_bearing = 0;

	updateMagneticDeclination(lat, lon, alt);
}

void Magnetometer::updateMagneticDeclination(float lat, float lon, float alt){

	/* intermediate Mag model values */
	double Bx, By, Bz, H;

	/* Determine current time. */
	time_t t = time(NULL);
	tm* timePtr = localtime(&t);

	MagneticModel mag("emm2015");

		/* Get intermediate values using current time and position */
	mag(timePtr->tm_year + 1900, lat, lon, alt, Bx, By, Bz);

	/* Convert intermediate values into field components and store in class members */
	MagneticModel::FieldComponents(Bx, By, Bz, H,
		field_strength, magnetic_declination, magnetic_inclination);

}


int Magnetometer::refresh(){

	double angle, angleDeg;
	uint8_t data[6] = {0,0,0,0,0,0};


	if (status == MAG_SUCCESS){
		/* Send initial data register address */
		data[0] = FIRST_DATA_OUTPUT_REG;
		i2cDev->sendDev(data,1);

		/* Read axis */
		i2cDev->readDev(data,6);

#ifdef DEBUG
		for (int j=0; j < 6; j++)
				printf("%d -> %x\n", j, data[j]);
#endif

		x = (data[0] << 8) | data[1];
		z = (data[2] << 8) | data[3];
		y = (data[4] << 8) | data[5];

#ifdef DEBUG
		std::cout << "-------------" << std::endl;
		printf("x= %d\n", x);
		printf("y= %d\n", y);
		printf("z= %d\n", z);
#endif
		angle = atan2 (y,x);

	/*	if(angle < 0)
		   angle +=  2 *M_PI;
		if(angle > 2 * M_PI)
		   angle -= 2*M_PI;*/

		angleDeg = angle * 180/M_PI;

		if (kalman_state.init == false)
			kalman_init(0.025f, 16, 1, angleDeg);

		/* Update Filtered Bearing */
		kalman_update(angleDeg);

		/* Store Filtered Bearing */
		filtered_bearing = kalman_state.x;


		std::cout << "Mag: " << magnetic_inclination << " ";
		std::cout << "Angle: " << filtered_bearing + magnetic_inclination << " (" << angleDeg << ")"<< std::endl;
	}

	return MAG_SUCCESS;
}

void Magnetometer::kalman_init(float q, float r, float p, float x) {
	kalman_state.q = q;
	kalman_state.r = r;
	kalman_state.p = p;
	kalman_state.x = x;

	kalman_state.init = true;
}

void Magnetometer::kalman_update(float m) {
	//prediction update
	//omit x = x
	kalman_state.p = kalman_state.p + kalman_state.q;

	//measurement update
	kalman_state.k = kalman_state.p / (kalman_state.p + kalman_state.r);
	kalman_state.x = kalman_state.x + kalman_state.k * (m - kalman_state.x);
	kalman_state.p = (1 - kalman_state.k) * kalman_state.p;
}


Magnetometer::~Magnetometer() {
	delete i2cDev;
}

