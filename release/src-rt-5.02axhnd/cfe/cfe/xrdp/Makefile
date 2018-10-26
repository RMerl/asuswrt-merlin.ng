
#***********************************************************************
#
#  Copyright (c) 2014  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/
BLD_DIR = $(shell pwd)
BLD_DIR :=$(subst /cfe/, /cfe/,$(BLD_DIR))$
BLD_DIR :=$(word 1,$(BLD_DIR))
TOP_DIR :=$(BLD_DIR)/rdp
PROJECTS_DIR := $(TOP_DIR)/projects
PROJECT_DIR := $(PROJECTS_DIR)/$(RDP_PROJECT)

PROJ_FW_SRC_LIST := $(PROJECT_DIR)/fw_sources.list

ifeq ($(strip $(BRCM_CHIP)),6846)
    ifeq ($(strip $(CONFIG_GPL_RDP)),1)
        PROJ_DRV_SRC_LIST := $(PROJECT_DIR)/BCM6846_A0_gpl_sources.list
    else
        PROJ_DRV_SRC_LIST := $(PROJECT_DIR)/BCM6846_A0_sources.list
    endif
    PROJ_FW_SRC_LIST := $(PROJECT_DIR)/BCM6846_A0_fw_sources.list
else ifeq ($(strip $(BRCM_CHIP)),6856)
    ifeq ($(strip $(CONFIG_GPL_RDP)),1)
        PROJ_DRV_SRC_LIST := $(PROJECT_DIR)/BCM6856_A0_gpl_sources.list
    else
        PROJ_DRV_SRC_LIST := $(PROJECT_DIR)/BCM6856_A0_sources.list
    endif
    PROJ_FW_SRC_LIST := $(PROJECT_DIR)/BCM6856_A0_fw_sources.list
else ifeq ($(strip $(BRCM_CHIP)),6858)
    ifeq ($(strip $(CONFIG_GPL_RDP)),1)
        PROJ_DRV_SRC_LIST := $(PROJECT_DIR)/BCM6858_B0_gpl_sources.list
    else
        PROJ_DRV_SRC_LIST := $(PROJECT_DIR)/BCM6858_B0_sources.list
    endif
    PROJ_FW_SRC_LIST := $(PROJECT_DIR)/BCM6858_B0_fw_sources.list
endif

LN = ln -sf
MV = mv -f

ifneq ($(strip $(RDP_PROJECT)),)
include $(TOP_DIR)/projects/$(RDP_PROJECT)/make.proj_flags
endif

cfe: cfe_lib

LIB = libxrdp.a 

RDPOBJS = $(SRCS:.c=.o)
SRCS = $(notdir $(shell ls *.c))
 
cfe_lib: build_obj
	$(GAR) cr $(LIB) $(RDPOBJS)
#	find . -maxdepth 1 -type l -exec rm -f {} \;
EXTRA_INCLUDE += -I$(BLD_DIR)/cfe/cfe/include 
ifneq ($(strip $(CONFIG_GPL_RDP)),1)
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_),)
EXTRA_INCLUDE += -I$(PROJECT_DIR)/ -I$(TOP_DIR)/drivers/rdp_subsystem/
EXTRA_INCLUDE += -I$(TOP_DIR)/drivers/rdp_subsystem/$(RDP_SUBSYS_PROJECT) -I$(TOP_DIR)/drivers/bdmf/shell
else
EXTRA_INCLUDE += -I$(PROJECT_DIR)/ -I$(PROJECT_DIR)/drivers/rdp_subsystem/
EXTRA_INCLUDE += -I$(PROJECT_DIR)/drivers/rdp_subsystem/$(RDP_SUBSYS_PROJECT) -I$(TOP_DIR)/drivers/bdmf/shell
endif
EXTRA_INCLUDE += -I$(TOP_DIR)/drivers/rdpa_gpl/include/ -I$(TOP_DIR)/drivers/bdmf/system/sim
EXTRA_INCLUDE += -I$(TOP_DIR)/drivers/bdmf/system
EXTRA_INCLUDE += -I$(TOP_DIR)/drivers/bdmf/framework
EXTRA_INCLUDE += -I$(TOP_DIR)/drivers/bdmf/interface
endif
EXTRA_INCLUDE += -I$(BLD_DIR)/shared/opensource/include/bcm963xx
EXTRA_INCLUDE += -I$(BLD_DIR)/bcmdrivers/opensource/include/bcm963xx
EXTRA_INCLUDE += -I$(BLD_DIR)/shared/opensource/include/rdp
EXTRA_INCLUDE += -I$(BLD_DIR)/cfe/cfe/arch/arm/common/include/

ifeq ($(strip $(BRCM_CHIP)),6846)
EXTRA_INCLUDE += -I$(BLD_DIR)/cfe/cfe/arch/arm/cpu/armv7/include/
else
EXTRA_INCLUDE += -I$(BLD_DIR)/cfe/cfe/arch/arm/cpu/armv8/include/
endif

