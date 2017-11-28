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

#define PRU_NUM	0   // using PRU0

PRU::PRU() {
	int ret;

    if (getuid()!=0){
        printf("You must run this program as root. Exiting.\n");
        exit(EXIT_FAILURE);
    }

   /* Initialize structure used by prussdrv_pruintc_intc
    PRUSS_INTC_INITDATA is found in pruss_intc_mapping.h */
   tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;


   prussdrv_init();

	// Allocate and initialize memory
	ret = prussdrv_open (PRU_EVTOUT_0);

	if (ret != 0) {
		perror("prussdrv_open:");
		exit(EXIT_FAILURE);
	}

	// Map PRU's interrupts
	prussdrv_pruintc_init(&pruss_intc_initdata);
	// Load and execute the PRU program on the PRU
	prussdrv_exec_program (PRU_NUM, "./pulses.bin");

	// Wait for event completion from PRU, returns the PRU_EVTOUT_0 number
	int n = prussdrv_pru_wait_event (PRU_EVTOUT_0);
	printf("EBB PRU program completed, event number %d.\n", n);

	// Disable PRU and close memory mappings
	prussdrv_pru_disable(PRU_NUM);
	prussdrv_exit ();

}

PRU::~PRU() {
	// TODO Auto-generated destructor stub
}

