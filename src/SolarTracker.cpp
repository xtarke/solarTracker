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

	int ret, goHome = false, localCmd = SOLAR_RUNNING;

	/* Previous calculated zenith angle            *
	 * Used to calculate delta between            *
	 * two interactions. 180 is not a valid angle *
	 * but it is used to ignore first iteration   */
	double zenithk_1 = 180;
	//double deltaZenith = 0;
	//double azimuthk_1 = 0;
	// double deltaAzimuth = 0;

	/* Zenith angle between 0 a 180 degress */
	//double scaledZenith = 0;

	while (localCmd != SOLAR_EXIT){

		/* Read exit command */
		inputOutputMutex.lock();
		localCmd = solarStatus.cmd;
		goHome = solarStatus.goHome;
		inputOutputMutex.unlock();

		ret = serialGPS->ReadandParse();
		//serialGPS->printNumericalData();

		if (ret == GPS::GPS_SUCCESS){

			inputOutputMutex.lock();
			SPACalculation(GPS_ONLINE);

			solarStatus.GPSstatus = GPS::GPS_SUCCESS;

			/* Update location */
			longitude     = serialGPS->get_longitute();
			latitude      = serialGPS->get_latitue();
			elevation     = serialGPS->get_altitude();

			inputOutputMutex.unlock();

		}
		else {
			inputOutputMutex.lock();
			solarStatus.GPSstatus = ret;
			inputOutputMutex.unlock();

			std::cerr << "GPS offline, using local parameters: " << ret << std::endl;
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
		solarStatus.deltaZenith = solarStatus.spa.zenith - zenithk_1;
		zenithk_1 = solarStatus.spa.zenith;
		//deltaAzimuth = solarStatus.spa.azimuth - azimuthk_1;
		//azimuthk_1 = solarStatus.spa.azimuth;

		/* Normalize angles: */
		inputOutputMutex.lock();
		solarStatus.azimuthNormalized = (solarStatus.spa.azimuth > 180) ?
				(360.0 - solarStatus.spa.azimuth) : (-solarStatus.spa.azimuth);

		/* Normaliza now for Eppley */
		solarStatus.azimuthNormalized += 180;

		//if (solarStatus.azimuthNormalized < 0)

		/* Test Zenith derivative. If zero, does nothing */
		/* if (solarStatus.deltaZenith < 0)
			solarStatus.elevationNormalized = 90 - solarStatus.spa.zenith;
		else if (solarStatus.deltaZenith > 0)
			solarStatus.elevationNormalized = solarStatus.spa.zenith + 90; */

		solarStatus.elevationNormalized = 90 - solarStatus.spa.zenith;

		/* if ((solarStatus.deltaZenith < 0) && (solarStatus.azimuthNormalized < 0))
			scaledZenith = solarStatus.spa.zenith;
		else if ((solarStatus.deltaZenith > 0) && (solarStatus.azimuthNormalized >= 0))
			scaledZenith = -solarStatus.spa.zenith + 2*solarStatus.minZenith;

		solarStatus.elevationNormalized = 90 - scaledZenith;*/

		inputOutputMutex.unlock();


		if (goHome == true && localCmd == SOLAR_MANUAL) {
			inputOutputMutex.lock();
			solarStatus.goHome = false;
			inputOutputMutex.unlock();

			zeGoHome();
			azGoHome();
			continue;
		}

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
		stringValue = std::to_string(localStatus.spa.elevation);
		myComm->publish(NULL, "solar/gps/ele", stringValue.length(), stringValue.c_str(), 0 , false);


		float min, sec;
		min = 60.0*(localStatus.spa.sunrise - (int)(localStatus.spa.sunrise));
		sec = 60.0*(min - (int)min);

		stringValue = std::to_string((int)(localStatus.spa.sunrise)) + ":" + std::to_string((int)min) + ":" + std::to_string((int)sec);
		myComm->publish(NULL, "solar/sunrise", stringValue.length(), stringValue.c_str(), 0 , false);

		min = 60.0*(localStatus.spa.sunset - (int)(localStatus.spa.sunset));
		sec = 60.0*(min - (int)min);

		stringValue = std::to_string((int)(localStatus.spa.sunset)) + ":" + std::to_string((int)min) + ":" + std::to_string((int)sec);
		myComm->publish(NULL, "solar/sunset", stringValue.length(), stringValue.c_str(), 0 , false);

		/* Debug
		 * publish each 5min*/
		//if (i % 300 == 0) {
			std::string date = std::to_string(solarStatus.spa.year) + ":" +
					std::to_string(solarStatus.spa.month) + ":" +
					std::to_string(solarStatus.spa.day);
			std::string time = std::to_string(localStatus.spa.hour) + ":" +  std::to_string(localStatus.spa.minute);
			stringValue = std::to_string(localStatus.spa.azimuth) +  "," + std::to_string(localStatus.spa.azimuth_astro) + ","
					+ std::to_string(localStatus.spa.zenith) + "," +
					std::to_string(localStatus.GPSstatus) + "," + time;
			myComm->publish(NULL, "solar/debug", stringValue.length(), stringValue.c_str(), 0 , false);

			myComm->publish(NULL, "solar/time", time.length(), time.c_str(), 0 , false);
			myComm->publish(NULL, "solar/date", date.length(), date.c_str(), 0 , false);

		//}
		/* Publish ret */
		stringValue = std::to_string(localStatus.azimuthNormalized) + " " + std::to_string(localStatus.elevationNormalized)
				+ " " + std::to_string(localStatus.currentZePulsePos) + " " +
				std::to_string(localStatus.currentAzPulsePos);
		myComm->publish(NULL, "solar/norm", stringValue.length(), stringValue.c_str(), 0 , false);

		i++;

		/* Sleep for 1 second */
		sleep(1);
	}
}


