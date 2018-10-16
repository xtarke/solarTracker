/*
 * SolarPulses.h
 *
 *  Created on: Oct 10, 2018
 *      Author: xtarke
 */

#ifndef SOLARPULSES_HPP_
#define SOLARPULSES_HPP_

class SolarPulses {
private:
	 /* 180º of resolution: 9600 pulses 	 */
	const float PULSES_PER_STEP = 0.01875;

	enum hardwareLimits {
		AZ_MAX_PULSES = 17000,
		ZE_MAX_PULSES = 4600
	};

	const float LATITUDE = -27.594020;
	const float LONGITUDE = -48.542814;
	const float ELEVATION = 10;


public:
	SolarPulses();
	virtual ~SolarPulses();

	void generateSolarTable();

};

#endif /* SOLARPULSES_HPP_ */
