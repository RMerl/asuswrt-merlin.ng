# File: bcmdrivers/char/spudd/bcm963xx/Makefile
#
# Makefile for the BCM63xx IPSec SPU module
#

spudd-objs   := spudrv.o
spudd-objs += spuciphr.o
spudd-objs += spusha1.o
spudd-objs += spumd5.o
#spudd-objs += spuutils.o
spudd-objs += spuipsec.o

spudd-objs += linux/spu.o
spudd-objs += linux/spu_hash.o

ifneq ($(strip $(CONFIG_BCM_SPU_TEST)),)
spudd-objs += test/sputest.o
endif

obj-$(CONFIG_BCM_SPU) += spudd.o

EXTRA_CFLAGS += -I$(src) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -Werror -Wfatal-errors

# -include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s linux/*.o test/*.o

