
#***********************************************************************
#
#  Copyright (c) 2014  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/
CUR_DIR = $(shell pwd)
TOP_DIR :=$(subst /cfe/, /cfe/,$(CUR_DIR))
TOP_DIR :=$(word 1,$(TOP_DIR))
RDP_DIR :=$(TOP_DIR)/rdp
PROJECT_DIR := $(RDP_DIR)/projects/$(RDP_PROJECT)

-include $(RDP_DIR)/projects/$(RDP_PROJECT)/make.proj_flags

PROJ_DRV_SRC_LIST := $(PROJECT_DIR)/$(CHIP_TYPE)_gpl_sources.list

EXTRA_INCLUDE += -I$(TOP_DIR)/cfe/cfe/include 
EXTRA_INCLUDE += -I$(TOP_DIR)/shared/opensource/include/bcm963xx
EXTRA_INCLUDE += -I$(TOP_DIR)/bcmdrivers/opensource/include/bcm963xx
EXTRA_INCLUDE += -I$(TOP_DIR)/shared/opensource/include/rdp
EXTRA_INCLUDE += -I$(TOP_DIR)/cfe/cfe/arch/arm/common/include/
EXTRA_INCLUDE += -I$(TOP_DIR)/cfe/cfe/arch/arm/cpu/$(CPU)/include/
ifeq ("$(RDP_PROJECT)", "XRDP_CFE2")
EXTRA_INCLUDE += -I$(RDP_DIR)/projects/$(RDP_PROJECT)/drivers/rdd/BCM$(BRCM_CHIP)/auto
EXTRA_INCLUDE += -I$(RDP_DIR)/drivers/bdmf/shell 
EXTRA_INCLUDE += -I$(RDP_DIR)/drivers/bdmf/framework 
EXTRA_INCLUDE += -I$(RDP_DIR)/drivers/rdp_subsystem/BCM$(BRCM_CHIP)/autogen
EXTRA_INCLUDE += -I$(RDP_DIR)/drivers/rdp_subsystem/RU
EXTRA_INCLUDE += -I$(RDP_DIR)/drivers/rdp_subsystem/BCM$(BRCM_CHIP)/autogen/rnr 
endif

CFLAGS += -D_LIB_NO_MACROS_ -D__ARMEL__ -DBDMF_SESSION_H -D_BDMF_INTERFACE_H_ -D_BDMF_SYSTEM_H_ -DBDMF_MON_H -DCFG_RAMAPP=1 
CFLAGS += -include xrdp_cfe.h -include stdint.h -include stddef.h -include bdmf_errno.h  -Wall -Werror $(PROJ_CFLAGS) 
CFLAGS += -UUSE_BDMF_SHELL -DRU_USE_STDC -UVALIDATE_PARMS -UBDMF_DEBUG -UBDMF_SYSTEM_SIM -DBDMF_NO_TRACE -DRU_INCLUDE_DESC=0 -DRU_INCLUDE_FIELD_DB=0 
CFLAGS += -DRU_FIELD_CHECK_ENABLE=0 -DRU_INCLUDE_ACCESS=0 -DXRDP -DXRDP_NO_SWAP -D__BIT_TYPES_DEFINED__ -DFIRMWARE_LITTLE_ENDIAN -DCONFIG_GPL_RDP

vpath

ifeq ("$(RDP_PROJECT)", "XRDP_CFE2")
vpath %.c $(RDP_DIR)/drivers/rdp_subsystem/RU
vpath %.c $(RDP_DIR)/drivers/rdp_subsystem/BCM$(BRCM_CHIP)/autogen/fpm
vpath %.c $(RDP_DIR)/drivers/rdp_subsystem/BCM$(BRCM_CHIP)/autogen/rnr
vpath %.c $(RDP_DIR)/drivers/rdpa_gpl
endif
vpath %.c $(RDP_DIR)/drivers/rdp_subsystem/xrdp
vpath %.c $(RDP_DIR)/projects/$(RDP_PROJECT)/drivers/rdd
vpath %.c $(RDP_DIR)/projects/$(RDP_PROJECT)/drivers/rdp_subsystem/cpu
vpath %.c $(RDP_DIR)/projects/$(RDP_PROJECT)/drivers/rdp_subsystem/xrdp
vpath %.c $(RDP_DIR)/projects/$(RDP_PROJECT)/drivers/rdd/BCM$(BRCM_CHIP)/auto
vpath %.c $(RDP_DIR)/projects/$(RDP_PROJECT)/drivers/rdd/BCM$(BRCM_CHIP)

RDPOBJS  = access_logging.o 
RDPOBJS += data_path_init_basic.o 
RDPOBJS += rdp_cpu_ring.o 
RDPOBJS += rdp_platform.o 
RDPOBJS += rdd_ag_cpu_rx.o 
RDPOBJS += rdd_cpu_rx.o 
RDPOBJS += rdd_cpu_tx_ring.o 
RDPOBJS += rdd_map_auto.o 

ifeq ("$(RDP_PROJECT)", "XRDP_CFE2")
RDPOBJS += rdd_data_structures_auto.o
RDPOBJS += XRDP_FPM_AG.o 
RDPOBJS += XRDP_RNR_REGS_AG.o 
RDPOBJS += xrdp_drv_fpm_ag.o 
RDPOBJS += xrdp_drv_rnr_regs_ag.o 
RDPOBJS += rdpa_gpl_fpm.o
else
RDPOBJS += rdd_data_structures_auto_gpl_$(BRCM_CHIP).o
endif

LN = ln -sf
RM = rm -f

%.o : %.c
	$(GCC) $(CFLAGS) -ffunction-sections $(BASEFLAGS) $(EXTRA_INCLUDE) $<

.PHONY: cfe
cfe: $(RDPOBJS)
	$(GAR) cr libxrdp.a  $(RDPOBJS)

$(RDPOBJS): prepare_links

.PHONY: prepare_links
prepare_links:
	@$(foreach src,$(shell grep "drivers/rdd" $(PROJ_DRV_SRC_LIST)              | grep -v "^#" | grep -v "Makefile" | grep -v "\.c"),$(shell $(LN) $(RDP_DIR)/$(src) .))
	@$(foreach src,$(shell grep "drivers/rdp_subsystem" $(PROJ_DRV_SRC_LIST)    | grep -v "^#" | grep -v "Makefile" | grep -v "\.c"),$(shell $(LN) $(RDP_DIR)/$(src) .))
	@$(foreach src,$(shell grep "rdp_drv" $(PROJ_DRV_SRC_LIST)                  | grep -v "^#" | grep -v "Makefile" | grep -v "\.c"),$(shell $(LN) $(RDP_DIR)/$(src) .))
	@$(foreach src,$(shell grep "drivers/bdmf" $(PROJ_DRV_SRC_LIST)             | grep -v "^#" | grep -v "Makefile" | grep -v "\.c"),$(shell $(LN) $(RDP_DIR)/$(src) .))
	@$(foreach src,$(shell grep "drivers/rdpa_gpl/include" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile" | grep -v "\.c"),$(shell $(LN) $(RDP_DIR)/$(src) .))
	$(LN) XRDP_GPL_AG.h XRDP_AG.h
ifneq ("$(RDP_PROJECT)", "XRDP_CFE2")
	$(LN) rdd_data_structures_auto_gpl_$(BRCM_CHIP).h rdd_data_structures_auto.h
endif
	
.PHONY: clean
clean:
	$(RM) *.[oa]
	find . -maxdepth 1 -type l -print | xargs $(RM)
	$(RM) rdd_data_structures_auto_gpl.[c,h]
