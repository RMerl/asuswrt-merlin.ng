# File: bcmdrivers/char/bpm/bcm963xx/Makefile
#
# Makefile for the BCM63xx BPM module
#

bcm_bpm-objs   := bpm.o
obj-$(CONFIG_BCM_BPM) += bcm_bpm.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)

EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared

EXTRA_CFLAGS += -Werror -Wfatal-errors 

%.o:
	cp $(obj)/prebuilt/*.o $(obj)/

clean:
	rm -f core *.ko *.o *.a *.s



