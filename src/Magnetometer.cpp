/*
 * Magnetometer.cpp
 *
 *  Created on: Jan 17, 2018
 *      Author: Renan Augusto Starke
 */

#include "Magnetometer.h"
#include <iostream>
#include <math.h>

Magnetometer::Magnetometer() {

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
}

int Magnetometer::refresh(){

	double angle;
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
		y = (data[2] << 8) | data[3];
		z = (data[4] << 8) | data[5];

//#ifdef DEBUG
		std::cout << "-------------" << std::endl;
		printf("x= %d\n", x);
		printf("y= %d\n", y);
		printf("z= %d\n", z);
//#endif
		angle = atan2 (z,x) * 180 / M_PI;

		std::cout << "Angle: " << angle << std::endl;

	}

	return MAG_SUCCESS;
}


Magnetometer::~Magnetometer() {
	delete i2cDev;
}

