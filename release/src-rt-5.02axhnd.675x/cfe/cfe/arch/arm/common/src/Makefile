#
# This is just a Makefile fragment -- it is included by the master
# makefile, cfe.mk
#
# This file should just append object file names to "ALLOBJS",
# but since it is mean to be linked *first*, it will append
# modules to "CRT0OBJS"
#

ifndef INIT_ARM
ifeq ($(strip ${CFG_ARMV8_AARCH64}),1)
INIT_ARM = init_arm_aarch64.o exception_aarch64.o secondary_entry_aarch64.o
else
INIT_ARM = init_arm.o exception.o secondary_entry.o
endif
endif

CRT0OBJS += $(INIT_ARM)

ifeq ($(strip ${CFG_RAMAPP}),1)
ALLOBJS += arm_arena.o exchandler.o lib_setjmp.o disasm.o
endif

#
# Use the relocatable self-write engine
#
ifneq ($(strip ${CFG_FLASH}),1)
#ALLOBJS += dev_flash_all.o dev_flashop_engine.o
endif

makereg : ${TOP}/hosttools/makereg.c
	gcc -o makereg ${TOP}/hosttools/makereg.c

%.inc : %.regdef makereg
	./makereg $< $@ 

ui_soccmds.o : ${CPU_SRC}/ui_soccmds.c ${CPU}_socregs.inc

vapi.o : ${TOP}/verif/vapi.S sb1250_socregs.inc