CFLAGS += -D_LIB_NO_MACROS_ -D__ARMEL__ -DBDMF_SESSION_H -D_BDMF_INTERFACE_H_ -D_BDMF_SYSTEM_H_ -DBDMF_MON_H -DCFG_RAMAPP=1 
CFLAGS += -include xrdp_cfe.h -include stdint.h -include stddef.h -include bdmf_errno.h -std=gnu99 -Wall -Werror $(PROJ_CFLAGS) -UUSE_BDMF_SHELL -DRU_USE_STDC -UVALIDATE_PARMS -UBDMF_DEBUG -UBDMF_SYSTEM_SIM -DBDMF_NO_TRACE -DRU_INCLUDE_DESC=0 -DRU_INCLUDE_FIELD_DB=0 -DRU_FIELD_CHECK_ENABLE=0 -DRU_INCLUDE_ACCESS=0 
CFLAGS += -DXRDP -DXRDP_NO_SWAP
CFLAGS += -D__BIT_TYPES_DEFINED__ -DFIRMWARE_LITTLE_ENDIAN

ifeq ("$(CONFIG_GPL_RDP_GEN)", "1")
CFLAGS += -DCONFIG_GPL_RDP_GEN
endif

ifeq ("$(CONFIG_GPL_RDP)", "1")
CFLAGS += -DCONFIG_GPL_RDP
endif

RMV_GPL_RDP_LIST := nat.h gre.h no_fwd.h pbits_remarking.h
RMV_GPL_RDP_LIST += pbit_to_gem.h pbit_to_queue.h policer.h pppoe.h ingress_filter.h
RMV_GPL_RDP_LIST += bridge.h llc_snap_len.h
RMV_GPL_RDP_LIST += dhd*.h 
RMV_GPL_RDP_LIST += drop.h dscp.h ds_lite.h egress_vlan_isolation.h 
RMV_GPL_RDP_LIST += dei.h exception.h vlan_aggregation.h ttl.h tc_to_queue.h
RMV_GPL_RDP_LIST += fw_*.h
RMV_GPL_RDP_LIST += runner_fw_tools_defs.h runner_proprietary.h
RMV_GPL_RDP_LIST += rdd_runner_defs_auto.h

build_obj: prepare_links
	$(GCC) $(CFLAGS) -ffunction-sections $(BASEFLAGS) $(EXTRA_INCLUDE) $(SRCS) $(LFLAGS)

clean:
	rm -rf *.o;
	rm -f $(LIB);
	find . -maxdepth 1 -type l -exec rm -f {} \;
	rm -f rdd_data_structures_auto_gpl.[c,h]

distclean: clean
	find . |grep -v Makefile | grep -v cfe_main.c | xargs rm

prepare_links:
	$(foreach src,$(shell grep "\.h" $(PROJ_FW_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) .))
	$(foreach src,$(shell grep "drivers/rdd" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) .))
	$(foreach src,$(shell grep "firmware/.*\.h" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) .))
	$(foreach src,$(shell grep "firmware/.*rdd_.*\.c" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) .))
	$(foreach src,$(shell grep "drivers/rdp_subsystem" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) .))
	$(foreach src,$(shell grep "firmware_bin/" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) .))
	$(foreach src,$(shell grep "rdp_drv" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) .))
	$(foreach src,$(shell grep "drivers/bdmf" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile" | grep -v "*.c"),$(shell $(LN) $(TOP_DIR)/$(src) .))
	$(foreach src,$(shell grep "drivers/rdpa_gpl/include" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) .))
	$(foreach src,$(shell ls *.[c,h]),$(shell if [[ ! -f $(src) ]]; then $(RM) $(src); fi))
	$(if $(filter "$(CONFIG_GPL_RDP)", "1"), $(shell $(RM) data_path_init.c), $(shell $(RM) data_path_init_basic.c))
	$(if $(filter "$(CONFIG_GPL_RDP)", "1"), $(shell $(RM) $(RMV_GPL_RDP_LIST)))
	$(if $(filter "$(CONFIG_GPL_RDP)", "1"), $(shell $(LN) XRDP_GPL_AG.h XRDP_AG.h))
	$(if $(filter "$(CONFIG_GPL_RDP)", "1"), $(shell $(LN) rdd_data_structures_auto_gpl_$(BRCM_CHIP).h rdd_data_structures_auto.h))
	$(if $(filter "$(CONFIG_GPL_RDP_GEN)", "1"), $(shell sed -n -f data_struct_auto_h_to_gpl.sed rdd_data_structures_auto.h > rdd_data_structures_auto_gpl_$(BRCM_CHIP).h; $(MV) rdd_data_structures_auto_gpl_$(BRCM_CHIP).h $(PROJECT_DIR)/drivers/rdd))
	$(if $(filter "$(CONFIG_GPL_RDP_GEN)", "1"), $(shell sed -n -f data_struct_auto_c_to_gpl.sed rdd_data_structures_auto.c > rdd_data_structures_auto_gpl_$(BRCM_CHIP).c; $(MV) rdd_data_structures_auto_gpl_$(BRCM_CHIP).c $(PROJECT_DIR)/drivers/rdd))
	$(RM) *shell*
	$(RM) *rdd_simulator*
	$(RM) *rdd_ip_class*
	$(RM) *rdd_dhd*
	$(RM) *rdd_wlan*
	$(RM) *rdd_bridge*
	$(RM) rdd_tcam_ic.c
	$(RM) rdd_runner_reg_dump.c
	$(RM) rdd_runner_reg_dump_addrs.c
	$(RM) rdp_drv_hash.c
	$(RM) rdp_drv_natc.c
	$(RM) rdp_drv_policer.c
	$(RM) rdd_ic_common.c
	$(RM) rdd_iptv.*


