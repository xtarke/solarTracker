/*
 * buttons.h
 *
 *  Created on: Oct 11, 2018
 *      Author: xtarke
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

void buttons_init();

enum BUTTONS {
	NONE,
	SELECT,
	LEFT,
	DOWN,
	UP,
	RIGHT,
};

uint8_t get_button(uint8_t old_press);

#endif /* BUTTONS_H_ */
