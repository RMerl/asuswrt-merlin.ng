#
# Makefile for the Epon Stack driver.
#
-include $(src)/../make.epon

obj-$(CONFIG_BCM_EPON_STACK) += bcmepon.o

# Files belong to 6858 and 6856
ifeq ("$(BRCM_FULL_CHIP_NAME)" ,"6858A0")
CHIP_NAME:=BCM6858A0
else ifneq (,$(findstring 6858B,$(BRCM_FULL_CHIP_NAME)))
CHIP_NAME:=BCM6858B0
else ifeq ("$(BRCM_FULL_CHIP_NAME)" ,"6846")
CHIP_NAME:=BCM6846A0
else ifeq ("$(BRCM_FULL_CHIP_NAME)" ,"6856A0")
CHIP_NAME:=BCM6856A0
else ifeq ("$(BRCM_FULL_CHIP_NAME)" ,"6878")
CHIP_NAME:=BCM6878A0
else ifeq ("$(BRCM_FULL_CHIP_NAME)" ,"63158B0")
CHIP_NAME:=BCM63158
endif

DRIVER_FILES:=$(shell cd $(src) && find driver/$(CHIP_NAME) -type f -name '*.c')

bcmepon-objs += $(patsubst %.c, %.o, $(DRIVER_FILES))

bcmepon-objs += AeDriver.o
bcmepon-objs += mac_drv_ae.o
bcmepon-objs += phy_drv_int_ae.o
bcmepon-objs += Lif.o
bcmepon-objs += EponMac.o
ifeq ("$(CONFIG_BCM96858)" ,"y")
bcmepon-objs += Xif.o
endif
ifeq ("$(CONFIG_BCM96856)" ,"y")
bcmepon-objs += Xif.o
endif
ifeq ("$(CONFIG_BCM963158)" ,"y")
bcmepon-objs += Xif.o
endif
bcmepon-objs += ../shared/EponTypes.o
bcmepon-objs += ../shared/EponDriver.o

ifeq ("$(EPON_NORMAL_MODE)" ,"y")
    bcmepon-objs += PonManager.o
    bcmepon-objs += EponCmds.o
    bcmepon-objs += Nco.o
    # Files shared with other EPON platform
    bcmepon-objs += ../shared/OntDirector.o
    bcmepon-objs += ../shared/EponUser.o
    bcmepon-objs += ../shared/EponTimer.o
    bcmepon-objs += ../shared/PonConfigDb.o
    bcmepon-objs += ../shared/PonMgrFec.o
    bcmepon-objs += ../shared/rdpa_epon.o
    bcmepon-objs += ../shared/Stream.o
    bcmepon-objs += ../shared/Mpcp.o
    bcmepon-objs += ../shared/OntmMpcp.o
    bcmepon-objs += ../shared/CtcStats.o
    bcmepon-objs += ../shared/CtcAlarms.o
    bcmepon-objs += ../shared/PonMgrEncrypt.o
    bcmepon-objs += ../shared/EponCtrl.o
    bcmepon-objs += ../shared/EponFrame.o
    bcmepon-objs += ../shared/Holdover.o
    bcmepon-objs += ../shared/EponStats.o
    bcmepon-objs += ../shared/OptCtrl.o
    bcmepon-objs += ../shared/OpticCmds.o
    bcmepon-objs += ../shared/OsAstMsgQ.o
    bcmepon-objs += ../shared/ProtSwitch.o
    bcmepon-objs += ../shared/EponRogueDrv.o
    bcmepon-objs += ../shared/PowerSaving.o
    bcmepon-objs += ../shared/mac_drv_epon.o
    ifeq ("$(CONFIG_EPON_CLOCK_TRANSPORT)" ,"y")
    bcmepon-objs += ../shared/ClockTransport.o
    endif
    ifneq ($(strip $(CONFIG_COMPAT)),)
    bcmepon-objs += ../shared/EponCtrlCompat.o
    endif
endif

EXTRA_CFLAGS += $(EXTRA_CFLAGS_BDMF)
EXTRA_CFLAGS += -DUSE_BDMF_SHELL
EXTRA_CFLAGS += -DPHYS_ADDR_64BIT

EXTRA_CFLAGS += -I$(src)/../shared/include
EXTRA_CFLAGS += -I$(src)/include
EXTRA_CFLAGS += -I$(src)/include/platform
EXTRA_CFLAGS += -I$(src)/driver/$(CHIP_NAME)/hal
EXTRA_CFLAGS += -I$(src)/driver/$(CHIP_NAME)/ru_gen
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH) 
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/rdpa_gpl_ext/impl1/include/
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/rdpa_gpl/impl1/include/
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared

ifeq ("$(CONFIG_BCM_PON)" ,"y")
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/drv/egphy
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/drv
endif


EXTRA_CFLAGS += -DBDMF_DRIVER_GPL_LAYER

EXTRA_CFLAGS += -Werror -Wall -Wno-array-bounds -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
EXTRA_CFLAGS += -DLINUX_KERNEL -DINCLUDE_LOGS -DDBA_DEBUG_STATISTICS
EXTRA_CFLAGS += -DPLL_PPM_ADJ

-include $(TOPDIR)/Rules.make

clean:
	rm -rf `find . -name '*.[o,d]'`
	rm -rf `find ../shared/ -name '*.[o,d]'`
	rm -rf `find . -name '*~'`
	rm -rf `find ../shared/ -name '*~'`
	rm -rf `find . -name '*.ko'`
	rm -rf `find ../shared/ -name '*.ko'`
	rm -rf `find . -name '.*.cmd'`
	rm -rf `find ../shared/ -name '.*.cmd'`
	rm -rf $(MODULE_NAME).mod.c modules.order Module.symvers .tmp_versions

