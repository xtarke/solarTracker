# CC = g++
# CFLAGS = -Wall -O3 
# LDLIBS = 
# LDFLAGS = -lm -lpthread
# 
# DEPS = ./src/GPS.h ./src/SerialCom.h ./src/SolarTracker.h ./src/spa/spa.h
# OBJ = ./src/main.cpp ./src/GPS.cpp ./src/SerialCom.cpp ./src/SolarTracker.cpp ./src/spa/spa.c
# 
# %.o: %.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CFLAGS)
# 
# solarTracker: $(OBJ)
# 	$(CC) -o $@ $^  $(LDFLAGS) $(CFLAGS)
# 
# .PHONY: clean
# 
# clean:
# 	rm -f *.o *.hex ctrl_Inmoov_Braco

CXX = g++
CC = gcc
CFLAGS = -Wall -c -O3
CXXFLAGS = -c -Wall -O3
LDFLASG = -lm -pthread

CSOURCES=./src/spa/spa.c

CPPSOURCES= ./src/main.cpp \
            ./src/GPS.cpp \
            ./src/SerialCom.cpp \
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
