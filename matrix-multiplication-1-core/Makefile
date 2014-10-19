H_CXX     =g++
H_CXXFLAGS=-Wall -Wextra -std=c++1y -DE_EXECUTABLE=\"$(E_EXECUTABLE)\"
H_INCLUDE =-I ${EPIPHANY_HOME}/tools/host/include
H_LD      =$(H_CXX)
H_LIBS    =-L ${EPIPHANY_HOME}/tools/host/lib -le-hal

E_CXX     =e-g++
#E_CXXFLAGS=-Wall -Wextra -std=c++1y
E_CXXFLAGS=-Wall -Wextra -std=c++1y -O3
E_INCLUDE =
E_LD      =$(E_CXX)
E_LFLAGS  =-T ${EPIPHANY_HOME}/bsps/current/internal.ldf
E_LIBS    =-le-lib
E_OBJCOPY =e-objcopy --srec-forceS3 --output-target srec

PROJECT_NAME=matrix-multiplication-1-core

H_EXECUTABLE=$(PROJECT_NAME)-host
H_SOURCES=host.cpp
# suffix madness probably needed to discern between epiphany and host compilation
H_OBJECTS=$(patsubst %.cpp,%-host.o,$(H_SOURCES))

E_EXECUTABLE=$(PROJECT_NAME)-epiphany.srec
E_ELF=$(patsubst %.srec, %.elf, $(E_EXECUTABLE))
E_SOURCES=epiphany.cpp
E_OBJECTS=$(patsubst %.cpp,%-epiphany.o,$(E_SOURCES))

all: $(H_EXECUTABLE) $(E_EXECUTABLE)

$(E_EXECUTABLE): $(E_ELF)
	$(E_OBJCOPY) $^ $@

$(E_ELF): $(E_OBJECTS)
	$(E_LD) $(E_LFLAGS) -o $@ $^ $(E_LIBS)

%-epiphany.o: %.cpp
	$(E_CXX) $(E_CXXFLAGS) $(E_INCLUDE) -o $@ -c $<

$(H_EXECUTABLE): $(H_OBJECTS)
	$(H_LD) $(H_LFLAGS) -o $@ $^ $(H_LIBS)

%-host.o: %.cpp
	$(H_CXX) $(H_CXXFLAGS) $(H_INCLUDE) -o $@ -c $<

.depend-host: $(H_SOURCES)
	$(H_CXX) $(H_CXXFLAGS) $(H_INCLUDE) -MM $^ | sed s/.o:/-host.o:/ > $@

.depend-epiphany: $(E_SOURCES)
	$(E_CXX) $(E_CXXFLAGS) $(E_INCLUDE) -MM $^ | sed s/.o:/-epiphany.o:/ > $@

-include .depend-host
-include .depend-epiphany

.PHONY: clean
clean:
	rm -f *.o .depend-host .depend-epiphany $(H_EXECUTABLE) $(E_ELF) $(E_EXECUTABLE)

