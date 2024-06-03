# vfbnize utility

CFLAGS ?= -Werror -Wall -Wno-unused-value -I.

vfbnize: vfbnize.o iniparser/iniparser.o iniparser/dictionary.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -fr vfbnize.o vfbnize iniparser/iniparser.o iniparser/dictionary.o

