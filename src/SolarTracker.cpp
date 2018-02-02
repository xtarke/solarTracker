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

	/* Previous calculated zenith angle            *
	 * Used to calculate delta between            *
	 * two interactions. 180 is not a valid angle *
	 * but it is used to ignore first iteration   */
	double zenithk_1 = 180;
	double deltaZenith = 0;
	//double azimuthk_1 = 0;
	// double deltaAzimuth = 0;

	while (localCmd != SOLAR_EXIT){

		/* Read exit command */
		inputOutputMutex.lock();
		localCmd = solarStatus.cmd;
		inputOutputMutex.unlock();

		ret = serialGPS->ReadandParse();
		//serialGPS->printNumericalData();

		if (ret == GPS::GPS_SUCCESS){

			inputOutputMutex.lock();
			SPACalculation(GPS_ONLINE);
			solarStatus.GPSstatus = GPS::GPS_SUCCESS;
			inputOutputMutex.unlock();

			/* Update location */
			longitude     = serialGPS->get_longitute();
			latitude      = serialGPS->get_latitue();
			elevation     = serialGPS->get_altitude();
		}
		else {
			inputOutputMutex.lock();
			solarStatus.GPSstatus = ret;
			inputOutputMutex.unlock();

			std::cerr << "GPS offline, using local parameters" << std::endl;
			SPACalculation(LOCAL_PARAM);
		}

		/* Ignore first iteration: it needs a valid angle first */
		if (zenithk_1 == 180) {
			zenithk_1 = solarStatus.spa.zenith;
			// azimuthk_1 = solarStatus.spa.azimuth;
			continue;
		}

		/* Calculate deltas:                  *
		 * if Zenith delta is lower than 0,   *
		 * Sun is rising. Else it is setting  */
		deltaZenith = solarStatus.spa.zenith - zenithk_1;
		zenithk_1 = solarStatus.spa.zenith;
		//deltaAzimuth = solarStatus.spa.azimuth - azimuthk_1;
		//azimuthk_1 = solarStatus.spa.azimuth;


		std::cout << "\t\t\tdeltaZenith: " << deltaZenith << std::endl;

		/* Normalize angles: */
		inputOutputMutex.lock();
		solarStatus.azimuthNormalized = (solarStatus.spa.azimuth > 180) ?
				(360.0 - solarStatus.spa.azimuth) : (-solarStatus.spa.azimuth);

		//
		if (solarStatus.azimuthNormalized < 0)
		//if (deltaZenith <= 0)
			solarStatus.elevationNormalized = 90 - solarStatus.spa.zenith;
		else
			solarStatus.elevationNormalized = solarStatus.spa.zenith + 90;
		inputOutputMutex.unlock();

		/* Do not update if it is in manual or exiting*/
		if (localCmd != SOLAR_RUNNING)
			continue;

		/* Go home if is night */
		ret = checkSunRiseSunSet();
		if (ret < 0) {
			zeGoHome();
			azGoHome();
			continue;
		}

		/* Update hardware position Zenith angle */
		zeRepos();
		azRepos();
	}
}