void SolarTracker::azRepos(){

	/* For lego */
	//int azimuthPulses = solarStatus.azimuthNormalized * 900.0/180.0;
	/* For Eppley */
	int azimuthPulses = solarStatus.azimuthNormalized / PULSES_PER_STEP;
	int azDeltaPulses = azimuthPulses - solarStatus.currentAzPulsePos;
	int pulses;

#ifdef DEBUG
	std::cout << "azimuthPulses: " << azimuthPulses << std::endl;
	std::cout << "\tazDeltaPulses: "  << azDeltaPulses << std::endl;
#endif

	/* Morning: go east */
	if (azDeltaPulses < 0){

		pulses = abs(azDeltaPulses);

		if ((solarStatus.currentAzPulsePos > -AZ_MAX_PULSES) && (pulses > 0)) {
//			realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::CLOCKWISE, pulses);

			realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::COUNTERCLOCKWISE, pulses);


			inputOutputMutex.lock();
			solarStatus.currentAzPulsePos = azimuthPulses;
			inputOutputMutex.unlock();

#ifdef DEBUG
			std::cout << "Going east: " << pulses << std::endl;
			std::cout << "\tcurrentAzPulsePos: " << solarStatus.currentAzPulsePos << std::endl;
#endif
		}
	}

	/* Afternoon: go west */
	if (azDeltaPulses >= 0){

		pulses = abs(azDeltaPulses);

		if ((solarStatus.currentAzPulsePos < AZ_MAX_PULSES) && (pulses > 0)){
			realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::CLOCKWISE, pulses);

			inputOutputMutex.lock();
			solarStatus.currentAzPulsePos = azimuthPulses;
			inputOutputMutex.unlock();

#ifdef DEBUG
			std::cout << "Going west: " << pulses << std::endl;
			std::cout << "\tcurrentAzPulsePos: " << solarStatus.currentAzPulsePos << std::endl;
#endif
		}
	}
}

void SolarTracker::azGoHome(){

	int pulses = abs(solarStatus.currentAzPulsePos);

	/* Morning: go back from east */
	if ((solarStatus.currentAzPulsePos < 0) && (pulses < AZ_MAX_PULSES) ){

		std::cout << "\tReturning: " << pulses << "pulses" << std::endl;
		realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::CLOCKWISE, pulses);

		/* Store current position */
		inputOutputMutex.lock();
		solarStatus.currentAzPulsePos = 0;
		inputOutputMutex.unlock();
	}

	/* Afternoon: go back from west */
	if ((solarStatus.currentAzPulsePos > 0) && (pulses < AZ_MAX_PULSES)) {
		std::cout << "\tReturning: " << pulses << "pulses" << std::endl;

		realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::COUNTERCLOCKWISE, pulses);

		/* Store current position */
		inputOutputMutex.lock();
		solarStatus.currentAzPulsePos = 0;
		inputOutputMutex.unlock();
	}
}


