#
# Makefile for the BCM Lport Serdes module
#

#obj-$(BCM_LPORT_SERDES) += lport_serdes.o
obj-n += lport_serdes.o

MERLIN_SDK_PATH = ../../../../../shared/broadcom/drivers/merlin
lport_serdes-objs := lport_serdes_wrapper.o
lport_serdes-objs += $(MERLIN_SDK_PATH)/merlin_lport_drv.o
lport_serdes-objs += $(MERLIN_SDK_PATH)/merlin_serdes_api.o
lport_serdes-objs += $(MERLIN_SDK_PATH)/merlin_serdes_debug.o

EXTRA_CFLAGS += -I$(SHARED_DIR)/broadcom/drivers/merlin
EXTRA_CFLAGS += -I$(SHARED_DIR)/broadcom/drivers/merlin/sdk
EXTRA_CFLAGS += -I$(SHARED_DIR)/broadcom/drivers/merlin/sdk/include
EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/drv/lport
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(INC_BDMF_PATH)/framework -I$(INC_BDMF_PATH)/system 
EXTRA_CFLAGS += -I$(INC_BDMF_PATH)/system/linux -I$(INC_BDMF_PATH)/system/linux/cm 
EXTRA_CFLAGS += -I$(INC_BDMF_PATH)/shell
EXTRA_CFLAGS += -DUSE_BDMF_SHELL


-include $(TOPDIR)/Rules.make

clean:
	rm -rf core *.o *.a *.s .*.cmd *.ko $(MERLIN_SDK_PATH)/*.o $(MERLIN_SDK_PATH)/.*.cmd

# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precidence
%.o : %.o_tmp
	$(Q)mv $< $@

%.o_tmp : %.$(BCMARCH).o_saved
	@echo "Using $<"
	$(Q)cp $< $@
