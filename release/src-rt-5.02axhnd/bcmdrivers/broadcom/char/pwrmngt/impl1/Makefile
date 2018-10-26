# File: bcmdrivers/char/pwrmngt/bcm963xx/Makefile
#
# Makefile for the Power Management Module
#
ifneq ($(PREBUILT_EXTRAMOD),1)
pwrmngtd-objs   := pwrmngtdrv.o
pwrmngtd-objs   += pwrmngt.o

ifneq ($(strip $(BUILD_BMU)),)
pwrmngtd-objs   += bmu.o    
endif

include_avs = 

ifneq ($(CONFIG_BCM_MoCA_AVS),)
include_avs = yes
endif

ifneq ($(CONFIG_BCM_AVS_PWRSAVE),)
include_avs = yes
endif

ifdef include_avs
pwrmngtd-objs   += pwrmngtavs.o

COMPILE_40NM_AVS=n
COMPILE_65NM_AVS=n

ifeq ($(strip $(BRCM_CHIP)),63268)
COMPILE_65NM_AVS=y
endif

# Checks for 40NM compilation
ifneq ($(strip $(CONFIG_BCM_MoCA_AVS)),)
COMPILE_40NM_AVS=y
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/board/bcm963xx/impl1/bbsi
EXTRA_CFLAGS += -DBCHP_CHIP=6802 -DBCHP_FAMILY=9999
else
EXTRA_CFLAGS += -DBCHP_CHIP=$(BRCM_CHIP) -DBCHP_FAMILY=9999
endif


ifeq ($(strip $(BRCM_CHIP)),6318)
COMPILE_40NM_AVS=y
endif

ifeq ($(strip $(COMPILE_65NM_AVS)),y)
pwrmngtd-objs += avs_65/avs_65.o
EXTRA_CFLAGS += -DRUN_AVS_65
endif

ifeq ($(strip $(COMPILE_40NM_AVS)),y)
pwrmngtd-objs += avs_40/avs_start.o
EXTRA_CFLAGS += -DRUN_AVS_40
endif

endif

ifneq (${BRCM_CPU_FREQ_PWRSAVE},)
EXTRA_CFLAGS += -DCONFIG_BCM_CPU_FREQ_PWRSAVE
endif

EXTRA_CFLAGS += -I.
EXTRA_CFLAGS += -I./avs_40
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/enet/shared 
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc
EXTRA_CFLAGS += -fno-exceptions
EXTRA_CFLAGS += -Werror -Wfatal-errors

else
pwrmngtd-objs += pwrmngtd$(PRBM_EXT).o
endif
obj-$(CONFIG_BCM_PWRMNGT) += pwrmngtd.o

 #-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s avs*/*.o *.ko