void SolarTracker::mqttPublishFunction(){

	int i = 0;

	struct solarStatus_t localStatus;
	localStatus.cmd = SOLAR_RUNNING;

	while (localStatus.cmd != SOLAR_EXIT){

		/* Read exit command */
		inputOutputMutex.lock();
		localStatus = solarStatus;
		inputOutputMutex.unlock();

		if (localStatus.GPSstatus == GPS::GPS_SUCCESS)
			myComm->publish(NULL, "solar/gps/status", 7, "online", 0 , false);
		else
			myComm->publish(NULL, "solar/gps/status", 8, "offline", 0 , false);

		/* Publish azimuth */
		std::string stringValue = std::to_string(localStatus.spa.azimuth);
		myComm->publish(NULL, "solar/az", stringValue.length(), stringValue.c_str(), 0 , false);

		/* Publish zenith */
		stringValue = std::to_string(localStatus.spa.zenith);
		myComm->publish(NULL, "solar/ze", stringValue.length(), stringValue.c_str(), 0 , false);

		/* Publish longitude */
		stringValue = std::to_string(localStatus.spa.longitude);
		myComm->publish(NULL, "solar/gps/lon", stringValue.length(), stringValue.c_str(), 0 , false);
		/* Publish latitute */
		stringValue = std::to_string(localStatus.spa.latitude);
		myComm->publish(NULL, "solar/gps/lat", stringValue.length(), stringValue.c_str(), 0 , false);
		/* Publish latitute */
		stringValue = std::to_string(localStatus.spa.latitude);
		myComm->publish(NULL, "solar/gps/ele", stringValue.length(), stringValue.c_str(), 0 , false);

		/* Debug
		 * publish each 5min*/
		if (i % 300 == 0) {
			std::string time = std::to_string(localStatus.spa.hour) + ":" +  std::to_string(localStatus.spa.minute) + ":" +
					std::to_string(localStatus.spa.second);
			stringValue = std::to_string(localStatus.spa.azimuth) + "," + std::to_string(localStatus.spa.zenith) + "," +
					std::to_string(localStatus.GPSstatus) + "," + time;
			myComm->publish(NULL, "solar/debug", stringValue.length(), stringValue.c_str(), 0 , false);

		}
		/* Publish ret */
		stringValue = std::to_string(localStatus.azimuthNormalized) + " " + std::to_string(localStatus.elevationNormalized) + " " +
				std::to_string(localStatus.currentZePulsePos) + " " + std::to_string(localStatus.currentAzPulsePos);
		myComm->publish(NULL, "solar/norm", stringValue.length(), stringValue.c_str(), 0 , false);

		i++;

		/* Sleep for 1 second */
		sleep(1);
	}
}


void SolarTracker::azRepos(){

	int azimuthPulses = solarStatus.azimuthNormalized * 900.0/180.0;
	int azDeltaPulses = azimuthPulses - solarStatus.currentAzPulsePos;
	int pulses;

	std::cout << "\tazDeltaPulses: "  << azDeltaPulses << std::endl;

	/* Morning: go east */
	if (azDeltaPulses < 0){

		pulses = abs(azDeltaPulses);

		if ((solarStatus.currentAzPulsePos > -950) && (pulses > 0)) {


			std::cout << "Going east: " << pulses << std::endl;


			realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::CLOCKWISE, pulses);

			inputOutputMutex.lock();
			solarStatus.currentAzPulsePos = azimuthPulses;
			inputOutputMutex.unlock();

			std::cout << "\tcurrentAzPulsePos: " << solarStatus.currentAzPulsePos << std::endl;
		}
	}

	/* Afternoon: go west */
	if (azDeltaPulses >= 0){

		pulses = abs(azDeltaPulses);

		if ((solarStatus.currentAzPulsePos < 950) && (pulses > 0)){

			std::cout << "Going west: " << pulses << std::endl;

			realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::COUNTERCLOCKWISE, pulses);

			inputOutputMutex.lock();
			solarStatus.currentAzPulsePos = azimuthPulses;
			inputOutputMutex.unlock();

			std::cout << "\tcurrentAzPulsePos: " << solarStatus.currentAzPulsePos << std::endl;
		}
	}
}

void SolarTracker::azGoHome(){

	int pulses = abs(solarStatus.currentAzPulsePos);

	/* Morning: go back from east */
	if ((solarStatus.currentAzPulsePos < 0) && (pulses < 950) ){

		std::cout << "\tReturning: " << pulses << "pulses" << std::endl;
		realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::COUNTERCLOCKWISE, pulses);

		/* Store current position */
		inputOutputMutex.lock();
		solarStatus.currentAzPulsePos = 0;
		inputOutputMutex.unlock();
	}

	/* Afternoon: go back from west */
	if ((solarStatus.currentAzPulsePos > 0) && (pulses < 950)) {
		std::cout << "\tReturning: " << pulses << "pulses" << std::endl;

		realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::CLOCKWISE, pulses);

		/* Store current position */
		inputOutputMutex.lock();
		solarStatus.currentAzPulsePos = 0;
		inputOutputMutex.unlock();
	}
}


