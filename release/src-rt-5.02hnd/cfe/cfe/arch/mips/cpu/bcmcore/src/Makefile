
ALLOBJS += bcmcore_cpuinit.o bcmcore_l1cache.o
ifeq ($(strip ${IKOS_FAST_CACHE_INIT}),1)
ALLOBJS +=               \
	production_cache.o
endif

ifeq ($(strip ${CFG_RAMAPP}),1)
ALLOBJS += bcmcore_arena.o
endif

CFLAGS += -mips32 -D_MIPSREGS32_ -mno-abicalls 
# CFLAGS += -mips2 -D_MIPSREGS32_