void SolarTracker::zeRepos(){
	/* Lego Model
	int zenithPulses = solarStatus.elevationNormalized * 320.0/18.0;
	*/

	int zenithPulses = solarStatus.elevationNormalized / PULSES_PER_STEP;
	int zeDeltaPulses = zenithPulses - solarStatus.currentZePulsePos;
	int pulses = abs(zeDeltaPulses);

#ifdef DEBUG
	std::cout << "\tzenithPulses: "  << zenithPulses << std::endl;
#endif

	/* Do for delta pulses */
	if ((solarStatus.currentZePulsePos <= ZE_MAX_PULSES) && (solarStatus.currentZePulsePos >= 0) && (pulses > 0)) {

		if (zenithPulses < 0)
				zenithPulses = 0;
		else {
			if (zeDeltaPulses > 0)
				realTimeHardware->goPos(PRU::ZENITH_SERVO, PRU::COUNTERCLOCKWISE, pulses);
			else
				realTimeHardware->goPos(PRU::ZENITH_SERVO, PRU::CLOCKWISE, pulses);
		}

		/* Store current position */
		inputOutputMutex.lock();
		solarStatus.currentZePulsePos = zenithPulses;
		inputOutputMutex.unlock();
	}
}

void SolarTracker::zeGoHome(){

	if ((solarStatus.currentZePulsePos <= ZE_MAX_PULSES) && (solarStatus.currentZePulsePos > 0)){
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

	while ( localCmd != SOLAR_EXIT){

		/* Read global values  */
		inputOutputMutex.lock();
		localCmd = solarStatus.cmd;
		inputOutputMutex.unlock();

        //magSensor->refresh();

		usleep(50000);
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

	volatile int localCmd = SOLAR_RUNNING, mycmd;

	while (localCmd != SOLAR_EXIT){

		if (!myComm->queueCmdIsEmpty()) {

			mycmd = myComm->deQueueCmd();

			inputOutputMutex.lock();
			solarStatus.cmd = mycmd;

			if (mycmd == SOLAR_MANUAL)
				solarStatus.goHome = true;

			inputOutputMutex.unlock();

		}

		if (!myComm->queueAzIsEmpty() ) {
			mycmd = myComm->deQueueAz();
			if (localCmd == SOLAR_MANUAL)
				azManualPos(mycmd);
		}

		if (!myComm->queueZeIsEmpty() ) {
			mycmd = myComm->deQueueZe();
			if (localCmd == SOLAR_MANUAL)
				zeManualPos(mycmd);
		}

		if (!myComm->queueZeHomeIsEmpty() ) {
			mycmd = myComm->deQueueZeHome();
			
			//TODO: Mutex?
			std::cerr << solarStatus.spa.year << "-";
			std::cerr << solarStatus.spa.month << "-";
			std::cerr << solarStatus.spa.day << " ";
			std::cerr << solarStatus.spa.hour << "-";
			std::cerr << solarStatus.spa.minute << "-";
			std::cerr << solarStatus.spa.second << " ";
			std::cerr << "ZeHome (cmd) (value):  " << localCmd  << " / " <<  mycmd << std::endl;

			if (localCmd == SOLAR_MANUAL || localCmd == SOLAR_CALIB)
					zeSetHomePos(mycmd);
		}

		if (!myComm->queueAzHomeIsEmpty() ) {

			mycmd = myComm->deQueueAzHome();

			//TODO: Mutex?
			std::cerr << solarStatus.spa.year << "-";
			std::cerr << solarStatus.spa.month << "-";
			std::cerr << solarStatus.spa.day << " ";
			std::cerr << solarStatus.spa.hour << "-";
			std::cerr << solarStatus.spa.minute << "-";
			std::cerr << solarStatus.spa.second << " ";
			std::cerr << "AzHome (cmd) (value):  " << localCmd  <<  " / " << mycmd << std::endl;			
	
			if (localCmd == SOLAR_MANUAL || localCmd == SOLAR_CALIB)
				azSetHomePos(mycmd);
		}

		/* Read exit command */
		inputOutputMutex.lock();
		localCmd = solarStatus.cmd;
		inputOutputMutex.unlock();

		sleep(1);
	}

}

void SolarTracker::zeSetHomePos(int pulses){

	/* Negative value: go clockwise */
	if (pulses < 0){
		std::cerr << "\tZENITH_SERVO: " << pulses << std::endl;
		realTimeHardware->goPos(PRU::ZENITH_SERVO, PRU::CLOCKWISE, abs(pulses));
	}

	/* Negative value: go clockwise */
	if (pulses > 0){
		std::cerr << "\tZENITH_SERVO: " << pulses << std::endl;
		realTimeHardware->goPos(PRU::ZENITH_SERVO, PRU::COUNTERCLOCKWISE, abs(pulses));
	}
}

void SolarTracker::azSetHomePos(int pulses){

	/* Negative value: go clockwise */
	if (pulses < 0){
		std::cerr << "\tAZIMUTH_SERVO: " << pulses << std::endl;
		realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::CLOCKWISE, abs(pulses));
	}

	/* Negative value: go clockwise */
	if (pulses > 0){
		std::cerr << "\tAZIMUTH_SERVO: " << pulses << std::endl;
		realTimeHardware->goPos(PRU::AZIMUTH_SERVO, PRU::COUNTERCLOCKWISE, abs(pulses));
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

void SolarTracker::zeManualPos(int pulses){
	int currentPos = 0;
	int zeDeltaPulses;
	int absPulses;

	/* Read current position */
	inputOutputMutex.lock();
	currentPos = solarStatus.currentZePulsePos;
	inputOutputMutex.unlock();

	/* Go counterclockwise */
	if ((pulses > 0) && (currentPos < ZE_MAX_PULSES)){

		zeDeltaPulses = currentPos + pulses;

		if (zeDeltaPulses > ZE_MAX_PULSES){
			pulses = ZE_MAX_PULSES - currentPos;
		}

		currentPos += pulses;

		if (pulses > 0)
			realTimeHardware->goPos(PRU::ZENITH_SERVO, PRU::COUNTERCLOCKWISE, pulses);

		/* Store current position */
		inputOutputMutex.lock();
		solarStatus.currentZePulsePos = currentPos;
		inputOutputMutex.unlock();
	}

	/* Go counterclockwise */
	if ((pulses < 0) && (currentPos > 0)){

		zeDeltaPulses = currentPos + pulses;

		if (zeDeltaPulses < 0){
			pulses = 0 - currentPos;
		}
		currentPos += pulses;

		absPulses = abs(pulses);

		if (absPulses > 0)
			realTimeHardware->goPos(PRU::ZENITH_SERVO, PRU::CLOCKWISE, absPulses);

		/* Store current position */
		inputOutputMutex.lock();
		solarStatus.currentZePulsePos = currentPos;
		inputOutputMutex.unlock();
	}

}

void SolarTracker::getSolarNoonZeAngle(){
	int result;
	/* System Data */
	time_t t = time(0);
	struct tm * now = localtime(&t);

	spa_data spa;

	double minZenith = 90;

	//enter required input values into SPA structure
	spa.year          = now->tm_year + 1900;
	spa.month         = now->tm_mon + 1;
	spa.day           = now->tm_mday;

	spa.timezone      = -3.0;
	spa.delta_ut1     = 0;
	spa.delta_t       = 67;

	spa.longitude     = longitude;
	spa.latitude      = latitude;
	spa.elevation     = elevation;

	spa.pressure      = 820;
	spa.temperature   = 11;
	spa.slope         = 30;
	spa.azm_rotation  = -10;
	spa.atmos_refract = 0.5667;
	spa.function      = SPA_ZA;


	/* Iteration between 10h and 16h: localtime */
	for (int i=0; i < 6*3600; i++){

		float hour = (float)i / 3600;
		float min = (hour - (int)hour)*60;
		float sec = (min - (int)min)*60;

		spa.hour          = 10 + (int)hour ;
		spa.minute        = (int)min;
		spa.second        = (int)sec;

		result = spa_calculate(&spa);

		if (result  != 0){
			std::cerr << "Erro estimating min Zenith angle (spa error code): " << result << std::endl;
			exit(-1);
		}

		if (spa.zenith < minZenith)
			minZenith = spa.zenith;
	}

	solarStatus.minZenith = minZenith;
	std::cout << "Min Zenith is:" << minZenith << std::endl;

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

	solarStatus.spa.timezone      = -3.0;
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

	} else {
		std::cerr << solarStatus.spa.year << "-";
		std::cerr << solarStatus.spa.month << "-";
		std::cerr << solarStatus.spa.day << " ";
		std::cerr << solarStatus.spa.hour << "-";
		std::cerr << solarStatus.spa.minute << "-";
		std::cerr << solarStatus.spa.second << " ";
		std::cerr << "SPA Error Code: " << result << std::endl;
	}	
}


SolarTracker::SolarTracker(const char* GPSdevFilename, std::string configPath) {

	int ret;
	solarStatus.cmd = SOLAR_MANUAL;
	solarStatus.goHome = false;
	solarStatus.GPSstatus = GPS::GPS_NOT_READY;

	myconfigPath = configPath;

	/* Zenith pos */
	solarStatus.currentZePulsePos = 0;
	solarStatus.elevationNormalized = 0;

	/* Azimuth position */
	solarStatus.currentAzPulsePos = 0;
	solarStatus.azimuthNormalized = 0;

	realTimeHardware = new PRU(myconfigPath);
	serialGPS = new GPS(GPSdevFilename);

	myComm = new MqttComm("soltTracker", "localhost", 1883);
	mqqtPublishThread = new std::thread(&SolarTracker::mqttPublishFunction, this);

	std::string confFile = configPath + "/loc.conf";

	/* Check existence of loc.conf */
	std::ifstream confFileIn (confFile);

	if (!confFileIn.is_open()){
		std::cerr << "Location file missing, generating... " << std::endl;

		/* Get location */
		do ret = serialGPS->ReadandParse(); while (ret != 0);

		/* Write to disk */
		if (writeLocConfFile() != 0){
			std::cerr << "Could not write loc.conf... Aborting" << std::endl;
			exit(-1);
		}
	}else
		readLocConfFile();

	/* disable NTP */
	system("timedatectl set-ntp 0");

	/* Configure system time */
	do ret = serialGPS->ReadandParse(); while (ret != 0);
	/* Get time */
	serialGPS->updateDate();

	/*  timedatectl set-time "yyyy-MM-dd hh:mm:ss" */

	std::string time = "timedatectl set-time \"" + std::to_string((int)serialGPS->get_year()) + "-" +
			std::to_string((int)serialGPS->get_month()) + "-" +
			std::to_string((int)serialGPS->get_day()) + " " +
			std::to_string((int)serialGPS->get_hh()) + ":" +
			std::to_string((int)serialGPS->get_mm()) + ":" +
			std::to_string((int)serialGPS->get_ss()) + "\"";

	std::cout << "Configure time: " << time << std::endl;
	system(time.c_str());

	/* After gps is online */

	//Todo: magSensor = new Magnetometer(latitude, longitude, elevation);
	//Todo: getSolarNoonZeAngle();

	gpsComThread = new std::thread(&SolarTracker::GPSComThreadFunction, this);
	//Todo: MagComThread = new std::thread(&SolarTracker::MagComThreadFunction, this);
	//Todo: inputOutputThread = new std::thread(&SolarTracker::inputOutputFunction, this);
	mqqtCommandsThread = new std::thread(&SolarTracker::mqttCommandsFunction, this);

}

int SolarTracker::writeLocConfFile(){

	int ret = 0;

	ret = serialGPS->ReadandParse();

	if (ret == 0) {

		std::string confFile = myconfigPath + "/loc.conf";
		std::ofstream confFileOut (confFile);

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
	std::string confFile = myconfigPath + "/loc.conf";
	std::ifstream confFileIn (confFile);
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
	// delete magSensor;
	delete myComm;
	delete realTimeHardware;

}

