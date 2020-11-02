#
# Makefile for the bcmlibs module
#

ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
    EXTRA_CFLAGS += -DBCM_PKTFWD
	# Enable credit based Host Flow Control
	ifneq ($(strip $(CONFIG_BCM_PKTFWD_FLCTL)),)
		EXTRA_CFLAGS += -DBCM_PKTFWD_FLCTL
	endif
endif # CONFIG_BCM_PKTFWD

bcmlibs-objs   := bcmlibs_module.o bit_pool_util.o
bcmlibs-objs   += bcm_pktfwd.o

obj-$(CONFIG_BCM_LIBS) += bcmlibs.o

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

