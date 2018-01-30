#!/bin/sh

scp ./solarTracker  debian@192.168.6.2:/home/debian/solar/
scp ./pulses.bin  debian@192.168.6.2:/home/debian/solar/
#scp ./src/*.cpp alarm@192.168.7.2:/home/alarm/solar/src/
#scp ./src/*.h alarm@192.168.7.2:/home/alarm/solar/src/
#scp ./src/spa/*.h alarm@192.168.7.2:/home/alarm/solar/src/spa/
#scp ./src/spa/*.c alarm@192.168.7.2:/home/alarm/solar/src/spa/
#scp ./src/pru/*.p debian@192.168.6.2:/home/debian/solar/
