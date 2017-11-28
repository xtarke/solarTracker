CXX = g++
CC = gcc
CFLAGS = -Wall -c -O0
CXXFLAGS = -c -Wall -O0
LDFLASG = -lm -pthread -lprussdrv


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
