//============================================================================
// Name        : solarTracker.cpp
// Author      : Renan Augusto Starke
// 			   : Charles Borges Lima
// Version     :
// Copyright   : Instituto Federal de Santa Catarna DAELN
// Description :
//
//
//============================================================================

#include <iostream>
#include <unistd.h>
#include <sys/types.h>

#include "SolarTracker.h"
#include "PRU.h"

#include "Magnetometer.h"


using namespace std;


int main() {


	if (getuid() != 0){
		std::cout << "You must run this program as root. Exiting.\n";
		exit(EXIT_FAILURE);
	}

	//SolarTracker solarTracaker("/dev/ttyS1");
	PRU hardware;


/*	for (int i=0; i < 60; i++) {
		hardware.goPos(PRU::AZIMUTH_SERVO, PRU::COUNTERCLOCKWISE, 1);
		sleep(1);

	} */
	//hardware.goPos(PRU::ZENITH_SERVO, PRU::COUNTERCLOCKWISE, 500);
	//sleep(1);
	//hardware.goPos(PRU::AZIMUTH_SERVO, PRU::COUNTERCLOCKWISE, 500);
	//sleep(1);
	//hardware.testRun(PRU::ZENITH_SERVO, PRU::COUNTERCLOCKWISE, 3200);
	//sleep(1);
	//hardware.goPos(PRU::ZENITH_SERVO, PRU::CLOCKWISE, 2199);



	return 0;
}
