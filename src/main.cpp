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
		printf("You must run this program as root. Exiting.\n");
		exit(EXIT_FAILURE);
	}

//	Magnetometer bussula;
//
//
//	for (int i=0; i < 50; i++){
//		bussula.refresh();
//		sleep(1);
//	}


	SolarTracker solarTracaker("/dev/ttyS1");
	//PRU hardware;

	//hardware.testRun(PRU::AZIMUTH_SERVO, PRU::COUNTERCLOCKWISE, 2);
	//usleep(10);
	//hardware.testRun(PRU::ZENITH_SERVO, PRU::CLOCKWISE, 10);
	//sleep(1);
//	hardware.testRun(PRU::AZIMUTH_SERVO, PRU::CLOCKWISE, 500);
	//sleep(1);


	return 0;
}
