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

PROJ_DRV_SRC_LIST := $(PROJECT_DIR)/sources.list
PROJ_FW_SRC_LIST := $(PROJECT_DIR)/fw_sources.list
LN = ln -sf

ifneq ($(strip $(RDP_PROJECT)),)
include $(TOP_DIR)/projects/$(RDP_PROJECT)/make.proj_flags
endif

cfe: cfe_lib

LIB = libxrdp.a 

RDPOBJS = $(SRCS:.c=.o)
SRCS = $(notdir $(shell ls *.c))
 
cfe_lib: build_obj
	$(AR) cr $(LIB) $(RDPOBJS)
	find . -maxdepth 1 -type l -exec rm -f {} \;
EXTRA_INCLUDE += -I$(BLD_DIR)/cfe/cfe/include 
EXTRA_INCLUDE += -I$(PROJECT_DIR)/ -I$(TOP_DIR)/drivers/rdp_subsystem/
EXTRA_INCLUDE += -I$(TOP_DIR)/drivers/rdp_subsystem/$(RDP_SUBSYS_PROJECT) -I$(TOP_DIR)/drivers/bdmf/shell
EXTRA_INCLUDE += -I$(TOP_DIR)/drivers/rdpa_gpl/include/ -I$(TOP_DIR)/drivers/bdmf/system/sim
EXTRA_INCLUDE += -I$(TOP_DIR)/drivers/bdmf/system
EXTRA_INCLUDE += -I$(TOP_DIR)/drivers/bdmf/framework
EXTRA_INCLUDE += -I$(TOP_DIR)/drivers/bdmf/interface
EXTRA_INCLUDE += -I$(BLD_DIR)/shared/opensource/include/bcm963xx
EXTRA_INCLUDE += -I$(BLD_DIR)/shared/opensource/include/rdp
EXTRA_INCLUDE += -I$(BLD_DIR)/cfe/cfe/arch/arm/cpu/armv8/include/
CFLAGS += -D_LIB_NO_MACROS_ -D__ARMEL__ -DBDMF_SESSION_H -D_BDMF_INTERFACE_H_ -D_BDMF_SYSTEM_H_ -DBDMF_MON_H -DCFG_RAMAPP=1 
CFLAGS += -include xrdp_cfe.h -include stdint.h -include stddef.h -include bdmf_errno.h -std=gnu99 -Wall -Werror $(PROJ_CFLAGS) -UUSE_BDMF_SHELL -DRU_USE_STDC -UVALIDATE_PARMS -UBDMF_DEBUG -UBDMF_SYSTEM_SIM -DBDMF_NO_TRACE -DRU_INCLUDE_DESC=0 -DRU_INCLUDE_FIELD_DB=0 -DRU_FIELD_CHECK_ENABLE=0 -DRU_INCLUDE_ACCESS=0 
CFLAGS += -DXRDP -DXRDP_NO_SWAP
CFLAGS += -D__BIT_TYPES_DEFINED__ -DFIRMWARE_LITTLE_ENDIAN

build_obj: prepare_links
	$(GCC) $(CFLAGS) -ffunction-sections $(BASEFLAGS) $(EXTRA_INCLUDE) $(SRCS) $(LFLAGS)

clean:
	rm -rf *.o;
	rm -f $(LIB);
	find . -maxdepth 1 -type l -exec rm -f {} \;

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
	$(RM) *shell*
	$(RM) *rdd_simulator*
	$(RM) *rdd_ip_class*
	$(RM) rdd_tcam_ic.c
	$(RM) rdd_runner_reg_dump.c
	$(RM) rdd_runner_reg_dump_addrs.c
	$(RM) rdp_drv_hash.c
	$(RM) rdp_drv_natc.c
	$(RM) rdd_ic_common.c
	$(RM) rdd_iptv.*

