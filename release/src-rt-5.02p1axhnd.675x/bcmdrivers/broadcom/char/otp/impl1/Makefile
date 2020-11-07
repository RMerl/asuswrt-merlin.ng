# Makefile for the bcm963xx board drivers
#


obj-$(CONFIG_BCM_OTP) := otp.o 
bcmotp-objs += otp.o

EXTRA_CFLAGS += -I. -I$(src)/.  -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -g


-include $(TOPDIR)/Rules.make


clean:
	rm -f core *.ko *.o *.a *.s

bcm_headers_install:
	echo BCMLIBS installing header
	echo cp -f --no-preserve=mode -r ./*.h $(INC_BCMDRIVER_PATH)/include/
	cp -f --no-preserve=mode -r ./*.h $(INC_BCMDRIVER_PATH)/include/

.PHONY: bcm_headers_install
