#ifndef I2CMODULE_H
#define I2CMODULE_H

#include <stdint.h>

class i2cModule
{
private:
    const char *devFileName = "/dev/i2c-2";
    uint8_t i2cAddr;
    int fd;

public:
    i2cModule();
    ~i2cModule();

    int sendDev(uint8_t *buffer, int length);
    int readDev(uint8_t *buffer, int lenght);

    int openDev(uint8_t i2cAddr_);
    int closeDev();

    int change_i2cAddr(uint8_t _addr);

    uint8_t get_i2cAddr() {return i2cAddr;}
};

#endif // I2CMODULE_H
