/*
 * SerialCom.h
 *
 *  Created on: Nov 23, 2017
 *      Author: xtarke
 */

#ifndef SERIALCOM_H_
#define SERIALCOM_H_

#include <termios.h>
#include <string>
#include <vector>

class SerialCom {
public:
	SerialCom(const char* devFilename);
	virtual ~SerialCom();

	int readData(std::string &serialCanData);


private:
	/* File description for the serial port */
	int fd;
	struct termios oldtio,newtio;

	void readGpsLine(std::string &serialCanData);

};

#endif /* SERIALCOM_H_ */
