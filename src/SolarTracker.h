/*
 * SolarTracker.h
 *
 *  Created on: Nov 27, 2017
 *      Author: xtarke
 */

#ifndef SOLARTRACKER_H_
#define SOLARTRACKER_H_

#include "GPS.h"
#include "MqttComm.h"

#include "spa/spa.h"

#include <thread>
#include <mutex>

class SolarTracker {

private:

	GPS *serialGPS;
	MqttComm *myComm;

	/* Threads */
	std::thread *gpsComThread;
	void GPSComThreadFunction();

	void SPACalculationThreadFunction();

	std::thread *inputOutputThread;
	void inputOutputFunction();
	std::mutex inputOutputMutex;
	int cmd;

	enum solar_cmds {
		SOLAR_RUNNING = 0,
		SOLAR_EXIT = 1
	};


	spa_data spa;

public:
	SolarTracker(const char* GPSdevFilename);
	virtual ~SolarTracker();
};

#endif /* SOLARTRACKER_H_ */
