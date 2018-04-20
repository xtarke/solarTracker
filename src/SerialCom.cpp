/*
 * SerialCom.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: xtarke
 */

#include "SerialCom.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <time.h>

#include <iostream>
#include <string>

SerialCom::SerialCom(const char* devFilename) {

	/* Try to open GPS device */
	fd = open(devFilename, O_RDWR | O_NOCTTY );

	if (fd < 0) {perror(devFilename); exit(-1); }

	tcgetattr(fd,&oldtio); /* save current serial port settings */
	bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

	/*
	BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
	CRTSCTS : output hardware flow control (only used if the cable has
			all necessary lines. See sect. 7 of Serial-HOWTO)
	CS8     : 8n1 (8bit,no parity,1 stopbit)
	CLOCAL  : local connection, no modem contol
	CREAD   : enable receiving characters
	*/
	newtio.c_cflag = B4800 | CRTSCTS | CS8 | CLOCAL | CREAD;

	/*
	IGNPAR  : ignore bytes with parity errors
	ICRNL   : map CR to NL (otherwise a CR input on the other computer
			will not terminate input)
	otherwise make device raw (no other input processing)
	*/
	newtio.c_iflag = IGNPAR | ICRNL;

	/*
	Raw output.
	*/
	newtio.c_oflag = 0;

	/*
	ICANON  : enable canonical input
	disable all echo functionality, and don't send signals to calling program
	Canonical input should be used since NEMA GPS strings terminate with \n\r 	*/
	newtio.c_lflag = ICANON;

	/*
	initialize all control characters
	default values can be found in /usr/include/termios.h, and are given
	in the comments, but we don't need them here
	*/
	newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
	newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	newtio.c_cc[VERASE]   = 0;     /* del */
	newtio.c_cc[VKILL]    = 0;     /* @ */
	newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
	newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	newtio.c_cc[VSWTC]    = 0;     /* '\0' */
	newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
	newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	newtio.c_cc[VEOL]     = 0;     /* '\0' */
	newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	newtio.c_cc[VEOL2]    = 0;     /* '\0' */

	/*
	now clean the modem line and activate the settings for the port
	*/
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);

}

SerialCom::~SerialCom() {
	/* Restore old port config */
	tcsetattr(fd,TCSANOW,&oldtio);

	close(fd);
}


int SerialCom::readData(std::string &serialCanData){

	/* skm53-gps Module sends 6 NEMA frames             *
	 * Loop interates 12 since each line has \n\r       *
	 * and read returns for \n and \r in Canonical mode */
	for (int i = 0; i < 16;i++) {
		readGpsLine(serialCanData);
	}

	#ifdef DEBUG
	std::cout << serialCanData << std::endl;
	#endif

	return 0;
}


void SerialCom::readGpsLine(std::string &serialCanData){

	int ret  = 0;
	char buffer[256];

	ret = read(fd,buffer,255);
	/* set end of string */
	buffer[ret]=0;

	/* Adds new GPS data to string */
	serialCanData += buffer;
}

