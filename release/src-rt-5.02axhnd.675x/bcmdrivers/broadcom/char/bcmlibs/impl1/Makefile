#
# Makefile for the bcmlibs module
#

ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
    EXTRA_CFLAGS += -DBCM_PKTFWD
endif # CONFIG_BCM_PKTFWD

obj-$(CONFIG_BCM_LIBS) += bcmlibs.o
ifneq ($(PREBUILT_EXTRAMOD),1)
bcmlibs-objs   := bcmlibs_module.o bit_pool_util.o
bcmlibs-objs   += bcm_pktfwd.o
else
bcmlibs-objs := bcmlibs$(PRBM_EXT).o
endif

EXTRA_CFLAGS += -I. -I$(src)/../include/
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

EXTRA_CFLAGS += -Werror -Wfatal-errors


clean:
	rm -f core *.ko *.o *.a *.s


bcm_headers_install:
	echo BCMLIBS installing header
	echo cp -f --no-preserve=mode -r ../include/* $(INC_BCMDRIVER_PATH)/include/
	cp -f --no-preserve=mode -r ../include/* $(INC_BCMDRIVER_PATH)/include/

.PHONY: bcm_headers_install

