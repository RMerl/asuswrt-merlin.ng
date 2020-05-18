
CC = gcc

# X86
#COPTS = -I../include -g -O2 -mmmx -msse
#PLATFORM = X86
DIS = objdump -d -S

# EMULATED
COPTS = -I../include -D__SIMD_EMU__ -g -O2
PLATFORM = emu

default: test_simd.c emulated
	${CC} ${COPTS} -o test_simd_${PLATFORM} test_simd.c

emulated: test_simd.c
	${CC} ${COPTS} -D__SIMD_EMU__ -o test_simd_emu test_simd.c

xdiff: default emulated
	${DIS} test_simd_${PLATFORM} > test_simd_${PLATFORM}.dis
	${DIS} test_simd_emu > test_simd_emu.dis
	xdiff test_simd_emu.dis test_simd_${PLATFORM}.dis

clean:
	rm test_simd_${PLATFORM} test_simd_emu *.dis
