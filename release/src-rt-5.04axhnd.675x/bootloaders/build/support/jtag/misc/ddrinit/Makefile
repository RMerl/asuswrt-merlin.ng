CROSS_COMPILE?=/opt/toolchains//crosstools-arm-gcc-9.2-linux-4.19-glibc-2.30-binutils-2.32/usr/bin/arm-buildroot-linux-gnueabi
TEXT_ADDR=0xfffe0000

all: ddrinit

ddrinit:
	$(CROSS_COMPILE)-gcc  ddrinit.c -nostdlib  -Wl,-z,max-page-size=4096 -Ttext=$(TEXT_ADDR) -g -o ddrinit -O0 init.s

clean:
	-rm ddrinit *.o

.PHONY: 63148 63138

63148 63138: ddrinit
