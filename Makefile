CXX = arm-linux-gnueabihf-g++
CC = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -c -O3 -I ../am335x_pru_package/pru_sw/app_loader/include
CXXFLAGS = -c -Wall -O3  -I ../am335x_pru_package/pru_sw/app_loader/include
LDFLASG = -L ../am335x_pru_package/pru_sw/app_loader/lib/  -lm -pthread -lprussdrv 


CSOURCES=./src/spa/spa.c

CPPSOURCES= ./src/main.cpp \
            ./src/GPS.cpp \
            ./src/SerialCom.cpp \
            ./src/PRU.cpp \
            ./src/SolarTracker.cpp

COBJECTS=$(CSOURCES:.c=.o)                              
CPPOBJECTS = $(CPPSOURCES:.cpp=.o) 
EXECUTABLE=solarTracker


all: $(EXECUTABLE)

$(EXECUTABLE): $(COBJECTS) $(CPPOBJECTS)
	echo $(CXX) $(COBJECTS) $(CPPOBJECTS) -o $@
	$(CXX) $(COBJECTS) $(CPPOBJECTS) $(LDFLASG) -o $@

.c.o: *.h
	$(CC) $(CFLAGS) $< -o $@

.cpp.o: *.h
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(COBJECTS)
	rm -rf $(CPPOBJECTS)
	rm -rf $(EXECUTABLE)
