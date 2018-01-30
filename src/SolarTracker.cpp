/*
 * SolarTracker.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: xtarke
 */

#include "SolarTracker.h"
#include "GPS.h"

#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <unistd.h>

// #define DEBUG

/* Azimuth is measured from the north point (sometimes from the south point)
 *  of the horizon around to the east; altitude is the angle above the horizon.*/


void SolarTracker::GPSComThreadFunction(){

	int ret, localCmd = SOLAR_RUNNING;
	int i = 0;

	while (localCmd == SOLAR_RUNNING){

		/* Read exit command */
		inputOutputMutex.lock();
		localCmd = cmd;
		inputOutputMutex.unlock();

		ret = serialGPS->ReadandParse();
		//serialGPS->printNumericalData();

		if (ret == 0){
			SPACalculationThreadFunction(GPS_ONLINE);
			myComm->publish(NULL, "solar/gps/status", 7, "online", 0 , false);

			/* Update location */
			longitude     = serialGPS->get_longitute();
			latitude      = serialGPS->get_latitue();
			elevation     = serialGPS->get_altitude();
		}
		else {
			std::cerr << "GPS offline, using local parameters" << std::endl;
			SPACalculationThreadFunction(LOCAL_PARAM);
			myComm->publish(NULL, "solar/gps/status", 8, "offline", 0 , false);
		}

		ret = checkSunRiseSunSet();

		if (ret < 0) {
			zeGoHome();
			continue;
		}

		/* Normalize angles: */
		azimuthNormalized = (spa.azimuth > 180) ? (360.0 - spa.azimuth) : (-spa.azimuth);

		if (azimuthNormalized < 0)
			elevationNormalized = spa.zenith - 90;
		else
			elevationNormalized = spa.zenith + 90;

		/* Update hardware position Zenith angle */
		zeRepos();

		/* Publish azimuth */
		std::string stringValue = std::to_string(spa.azimuth);
		myComm->publish(NULL, "solar/az", stringValue.length(), stringValue.c_str(), 0 , false);

		/* Publish zenith */
		stringValue = std::to_string(spa.zenith);
		myComm->publish(NULL, "solar/ze", stringValue.length(), stringValue.c_str(), 0 , false);

		/* Publish longitude */
		stringValue = std::to_string(spa.longitude);
		myComm->publish(NULL, "solar/gps/lon", stringValue.length(), stringValue.c_str(), 0 , false);
		/* Publish latitute */
		stringValue = std::to_string(spa.latitude);
		myComm->publish(NULL, "solar/gps/lat", stringValue.length(), stringValue.c_str(), 0 , false);
		/* Publish latitute */
		stringValue = std::to_string(spa.latitude);
		myComm->publish(NULL, "solar/gps/ele", stringValue.length(), stringValue.c_str(), 0 , false);



		/* Debug
		 * publish each 5min*/
		if (i % 300 == 0) {
			std::string time = std::to_string(spa.hour) + ":" +  std::to_string(spa.minute) + ":" +
					std::to_string(spa.second);
			stringValue = std::to_string(spa.azimuth) + "," + std::to_string(spa.zenith) + "," +
					std::to_string(ret) + "," + time;
			myComm->publish(NULL, "solar/debug", stringValue.length(), stringValue.c_str(), 0 , false);

		}
		/* Publish ret */
		stringValue = std::to_string(azimuthNormalized) + " " + std::to_string(elevationNormalized) + " " +
				std::to_string(currentZePulsePos);
		myComm->publish(NULL, "solar/norm", stringValue.length(), stringValue.c_str(), 0 , false);

		i++;



		// std::cout << "local CMD: " << localCmd << std::endl;
	}
}

void SolarTracker::zeRepos(){

	int zenithPulses = elevationNormalized * 320.0/18.0;
	int zeDeltaPulses = zenithPulses - currentZePulsePos;

	std::cout << "zeDeltaPulses: "  << zeDeltaPulses << std::endl;

	/* Do for delta pulses */
	if (currentZePulsePos < 3200 && zeDeltaPulses > 0) {
		realTimeHardware->goPos(PRU::ZENITH_SERVO, PRU::COUNTERCLOCKWISE, zeDeltaPulses);

		/* Store current position */
		currentZePulsePos = zenithPulses;
	}

	if (currentZePulsePos > 3200)
		std::cerr << "Number of pulses outside hardware window: " << currentAzPulsePos << std::endl;
}

void SolarTracker::zeGoHome(){

	if ((currentZePulsePos < 3200) && (currentZePulsePos > 0)){
		std::cout << "Returning: " << currentZePulsePos << "pulses" << std::endl;
		realTimeHardware->goPos(PRU::ZENITH_SERVO, PRU::CLOCKWISE, currentZePulsePos);
		currentZePulsePos = 0;
	}
	else
		std::cerr << "Number of pulses outside hardware window: " << currentZePulsePos << std::endl;
}

int SolarTracker::checkSunRiseSunSet(){

#ifdef DEBUG
	float min, sec;
	min = 60.0*(spa.sunrise - (int)(spa.sunrise));
	sec = 60.0*(min - (int)min);

	min = 60.0*(spa.sunset - (int)(spa.sunset));
	sec = 60.0*(min - (int)min);

	printf("Sunrise:       %02d:%02d:%02d Local Time\n", (int)(spa.sunrise), (int)min, (int)sec);
	printf("Sunset:        %02d:%02d:%02d Local Time\n", (int)(spa.sunset), (int)min, (int)sec);
#endif

	/* Fractional current time */
	double now = spa.hour + spa.minute/60.0 + spa.second/3600.0;

	/* Check SunRise */
	if (now < spa.sunrise)
		return BEFORE_SUNRISE;

	if (now > spa.sunset)
		return AFTER_SUNSET;

	return SUN_OK;
}


