/*
 * SolarTracker.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: xtarke
 */

#include "SolarTracker.h"

/* Azimuth is measured from the north point (sometimes from the south point)
 *  of the horizon around to the east; altitude is the angle above the horizon.*/


void SolarTracker::GPSComThreadFunction(){
	//while (1)

	for (int i=0; i < 10; i++){
		serialGPS->ReadandParse();
		//serialGPS->printNumericalData();
		SPACalculationThreadFunction();

		std::cout << "-------------------------\n";
	}
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


	serialGPS = new GPS(GPSdevFilename);

	gpsComThread = new std::thread(&SolarTracker::GPSComThreadFunction, this);


}

SolarTracker::~SolarTracker() {

	gpsComThread->join();

	delete serialGPS;

}

