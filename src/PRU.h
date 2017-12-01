/*
 * PRU.h
 *
 *  Created on: Nov 28, 2017
 *      Author: xtarke
 */

#ifndef PRU_H_
#define PRU_H_

#include <prussdrv.h>
#include <stdint.h>


class PRU {
private:
	tpruss_intc_initdata pruss_intc_initdata;

	void *pru0DataMemory;
	unsigned int *pru0DataMemory_int;

	struct PRUMessage {
		uint32_t servoId;			/* 	(0 -> ZENITH_SERVO, 1 -> AZIMUTH_SERVO)      */
		uint32_t zenithDirection;	/*  (0-> CLOCKWISE_Z,   1 -> COUNTERCLOCKWISE_Z) */
		uint32_t azimuthDirection;  /*	(0-> CLOCKWISE_A,   1 -> COUNTERCLOCKWISE_A) */
		uint32_t zenithPulses;		/*  number of pulses Z  */
		uint32_t azimuthPulses;		/*  number of pulses A  */
	};


public:
	PRU();
	virtual ~PRU();

	enum servoID {ZENITH_SERVO, AZIMUTH_SERVO};
	enum direction {CLOCKWISE, COUNTERCLOCKWISE};

	void testRun(enum servoID servo, enum direction dir, uint32_t pulses);
};

#endif /* PRU_H_ */
