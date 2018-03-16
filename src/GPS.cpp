/*
 * GPS.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: Renan Augusto Starke
 */

#include "GPS.h"

#define DEBUG

int GPS::ReadandParse(){
	int ret;

	gpsCom->readData(gpsData);
	ret = parseStringData();
	gpsData.clear();

	return ret;
}


int GPS::parseStringData(){

	std::vector<std::string> dataParts;
	std::string::size_type frameInit, frameSize;

	frameInit = gpsData.rfind("$GPGGA");

	if (frameInit != std::string::npos){
		frameSize = gpsData.substr(frameInit).find('\n');

		std::string gpsPosData = gpsData.substr(frameInit,frameSize);
		split( dataParts, gpsPosData, "," );

#ifdef DEBUG
		std::cout << "[" << frameInit << ", " << frameSize << "] " << gpsPosData << std::endl;
		std::cout << "I found: " << dataParts.size() << " Parameters" << std::endl;
		for (unsigned int i=0; i < dataParts.size(); i++)
			std::cout << dataParts[i] << std::endl;

		std::cout << "--------------------" << std::endl;
#endif

		/* Convert data to numerical data */

		try {
			numericalGpsData.hh = std::stoi(dataParts[1].substr(0,2));
			numericalGpsData.mm = std::stoi(dataParts[1].substr(2,2));
			numericalGpsData.ss = std::stoi(dataParts[1].substr(4,2));
		}
		catch(const std::exception& e){
			std::cerr << "Gps not yet Ready\n";
			return GPS_NOT_READY;
		}

		/* Latitude is:  ddmm.mmmm
		 * SPA needs fraction degress:
		 * Decimal Degrees = degrees + (minutes/60) + (seconds/3600)
		 */
		float degress;
		float minutes;
		float fracMinutes;

		try{
			degress = std::stof(dataParts[2].substr(0,2));
			minutes = std::stof(dataParts[2].substr(2,2));
			fracMinutes = std::stof(dataParts[2].substr(4,3));
		} catch(const std::exception& e){
			std::cerr << "Gps not yet Ready\n";
			return GPS_NOT_READY;
		}



#ifdef DEBUG
		std::cout << "\tLatitude degrees: " << degress << " " << minutes << "  " << fracMinutes << std::endl;
#endif

		numericalGpsData.latitute = degress + minutes/60 + fracMinutes/60;

#ifdef DEBUG
		std::cout << "\tLatitude degrees: " << numericalGpsData.latitute << std::endl;
#endif

		numericalGpsData.hemisphere = dataParts[3][0];

		/* Longitude is:  dddmm.mmmm
		 * SPA needs fraction degress:
		 * Decimal Degrees = degrees + (minutes/60) + (seconds/3600)
		 */

		try {
			degress = std::stof(dataParts[4].substr(0,3));
			minutes = std::stof(dataParts[4].substr(3,2));
			fracMinutes = std::stof(dataParts[4].substr(5,3));
		}catch(const std::exception& e){
			std::cerr << "Gps not yet Ready\n";
			return GPS_NOT_READY;
		}

#ifdef DEBUG
		std::cout << "\tFLongitude degrees: " << degress << " " << minutes << "  " << fracMinutes << std::endl;
#endif

		numericalGpsData.longitute = degress + minutes/60 + fracMinutes/60;

#ifdef DEBUG
		std::cout << "\nLongitute degrees: " << numericalGpsData.longitute << std::endl;
#endif

		try {
			numericalGpsData.meridian = dataParts[5][0];
			numericalGpsData.posFix = std::stoi(dataParts[6]);

			numericalGpsData.satellites = std::stoi(dataParts[7]);
			numericalGpsData.hdop = std::stof(dataParts[8]);

			numericalGpsData.mslAltitue = std::stof(dataParts[9]);
			numericalGpsData.altitudeUnit = dataParts[10][0];

			numericalGpsData.geoSep = std::stof(dataParts[11]);
			numericalGpsData.geoSepUnit = dataParts[12][0];
		}catch(const std::exception& e){
			std::cerr << "Gps not yet Ready\n";
			return GPS_NOT_READY;
		}
	}
	else
		return GPS_FAILURE;

	return GPS_SUCCESS;
}

void GPS::printNumericalData()
{
	std::cout << "hh: " << (int)numericalGpsData.hh << std::endl;
	std::cout << "mm: " << (int)numericalGpsData.mm << std::endl;
	std::cout << "ss: " << (int)numericalGpsData.ss << std::endl;
	std::cout << "latitude: " << numericalGpsData.latitute << std::endl;
	std::cout << "hemisphere: " << numericalGpsData.hemisphere << std::endl;

	std::cout << "longitude: " << numericalGpsData.longitute << std::endl;
	std::cout << "meridian: " << numericalGpsData.meridian << std::endl;

	std::cout << "posFix: " << (int)numericalGpsData.posFix << std::endl;

	std::cout << "satellites: " << (int)numericalGpsData.satellites << std::endl;
	std::cout << "hdop: " << numericalGpsData.hdop << std::endl;
	std::cout << "mslAltitue: " << numericalGpsData.mslAltitue << std::endl;

	std::cout << "altitudeUnit: " << numericalGpsData.altitudeUnit << std::endl;
	std::cout << "geoSep: " << numericalGpsData.geoSep << std::endl;
	std::cout << "geoSepUnit: " << numericalGpsData.geoSepUnit << std::endl;
}


GPS::GPS(const char* devFilename) {

	gpsCom = new SerialCom(devFilename);

}

GPS::~GPS() {
	delete gpsCom;
}

