/*
 * SolarTracker.h
 *
 *  Created on: Nov 27, 2017
 *      Author: xtarke
 */

#ifndef SOLARTRACKER_H_
#define SOLARTRACKER_H_

#include "GPS.h"
#include <thread>

#include "spa/spa.h"

class SolarTracker {

private:

	GPS *serialGPS;

	/* Threads */
	std::thread *gpsComThread;
	void GPSComThreadFunction();

	void SPACalculationThreadFunction();

	spa_data spa;

public:
	SolarTracker(const char* GPSdevFilename);
	virtual ~SolarTracker();
};

#endif /* SOLARTRACKER_H_ */
