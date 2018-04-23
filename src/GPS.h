/*
 * GPS.h
 *
 *  Created on: Nov 23, 2017
 *      Author: xtarke
 */

#ifndef GPS_H_
#define GPS_H_

#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

#include "SerialCom.h"

extern "C" {
	#include "spa/spa.h"
}

class GPS {
public:
	GPS(const char* devFilename);
	virtual ~GPS();

	int ReadandParse();
	int updateDate();

	void printNumericalData();

	uint8_t get_hh() {return numericalGpsData.hh;}
	uint8_t get_mm() {return numericalGpsData.mm;}
	uint8_t get_ss() {return numericalGpsData.ss;}

	uint8_t get_day() {return numericalGpsData.day;}
	uint8_t get_month() {return numericalGpsData.month;}
	uint16_t get_year() {return numericalGpsData.year;}

	float get_latitue() { return (numericalGpsData.hemisphere == 'N') ?
			numericalGpsData.latitute : -(numericalGpsData.latitute);  }

	float get_longitute() { return (numericalGpsData.meridian == 'E') ?
				numericalGpsData.longitute : -(numericalGpsData.longitute);  }

	float get_altitude() {return numericalGpsData.mslAltitue;}

	// all failure return codes must be negative
	enum gps_return_code {
		GPS_NOT_READY = -2,
		GPS_FAILURE = -1,
		GPS_SUCCESS = 0
	};

private:
	/* Serial tty communication */
	SerialCom *gpsCom;

	/* NEMA string Data */
	std::string gpsData;

	/* Parser functions */
	int parseStringData();
	int parseStringDateData();

	struct split {
		enum empties_t { empties_ok, no_empties };
	};

	/* Split template for data Parsing */
	template <typename Container> Container& split(Container& result,
				const typename Container::value_type& s,
				const typename Container::value_type& delimiters,
				split::empties_t empties = split::empties_ok )
	{
		result.clear();
		size_t current;
		size_t next = -1;

		do
		{
			if (empties == split::no_empties) {
				next = s.find_first_not_of( delimiters, next + 1 );
				if (next == Container::value_type::npos) break;
					next -= 1;
			}
			current = next + 1;
			next = s.find_first_of( delimiters, current );
			result.push_back( s.substr( current, next - current ) );
		}
		while (next != Container::value_type::npos);

		return result;
	}

	struct GGADataFormat{
		/* UTC Time */
		uint8_t hh;
		uint8_t mm;
		uint8_t ss;

		/* UTC Date */
		uint8_t day;
		uint8_t month;
		uint16_t year;

		/* UTC Posistion */
		float latitute;
		char hemisphere;	/* North or South */
		float longitute;
		char meridian;		/* East or West */

		uint8_t posFix;
		/* 0: Fix no available
		 * 1: GPS SPS Mode, fix Valid
		 * 2: Differencial GPS, SSP Mode, fix valid
		 * 3: GPS PPS Mode, fix valid	 */

		uint8_t satellites;

		float hdop; 		/*  Horizontal Dilution of Precision */
		float mslAltitue; 	/*  Altitude (referenced to the Ellipsoid) */
		char altitudeUnit;

		float geoSep;		/* Geoidal Separation */
		char geoSepUnit;

		/* Not used */
		float ageDiffCorre;
		float diffRefStationId;
		uint8_t checkSum;
	} numericalGpsData;
};

#endif /* GPS_H_ */