void SolarTracker::MagComThreadFunction(){

	int ret, localCmd = SOLAR_RUNNING;
	int i = 0;

	while (localCmd == SOLAR_RUNNING){

		ret = magSensor->refresh();

		/* Read exit command */
		inputOutputMutex.lock();
		localCmd = cmd;
		inputOutputMutex.unlock();

		sleep(1);
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


void SolarTracker::SPACalculationThreadFunction(int mode){
	int result;

	/* System Data */
	time_t t = time(0);
	struct tm * now = localtime(&t);

	//enter required input values into SPA structure
	spa.year          = now->tm_year + 1900;
	spa.month         = now->tm_mon + 1;
	spa.day           = now->tm_mday;

#ifdef DEBUG
	std::cout << "year: " <<  spa.year << std::endl;
	std::cout << "month: " << spa.month << std::endl;
	std::cout << "day: " << spa.day << std::endl;
#endif

	if (mode == GPS_ONLINE){
		spa.hour          = serialGPS->get_hh() - 2;
		spa.minute        = serialGPS->get_mm();
		spa.second        = serialGPS->get_ss();
	}else {
		spa.hour          = now->tm_hour - 2;
		spa.minute        = now->tm_min;
		spa.second        = now->tm_sec;
	}

#ifdef DEBUG
	std::cout << "hour: " <<  spa.hour << std::endl;
	std::cout << "minute: " << spa.minute << std::endl;
	std::cout << "second: " << spa.second << std::endl;
#endif

	spa.timezone      = -2.0;
	spa.delta_ut1     = 0;
	spa.delta_t       = 67;

	if (mode == GPS_ONLINE){
		spa.longitude     = serialGPS->get_longitute();
		spa.latitude      = serialGPS->get_latitue();
		spa.elevation     = serialGPS->get_altitude();
	}else {
		spa.longitude     = longitude;
		spa.latitude      = latitude;
		spa.elevation     = elevation;
	}

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

#ifdef DEBUG
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
#endif

#ifdef DEBUG
		float min, sec;
		min = 60.0*(spa.sunrise - (int)(spa.sunrise));
		sec = 60.0*(min - (int)min);

		min = 60.0*(spa.sunset - (int)(spa.sunset));
		sec = 60.0*(min - (int)min);


		printf("Sunrise:       %02d:%02d:%02d Local Time\n", (int)(spa.sunrise), (int)min, (int)sec);
		printf("Sunset:        %02d:%02d:%02d Local Time\n", (int)(spa.sunset), (int)min, (int)sec);
#endif

	} else
		std::cerr << "SPA Error Code: " << result << std::endl;


}


SolarTracker::SolarTracker(const char* GPSdevFilename) {

	cmd = SOLAR_RUNNING;

	/* Zenith pos */
	currentZePulsePos = 0;
	elevationNormalized = 0;

	/* Azimuth position */
	currentAzPulsePos = 0;
	azimuthNormalized = 0;

	realTimeHardware = new PRU();
	serialGPS = new GPS(GPSdevFilename);
	magSensor = new Magnetometer();

	/* Check existence of loc.conf */
	std::ifstream confFileIn ("loc.conf");

	if (!confFileIn.is_open()){
		std::cerr << "Location file missing, generating... " << std::endl;
		int ret;

		/* Get location */
		do ret = serialGPS->ReadandParse(); while (ret != 0);

		/* Write to disk */
		if (writeLocConfFile() != 0){
			std::cerr << "Could not write loc.conf... Aborting" << std::endl;
			exit(-1);
		}
	}else
		readLocConfFile();

	gpsComThread = new std::thread(&SolarTracker::GPSComThreadFunction, this);
	//MagComThread = new std::thread(&SolarTracker::MagComThreadFunction, this);
	inputOutputThread = new std::thread(&SolarTracker::inputOutputFunction, this);

	myComm = new MqttComm("soltTracker", "localhost", 1883);
}

int SolarTracker::writeLocConfFile(){

	int ret = 0;
	std::ofstream confFileOut ("loc.conf");

	if (confFileOut.is_open()){
		confFileOut << std::fixed << std::setprecision(6);

		latitude      = serialGPS->get_latitue();
		longitude     = serialGPS->get_longitute();
		elevation     = serialGPS->get_altitude();

		confFileOut << latitude << ";" << longitude << ";" << elevation;

		confFileOut.close();

	} else{
		std::cerr << "Could not write loc.conf... Aborting" << std::endl;
		ret = -1;
	}

	return ret;
}

void SolarTracker::readLocConfFile(){

	std::string line;
	std::ifstream confFileIn ("loc.conf");
	std::vector<std::string> tokens;

	if (!confFileIn.is_open()){
		std::cerr << "Location file missing, it is strange... " << std::endl;
	}

	/* Fist line is the location: latitude;longitude;altitude */
	std::getline (confFileIn,line);

	tokens = split(line,";");

	if (tokens.size() < 3){
		std::cerr << "Location file is invalid, it is strange... " << std::endl;
	}
	else {
		latitude = std::stof(tokens[0]);
		longitude = std::stof(tokens[1]);
		elevation = std::stof(tokens[2]);
	}
	confFileIn.close();
}


SolarTracker::~SolarTracker() {
	inputOutputThread->join();
	gpsComThread->join();
	//MagComThread->join();

	/* Write last known location */
	writeLocConfFile();
	/* Go home position */
	//zeGoHome();

	myComm->disconnect();

	delete inputOutputThread;
	delete gpsComThread;
	//delete MagComThread;

	delete serialGPS;
	delete magSensor;
	delete myComm;
	delete realTimeHardware;

}

