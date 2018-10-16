/*
 * SolarPulses.cpp
 *
 *  Created on: Oct 10, 2018
 *      Author: xtarke
 */

#include "SolarPulses.hpp"
#include "spa/spa.h"
#include <time.h>
#include <iostream>

SolarPulses::SolarPulses() {
	// TODO Auto-generated constructor stub

}

SolarPulses::~SolarPulses() {
	// TODO Auto-generated destructor stub
}

void SolarPulses::generateSolarTable(){
	int result;
	/* System Data */
	time_t t = time(0);
	struct tm * now = localtime(&t);

	spa_data spa;

	//enter required input values into SPA structure
	spa.year          = now->tm_year + 1900;
	spa.month         = now->tm_mon + 1;
	spa.day           = now->tm_mday;

	std::cout << spa.year << ":" << spa.month << ":" << spa.day << std::endl;


	spa.timezone      = -2.0;
	spa.delta_ut1     = 0;
	spa.delta_t       = 67;

	spa.longitude     = LATITUDE;
	spa.latitude      = LONGITUDE;
	spa.elevation     = ELEVATION;

	spa.pressure      = 820;
	spa.temperature   = 11;
	spa.slope         = 30;
	spa.azm_rotation  = -10;
	spa.atmos_refract = 0.5667;
	spa.function      = SPA_ZA;

	spa.hour          = 5;
	spa.second        = 0;

	std::cout << "time;zenith;azimuth" << std::endl;

	/* Iteration between 7h and 17h: localtime */
	for (int i=0; i < 700; i++){

		float min = (float)((int) i % 60);
		spa.minute        = (int)min;

		if (spa.minute == 0)
			spa.hour++;

		result = spa_calculate(&spa);

		if (result  != 0){
			std::cerr << "Error estimating solar angles (spa error code): " << result << std::endl;
			exit(-1);
		}

		double azimuthNormalized = (spa.azimuth > 180) ? (360.0 - spa.azimuth) : (-spa.azimuth);
		azimuthNormalized += 180;

		double elevationNormalized = 90 - spa.zenith;

		int zepulses = elevationNormalized / PULSES_PER_STEP;
		int azimuthPulses = azimuthNormalized / PULSES_PER_STEP;

		std::cout << spa.hour << ":" << spa.minute;
		std::cout << ";" << spa.zenith << ";" << spa.azimuth << ";" << zepulses << ";" << azimuthPulses << std::endl;
	}
}

