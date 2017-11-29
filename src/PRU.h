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

public:
	PRU();
	virtual ~PRU();

	enum servoID {ZENITH_SERVO, AZIMUTH_SERVO};
	enum direction {CLOCKWISE, COUNTERCLOCKWISE};

	void testRun(enum servoID servo, enum direction dir, uint32_t pulses);
};

#endif /* PRU_H_ */