void SolarTracker::zeRepos(){

	int zenithPulses = solarStatus.elevationNormalized * 320.0/18.0;
	int zeDeltaPulses = zenithPulses - solarStatus.currentZePulsePos;
	int pulses = abs(zeDeltaPulses);

	std::cout << "\tzeDeltaPulses: "  << zeDeltaPulses << std::endl;

	/* Do for delta pulses */
	if ((solarStatus.currentZePulsePos < 3200) && (solarStatus.currentZePulsePos >= 0) && (pulses > 0)) {

		if (zeDeltaPulses > 0)
			realTimeHardware->goPos(PRU::ZENITH_SERVO, PRU::COUNTERCLOCKWISE, pulses);
		else
			realTimeHardware->goPos(PRU::ZENITH_SERVO, PRU::CLOCKWISE, pulses);

		/* Store current position */
		inputOutputMutex.lock();
		solarStatus.currentZePulsePos = zenithPulses;
		inputOutputMutex.unlock();
	}
}

void SolarTracker::zeGoHome(){

	if ((solarStatus.currentZePulsePos <= 3200) && (solarStatus.currentZePulsePos > 0)){
		std::cout << "\tReturning: " << solarStatus.currentZePulsePos << "pulses" << std::endl;
		realTimeHardware->goPos(PRU::ZENITH_SERVO, PRU::CLOCKWISE, solarStatus.currentZePulsePos);

		inputOutputMutex.lock();
		solarStatus.currentZePulsePos = 0;
		inputOutputMutex.unlock();
	}

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
	double now = solarStatus.spa.hour + solarStatus.spa.minute/60.0 + solarStatus.spa.second/3600.0;

	/* Check SunRise */
	if (now < solarStatus.spa.sunrise)
		return BEFORE_SUNRISE;

	if (now > solarStatus.spa.sunset)
		return AFTER_SUNSET;

	return SUN_OK;
}


void SolarTracker::MagComThreadFunction(){

	int localCmd = SOLAR_RUNNING;

	while (localCmd != SOLAR_EXIT){

		magSensor->refresh();

		/* Read exit command */
		inputOutputMutex.lock();
		localCmd = solarStatus.cmd;
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
	} while (localCmd != SOLAR_EXIT);


	/* Exit command */
	inputOutputMutex.lock();
	solarStatus.cmd = localCmd;
	inputOutputMutex.unlock();
}

void SolarTracker::mqttCommandsFunction(){

	int localCmd = SOLAR_RUNNING, mycmd;

	while (localCmd != SOLAR_EXIT){

		if (!myComm->queueCmdIsEmpty()) {
			mycmd = myComm->deQueueCmd();

			inputOutputMutex.lock();
			solarStatus.cmd = mycmd;
			inputOutputMutex.unlock();
		}

		if (!myComm->queueAzIsEmpty() ) {
			mycmd = myComm->deQueueAz();
			if (localCmd == SOLAR_MANUAL)
				azManualPos(mycmd);
		}

		/* Read exit command */
		inputOutputMutex.lock();
		localCmd = solarStatus.cmd;
		inputOutputMutex.unlock();

		sleep(1);
	}

}

void SolarTracker::azManualPos(int pulses){

	int currentPos = 0;
	int azDeltaPulses;
	int absPulses;

	/* Read current position */
	inputOutputMutex.lock();
	currentPos = solarStatus.currentAzPulsePos;
	inputOutputMutex.unlock();

	/* Negative value: go East */
	if (((pulses < 0) && (currentPos > -AZ_MAX_PULSES)) ){

		azDeltaPulses = currentPos + pulses;

		if (azDeltaPulses < -AZ_MAX_PULSES)
			pulses = -AZ_MAX_PULSES - currentPos;

		absPulses = abs(pulses);
		currentPos += pulses;

			if (absPulses > 0)
			realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::CLOCKWISE, absPulses);

		/* Store current position */
		inputOutputMutex.lock();
		solarStatus.currentAzPulsePos = currentPos;
		inputOutputMutex.unlock();
	}

	/* Negative value: go West */
	if ((pulses > 0) && (currentPos < AZ_MAX_PULSES)){

		azDeltaPulses = currentPos + pulses;

		if (azDeltaPulses > AZ_MAX_PULSES){
			pulses = AZ_MAX_PULSES - currentPos;
		}

		currentPos += pulses;

		if (pulses > 0)
			realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::COUNTERCLOCKWISE, pulses);

		/* Store current position */
		inputOutputMutex.lock();
		solarStatus.currentAzPulsePos = currentPos;
		inputOutputMutex.unlock();
	}

}


