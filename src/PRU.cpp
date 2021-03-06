/*
 * PRU.cpp
 *
 *  Created on: Nov 28, 2017
 *      Author: xtarke
 */

#include "PRU.h"
#include <stdio.h>
#include <stdlib.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <iostream>

#define PRU_NUM	0   // using PRU0
// #define DEBUG

PRU::PRU(std::string configPath) {
	int ret;

	myconfigPath = configPath;

	/* Initialize structure used by prussdrv_pruintc_intc
	PRUSS_INTC_INITDATA is found in pruss_intc_mapping.h */
	pruss_intc_initdata = PRUSS_INTC_INITDATA;

	prussdrv_init();

	// Allocate and initialize memory
	ret = prussdrv_open (PRU_EVTOUT_0);

	if (ret != 0) {
		std::cerr << "Error execution prussdrv_open in PRU modules. Aborting ...\n";
		exit(EXIT_FAILURE);
	}

	// Map PRU's interrupts
	ret = prussdrv_pruintc_init(&pruss_intc_initdata);

	if (ret < 0)
		std::cerr << "prussdrv_pruintc_init: " << ret << std::endl;;

	// MAP PRU local sram data
	ret = prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, &pru0DataMemory);
	pru0DataMemory_int = (unsigned int *) pru0DataMemory;

	if (ret < 0)
		std::cerr << "prussdrv_map_prumem: " << ret << std::endl;;

}

PRU::~PRU() {
	// Disable PRU and close memory mappings
	prussdrv_pru_disable(PRU_NUM);
	prussdrv_exit ();
}


void PRU::goPos(enum servoID servo, enum direction dir, uint32_t pulses){
	int ret;
	struct PRUMessage prusData;

	prusData.servoId = servo;		/* 	(0 -> ZENITH_SERVO, 1 -> AZIMUTH_SERVO)      */
	prusData.zenithDirection = dir;		    /*  (0-> CLOCKWISE_Z,   1 -> COUNTERCLOCKWISE_Z) */
	prusData.azimuthDirection = dir;		    /*	(0-> CLOCKWISE_A,   1 -> COUNTERCLOCKWISE_A) */
	prusData.zenithPulses = pulses;       /*  number of pulses Z  */
	prusData.azimuthPulses = pulses;       /*  number of pulses A  */

	/* Pass Servo moviment do PRU */
	ret = prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, (const uint32_t *)&prusData, sizeof(prusData));

	if (ret < 0)
		std::cerr << "prussdrv_pru_write_memory: " << ret << std::endl;;

	// Load and execute the PRU program on the PRU

	std::string fileName = myconfigPath + "/pulses.bin";

	ret = prussdrv_exec_program (PRU_NUM, fileName.c_str());

	if (ret < 0)
		std::cerr << "prussdrv_exec_program: " << ret << std::endl;;

	// Wait for event completion from PRU, returns the PRU_EVTOUT_0 number
	int n = prussdrv_pru_wait_event (PRU_EVTOUT_0);

#ifdef DEBUG
	printf("EBB PRU program completed, event number %d.\n", n);
#endif

	prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

}

