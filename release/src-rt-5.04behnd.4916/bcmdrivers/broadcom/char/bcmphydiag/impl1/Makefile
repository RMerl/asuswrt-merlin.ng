obj-$(CONFIG_BCM_PHY_DIAG) += bcmdiagdd.o 

bcmdiagdd-objs += BcmDiagDrv.o BcmDiag.o

EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += ${BUILD_DSL_FLAGS}
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)

ifneq "$(findstring 63138,$(BRCM_CHIP))$(findstring 63148,$(BRCM_CHIP))$(findstring 63158,$(BRCM_CHIP))$(findstring 63178,$(BRCM_CHIP))" ""
EXTRA_CFLAGS += -DADSLDRV_LITTLE_ENDIAN
endif

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean
