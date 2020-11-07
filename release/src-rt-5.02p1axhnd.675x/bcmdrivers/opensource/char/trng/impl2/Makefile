#
# Makefile for the BCM63xx TRNG driver

obj-$(CONFIG_BCM_TRNG) += bcmtrng.o 
bcmtrng-objs := trng.o Sha256.o

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

