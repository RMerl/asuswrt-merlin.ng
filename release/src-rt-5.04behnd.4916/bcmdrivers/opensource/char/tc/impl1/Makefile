#
# Makefile for the tc module
#

bcm_tc-objs   := tc_drv.o tc_ingress.o runner.o
obj-$(CONFIG_BCM_TC) += bcm_tc.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) $(EXTRA_CFLAGS_RDPA) $(INC_RDP_FLAGS)

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/bcm963xx
EXTRA_CFLAGS += $(EXTRA_INCLUDES) $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -Werror -Wall

clean:
	rm -f core *.ko *.o *.a *.s
