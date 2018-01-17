#include "i2cmodule.h"
#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port

#include <iostream>

i2cModule::i2cModule()
{
    fd = -1;
}

int i2cModule::openDev(uint8_t i2cAddr_){

    i2cAddr = i2cAddr_;

    if ((fd = open(devFileName, O_RDWR)) < 0) {
        //ERROR HANDLING: you can check errno to see what went wrong
        std::cerr  << "Failed to open the i2c bus\n";
        return -1;
     }

     if (ioctl(fd, I2C_SLAVE, i2cAddr) < 0) {
         std::cerr << "Failed to acquire bus access and/or talk to slave.\n";
         return -1;
      }

     return 0;
}

int i2cModule::change_i2cAddr(uint8_t _addr){

    if (ioctl(fd, I2C_SLAVE, _addr) < 0) {
        std::cerr << "Failed to acquire bus access and/or talk to slave.\n";
        return -1;
     }

    i2cAddr = _addr;

    return 0;
}


int i2cModule::closeDev(){
    return close(fd);
}

i2cModule::~i2cModule(){
    if (fd > 0)
        closeDev();
}

int i2cModule::sendDev(uint8_t *buffer, int length){
    int bytesSent = 0;

    if (fd < 0){
        std::cerr << "i2c module not opened\n";
        return -1;
    }

    bytesSent = write(fd, buffer, length);

    if (bytesSent != length) {
        /* ERROR HANDLING: i2c transaction failed */
        std::cerr << "Failed to write to the i2c bus.\n";
    }

    return bytesSent;
}

int i2cModule::readDev(uint8_t *buffer, int lenght){
	int bytesRead = 0;

	if (fd < 0){
		std::cerr << "i2c module not opened\n";
		return -1;
	}

	bytesRead = read(fd,buffer,lenght);

	if (bytesRead != lenght) {
		/* ERROR HANDLING: i2c transaction failed */
		std::cerr << "Failed to write to the i2c bus.\n";
	}

	return bytesRead;
}

