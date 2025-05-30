# File: bcmdrivers/opensource/net/xtmrt/Makefile
#
# Makefile for the BCM63xx bcmxtmrt driver.
#

obj-$(CONFIG_BCM_XTMRT) += bcmxtmrtdrv.o

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63178_),)
ifneq ($(CONFIG_BCM_ARCHER),)
EXTRA_CFLAGS += -DCONFIG_BCM_ARCHER
bcmxtmrtdrv-objs := bcmxtmrt.o bcmxtmrtbond.o xtmrt_cfg.o xtmrt_archer.o
else
bcmxtmrtdrv-objs := bcmxtmrt.o bcmxtmrtbond.o xtmrt_cfg.o xtmrt_bpm.o xtmrt_iq.o
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_63158_63146_),)
bcmxtmrtdrv-objs := bcmxtmrt.o bcmxtmrtbond.o xtmrt_cfg.o xtmrt_runner.o xtmrt_runner_ex.o
endif

bcmxtmrtdrv-objs += xtmrt_dbg.o

EXTRA_CFLAGS += ${BUILD_DSL_FLAGS}

ifeq ($(strip $(BRCM_DRIVER_ADSL)),)
EXTRA_CFLAGS += -DNO_XDSL_MODULE
endif

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) \
-I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -DDPERF_MON_BONDING_US -DDPTMBOND_US_PRIO_TRAFFIC_SPLIT
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/xtmrt/impl$(CONFIG_BCM_XTMRT_IMPL)

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_63158_63146_),)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH) 
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_63158_),)
EXTRA_CFLAGS += -DHOST_PATH_ADD_BOND_HDR
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63146_),)
EXTRA_CFLAGS += -DHW_ACCEL_MIRRORING
endif

EXTRA_CFLAGS += -Werror -Wfatal-errors 
ifeq "$(BRCM_PHY_LOOPBACK)" "y"
EXTRA_CFLAGS += -DPHY_LOOPBACK
endif

ifneq ($(strip $(BCM_RDPA_TM_C)),)
CFLAGS += -DTM_C_CODE
endif

EXTRA_CFLAGS += -DDUMP_DATA -DDFRAGMENT_ALIGNMENT_32BIT_DEBUG

#Uncomment the below line to enable XTM_DEBUG_PHY_VECTOR_PKTS
#EXTRA_CFLAGS += -DXTM_DEBUG_PHY_VECTOR_PKTS

clean:
	rm -f core *.o *.a *.s
