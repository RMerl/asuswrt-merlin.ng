# File: bcmdrivers/opensource/net/score/impl1/Makefile
#
# Makefile for the SCORE module
#
bcm_score-objs := score_main.o score_procfs.o score_qstat.o score_ct.o score_hw.o score_queue.o
obj-$(CONFIG_BCM_SCORE) += bcm_score.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -I$(INC_BCMLIBS_PATH)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) $(EXTRA_CFLAGS_RDPA) $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/archer/impl1
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/bcm963xx
EXTRA_CFLAGS += $(EXTRA_INCLUDES) $(INC_RDP_FLAGS)

EXTRA_CFLAGS += -Werror

clean:
	rm -f core *.ko *.o *.a *.s

