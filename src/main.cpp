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
#include <getopt.h>
#include <string>

#include "SolarTracker.h"
#include "PRU.h"

#include "Magnetometer.h"

using namespace std;

static const struct option long_options[] = {
    { "config", required_argument, NULL, 'c'},
    { "help", no_argument, NULL, 'h'},
    { NULL, no_argument, NULL, 0}
};

static void usage(void);
static int parse_command_line(int argc, char** argv);

string configPath;

int main(int argc, char *argv[]) {


	if (getuid() != 0){
		std::cout << "You must run this program as root. Exiting.\n";
		exit(EXIT_FAILURE);
	}

	parse_command_line(argc, argv);

	SolarTracker solarTracaker("/dev/ttyS1", configPath);
	//PRU hardware;


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

/* Usage params */
static void usage(void) {
	std::cout << "Solar Tracker" << endl;
	std::cout << "Usage:" << endl << endl;
	std::cout << "\t\t-c (--config) < configuration files path >" << endl;


    exit(EXIT_FAILURE);
}

static int parse_command_line(int argc, char** argv) {
    int c;

    if (argc < 2){
    	usage();
    	exit(0);
    }

    while (1) {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "c:h",
                long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:
                break;

            case 'c':
            	configPath = optarg;
                break;

            case 'h':
                usage();
                exit(0);
        }
    }



    return 0;
}
