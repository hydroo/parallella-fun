H_CC     =gcc
H_CFLAGS =
H_INCLUDE=-I ${EPIPHANY_HOME}/tools/host/include
H_LD     =$(H_CC)
H_LIBS   =-L ${EPIPHANY_HOME}/tools/host/lib -le-hal -lpthread

E_CC     =e-gcc
E_CFLAGS =
E_INCLUDE=
E_LD     =$(E_CC)
E_LFLAGS =-T ${EPIPHANY_HOME}/bsps/current/internal.ldf
E_LIBS   =-le-lib -lm
E_OBJCOPY=e-objcopy --srec-forceS3 --output-target srec

PROJECT_NAME=matrix-multiplication

H_EXECUTABLE=$(PROJECT_NAME)-host
H_SOURCES=e-test.c
# suffix madness probably needed to discern between epiphany and host compilation
H_OBJECTS=$(patsubst %.c,%-host.o,$(H_SOURCES))

E_EXECUTABLE=$(PROJECT_NAME)-epiphany.srec
E_ELF=$(patsubst %.srec, %.elf, $(E_EXECUTABLE))
E_SOURCES=e-task.c test_common.c
E_OBJECTS=$(patsubst %.c,%-epiphany.o,$(E_SOURCES)) 

all: $(H_EXECUTABLE) $(E_EXECUTABLE)

$(E_EXECUTABLE): $(E_ELF)
	$(E_OBJCOPY) $^ $@

$(E_ELF): $(E_OBJECTS)
	$(E_LD) $(E_LFLAGS) -o $@ $^ $(E_LIBS)

%-epiphany.o: %.c
	$(E_CC) $(E_CFLAGS) $(E_INCLUDE) -o $@ -c $<

$(H_EXECUTABLE): $(H_OBJECTS)
	$(H_LD) $(H_LFLAGS) -o $@ $^ $(H_LIBS)

%-host.o: %.c
	$(H_CC) $(H_CFLAGS) $(H_INCLUDE) -o $@ -c $<

.PHONY: clean
clean:
	rm -f *.o $(H_EXECUTABLE) $(E_ELF) $(E_EXECUTABLE)

