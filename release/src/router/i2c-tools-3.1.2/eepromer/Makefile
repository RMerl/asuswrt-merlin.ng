#eepromer  Makefile

CFLAGS = -O2 -I../include -Wall

all:  eepromer eeprom eeprog

eepromer:  eepromer.o

eeprom:  eeprom.o

eeprog: eeprog.o 24cXX.o

clean:
	rm -rf *~ *.o eepromer eeprom eeprog