void SolarTracker::SPACalculation(int mode){
	int result;

	/* System Data */
	time_t t = time(0);
	struct tm * now = localtime(&t);

	//enter required input values into SPA structure
	solarStatus.spa.year          = now->tm_year + 1900;
	solarStatus.spa.month         = now->tm_mon + 1;
	solarStatus.spa.day           = now->tm_mday;

#ifdef DEBUG
	std::cout << "year: " <<  spa.year << std::endl;
	std::cout << "month: " << spa.month << std::endl;
	std::cout << "day: " << spa.day << std::endl;
#endif

	if (mode == GPS_ONLINE){
		solarStatus.spa.hour          = serialGPS->get_hh() - 3;
		solarStatus.spa.minute        = serialGPS->get_mm();
		solarStatus.spa.second        = serialGPS->get_ss();
	}else {
		solarStatus.spa.hour          = now->tm_hour - 3;
		solarStatus.spa.minute        = now->tm_min;
		solarStatus.spa.second        = now->tm_sec;
	}

#ifdef DEBUG
	std::cout << "hour: " <<  spa.hour << std::endl;
	std::cout << "minute: " << spa.minute << std::endl;
	std::cout << "second: " << spa.second << std::endl;
#endif

	solarStatus.spa.timezone      = -2.0;
	solarStatus.spa.delta_ut1     = 0;
	solarStatus.spa.delta_t       = 67;

	if (mode == GPS_ONLINE){
		solarStatus.spa.longitude     = serialGPS->get_longitute();
		solarStatus.spa.latitude      = serialGPS->get_latitue();
		solarStatus.spa.elevation     = serialGPS->get_altitude();
	}else {
		solarStatus.spa.longitude     = longitude;
		solarStatus.spa.latitude      = latitude;
		solarStatus.spa.elevation     = elevation;
	}

	solarStatus.spa.pressure      = 820;
	solarStatus.spa.temperature   = 11;
	solarStatus.spa.slope         = 30;
	solarStatus.spa.azm_rotation  = -10;
	solarStatus.spa.atmos_refract = 0.5667;
	solarStatus.spa.function      = SPA_ALL;

	result = spa_calculate(&solarStatus.spa);

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

	solarStatus.cmd = SOLAR_MANUAL;
	solarStatus.GPSstatus = GPS::GPS_NOT_READY;

	/* Zenith pos */
	solarStatus.currentZePulsePos = 0;
	solarStatus.elevationNormalized = 0;

	/* Azimuth position */
	solarStatus.currentAzPulsePos = 0;
	solarStatus.azimuthNormalized = 0;

	realTimeHardware = new PRU();
	serialGPS = new GPS(GPSdevFilename);
	magSensor = new Magnetometer();
	myComm = new MqttComm("soltTracker", "localhost", 1883);

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
	// inputOutputThread = new std::thread(&SolarTracker::inputOutputFunction, this);
	mqqtPublishThread = new std::thread(&SolarTracker::mqttPublishFunction, this);
	mqqtCommandsThread = new std::thread(&SolarTracker::mqttCommandsFunction, this);

}

int SolarTracker::writeLocConfFile(){

	int ret = 0;

	ret = serialGPS->ReadandParse();

	if (ret == 0) {

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
	//inputOutputThread->join();
	gpsComThread->join();
	//MagComThread->join();
	mqqtPublishThread->join();
	mqqtCommandsThread->join();

	/* Write last known location */
	writeLocConfFile();
	/* Go home position */

	zeGoHome();
	azGoHome();

	myComm->disconnect();

	//delete inputOutputThread;
	delete gpsComThread;
	//delete MagComThread;
	delete mqqtPublishThread;
	delete mqqtCommandsThread;

	delete serialGPS;
	delete magSensor;
	delete myComm;
	delete realTimeHardware;

}

