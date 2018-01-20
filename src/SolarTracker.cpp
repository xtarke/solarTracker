/*
 * SolarTracker.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: xtarke
 */

#include "SolarTracker.h"
#include "GPS.h"

/* Azimuth is measured from the north point (sometimes from the south point)
 *  of the horizon around to the east; altitude is the angle above the horizon.*/


void SolarTracker::GPSComThreadFunction(){
	//while (1)
	int ret, localCmd = SOLAR_RUNNING;
	char buffer[32];

	while (localCmd == SOLAR_RUNNING){
		ret = serialGPS->ReadandParse();
		//serialGPS->printNumericalData();

		if (ret == 0){
			SPACalculationThreadFunction();
			myComm->publish(NULL, "solar/gps/status", 7, "online",0 , false);

			/* Publish azimuth */
			std::snprintf(buffer, sizeof(buffer), "%g", spa.azimuth);
			myComm->publish(NULL, "solar/az", sizeof(buffer), buffer, 0 , false);
			/* Publish zenith */
			std::snprintf(buffer, sizeof(buffer), "%g", spa.zenith);
			myComm->publish(NULL, "solar/ze", sizeof(buffer), buffer, 0 , false);

			/* Publish longitude */
			std::snprintf(buffer, sizeof(buffer), "%g", spa.longitude);
			myComm->publish(NULL, "solar/gps/lon", sizeof(buffer), buffer, 0 , false);
			/* Publish latitute */
			std::snprintf(buffer, sizeof(buffer), "%g", spa.latitude);
			myComm->publish(NULL, "solar/gps/lat", sizeof(buffer), buffer, 0 , false);
			/* Publish latitute */
			std::snprintf(buffer, sizeof(buffer), "%g", spa.elevation);
			myComm->publish(NULL, "solar/gps/ele", sizeof(buffer), buffer, 0 , false);

		}
		else
			myComm->publish(NULL, "solar/gps/status", 8, "offline",0 , false);

		std::cout << "-------------------------\n";

		/* Read exit command */
		inputOutputMutex.lock();
		localCmd = cmd;
		inputOutputMutex.unlock();
	}
}

void SolarTracker::inputOutputFunction(){

	int localCmd = SOLAR_RUNNING;
	std::string trash;

	do {
		std::cout << "Command: (> 0 to exit)\n";
		std::cin >> trash;

		try {
			localCmd = std::stoi(trash);
		}catch(const std::exception& e){
			std::cerr << "Invalid input data\n";
			localCmd = 0;
		}
	} while (localCmd == SOLAR_RUNNING);

	/* Test exit command */
	inputOutputMutex.lock();
	cmd = localCmd;
	inputOutputMutex.unlock();
}


void SolarTracker::SPACalculationThreadFunction(){

	int result;
	float min, sec;

	/* System Data */
	time_t t = time(0);
	struct tm * now = localtime(&t);

	//enter required input values into SPA structure
	spa.year          = now->tm_year + 1900;
	spa.month         = now->tm_mon + 1;
	spa.day           = now->tm_mday;

	std::cout << "year: " <<  spa.year << std::endl;
	std::cout << "month: " << spa.month << std::endl;
	std::cout << "day: " << spa.day << std::endl;

	spa.hour          = serialGPS->get_hh() - 2;
	spa.minute        = serialGPS->get_mm();
	spa.second        = serialGPS->get_ss();

	std::cout << "hour: " <<  spa.hour << std::endl;
	std::cout << "minute: " << spa.minute << std::endl;
	std::cout << "second: " << spa.second << std::endl;

	spa.timezone      = -2.0;
	spa.delta_ut1     = 0;
	spa.delta_t       = 67;
	spa.longitude     = serialGPS->get_longitute();
	spa.latitude      = serialGPS->get_latitue();
	spa.elevation     = serialGPS->get_altitude();
	spa.pressure      = 820;
	spa.temperature   = 11;
	spa.slope         = 30;
	spa.azm_rotation  = -10;
	spa.atmos_refract = 0.5667;
	spa.function      = SPA_ALL;

	result = spa_calculate(&spa);

	if (result == 0)  //check for SPA errors
	{
		//display the results inside the SPA structure

		printf("Julian Day:    %.6f\n",spa.jd);
		printf("L:             %.6e degrees\n",spa.l);
		printf("B:             %.6e degrees\n",spa.b);
		printf("R:             %.6f AU\n",spa.r);
		printf("H:             %.6f degrees\n",spa.h);
		printf("Delta Psi:     %.6e degrees\n",spa.del_psi);
		printf("Delta Epsilon: %.6e degrees\n",spa.del_epsilon);
		printf("Epsilon:       %.6f degrees\n",spa.epsilon);
		printf("Zenith:        %.6f degrees\n",spa.zenith);
		printf("Azimuth:       %.6f degrees\n",spa.azimuth);
		printf("Incidence:     %.6f degrees\n",spa.incidence);

		min = 60.0*(spa.sunrise - (int)(spa.sunrise));
		sec = 60.0*(min - (int)min);
		printf("Sunrise:       %02d:%02d:%02d Local Time\n", (int)(spa.sunrise), (int)min, (int)sec);

		min = 60.0*(spa.sunset - (int)(spa.sunset));
		sec = 60.0*(min - (int)min);
		printf("Sunset:        %02d:%02d:%02d Local Time\n", (int)(spa.sunset), (int)min, (int)sec);

	} else printf("SPA Error Code: %d\n", result);


}


SolarTracker::SolarTracker(const char* GPSdevFilename) {

	cmd = SOLAR_RUNNING;

	serialGPS = new GPS(GPSdevFilename);
	myComm = new MqttComm("soltTracker", "192.168.6.1", 1883);

	gpsComThread = new std::thread(&SolarTracker::GPSComThreadFunction, this);
	inputOutputThread = new std::thread(&SolarTracker::inputOutputFunction, this);

}

SolarTracker::~SolarTracker() {

	gpsComThread->join();
	inputOutputThread->join();

	delete serialGPS;
	delete myComm;

}

