/*
 * SolarTracker.h
 *
 *  Created on: Nov 27, 2017
 *      Author: xtarke
 */

#ifndef SOLARTRACKER_H_
#define SOLARTRACKER_H_

#include "GPS.h"
#include "Magnetometer.h"
#include "MqttComm.h"
#include "PRU.h"

#include "spa/spa.h"

#include <thread>
#include <mutex>

class SolarTracker {

private:

	GPS *serialGPS;
	Magnetometer *magSensor;
	MqttComm *myComm;

	PRU *realTimeHardware;

	/* Threads */
	std::thread *gpsComThread;
	void GPSComThreadFunction();

	std::thread *MagComThread;
	void MagComThreadFunction();

	std::thread *inputOutputThread;
	void inputOutputFunction();

	std::thread *mqqtPublishThread;
	void mqttPublishFunction();

	std::thread *mqqtCommandsThread;
	void mqttCommandsFunction();

	void SPACalculation(int mode);
	void getSolarNoonZeAngle();

	enum solar_cmds {
		SOLAR_RUNNING = 0,
		SOLAR_MANUAL = 1,
		SOLAR_EXIT = 2
	};

	enum spa_mode{
		GPS_ONLINE = 0,
		LOCAL_PARAM = 1
	};

	double longitude;
	double latitude;
	double elevation;

	std::mutex inputOutputMutex;

	/* Read/Write in this structure should be protected with a mutex */
	struct solarStatus_t {
		int cmd;
		spa_data spa;
		int GPSstatus;

		/* Zenith position */
		int currentZePulsePos;
		/* elevationNormalized is between 0 and 180 degress *
		 * Info: Lego model
		 * if morning: [0,90] if afternoon: [90,180]	 **/
		double elevationNormalized;
		double deltaZenith;
		double minZenith;

		/* Azimuth position */
		int currentAzPulsePos;
		/* azimuthNormalized is betwwn -180 and 180 degress *
		 * [-180,0] -> Morning *
		 * [0,+180] -> Afternoon */
		double azimuthNormalized;

	} solarStatus;

	void readLocConfFile();
	int writeLocConfFile();

	void azRepos();
	void azGoHome();
	void zeRepos();
	void zeGoHome();

	void azManualPos(int pulses);
	void zeManualPos(int pulses);

	void zeSetHomePos(int pulses);
	void azSetHomePos(int pulses);

	int checkSunRiseSunSet();

	enum daynight {
		SUN_OK	= 0,
		BEFORE_SUNRISE = -1,
		AFTER_SUNSET = -2
	};

	enum hardwareLimits {
		AZ_MAX_PULSES = 950,
		ZE_MAX_PULSES = 3200
	};


	std::vector<std::string> split(const std::string& text, const std::string& delims)
	{
	    std::vector<std::string> tokens;
	    std::size_t start = text.find_first_not_of(delims), end = 0;

	    while((end = text.find_first_of(delims, start)) != std::string::npos)
	    {
	        tokens.push_back(text.substr(start, end - start));
	        start = text.find_first_not_of(delims, end);
	    }
	    if(start != std::string::npos)
	        tokens.push_back(text.substr(start));

	    return tokens;
	}

public:
	SolarTracker(const char* GPSdevFilename);
	virtual ~SolarTracker();
};

#endif /* SOLARTRACKER_H_ */
