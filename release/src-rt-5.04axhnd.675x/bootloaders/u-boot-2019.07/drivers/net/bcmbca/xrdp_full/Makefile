# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#

ifdef CONFIG_BCMBCA_PMC
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/pmc
endif

obj-y += ../xrdp/bcmbca_xrdp_api.o
EXTRA_CFLAGS += -DNO_ACCESS_LOGGING
# Flags for bcmbca_xrdp_api
ifdef CONFIG_BCMBCA_XRDP_PON
EXTRA_CFLAGS += -DXRDP_SBPM -DGEN45
#obj-y += rdp_drv_sbpm.o
else # CONFIG_BCMBCA_XRDP_DSL
EXTRA_CFLAGS += -DXRDP_DIRECT
endif

BRCM_CHIP = $(patsubst "bcm%",%,$(CONFIG_SYS_SOC))
CUR_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
BLD_DIR = $(shell pwd)
BLD_DIR :=$(subst /bootloaders/, /bootloaders/,$(BLD_DIR))$
BLD_DIR :=$(word 1,$(BLD_DIR))
TOP_DIR :=$(BLD_DIR)/rdp
RDP_PROJECT = XRDP_CFE2
PROJECTS_DIR := $(TOP_DIR)/projects
PROJECT_DIR := $(PROJECTS_DIR)/$(RDP_PROJECT)

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_63146_4912_),)
CHIP_VER = B0
else
CHIP_VER = A0
endif

PROJ_DRV_SRC_LIST := $(PROJECT_DIR)/BCM$(BRCM_CHIP)_$(CHIP_VER)_sources.list
PROJ_FW_SRC_LIST := $(PROJECT_DIR)/BCM$(BRCM_CHIP)_$(CHIP_VER)_fw_sources.list

ifdef CONFIG_BCMBCA_IKOS
KBUILD_CPPFLAGS += -DCONFIG_BRCM_IKOS
endif

LN = ln -sf
MV = mv -f
RM = rm -rf

include $(PROJECT_DIR)/make.proj_flags

EXTRA_CFLAGS += -I$(PROJECT_DIR)/ -I$(PROJECT_DIR)/drivers/rdp_subsystem/
EXTRA_CFLAGS += -I$(PROJECT_DIR)/drivers/rdp_subsystem/BCM$(BRCM_CHIP) -I$(TOP_DIR)/drivers/bdmf/shell
EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/rdpa_gpl/include/ -I$(TOP_DIR)/drivers/bdmf/system/sim
EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/bdmf/system
EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/bdmf/framework
EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/bdmf/interface
EXTRA_CFLAGS += -I$(BLD_DIR)/shared/opensource/include/bcm963xx
EXTRA_CFLAGS += -I$(BLD_DIR)/bcmdrivers/opensource/include/bcm963xx

EXTRA_CFLAGS += -include bdmf_errno.h -include stdio.h 
EXTRA_CFLAGS += -DBDMF_SESSION_H -D_BDMF_INTERFACE_H_ -D_BDMF_SYSTEM_H_ -DBDMF_MON_H -UBDMF_DEBUG -UBDMF_SYSTEM_SIM -UUSE_BDMF_SHELL -DBDMF_NO_TRACE 
EXTRA_CFLAGS += -DRU_USE_STDC -DRU_INCLUDE_DESC=0 -DRU_INCLUDE_FIELD_DB=0 -DRU_FIELD_CHECK_ENABLE=0 -DRU_INCLUDE_ACCESS=0 
EXTRA_CFLAGS += -DXRDP -DXRDP_NO_SWAP -DFIRMWARE_LITTLE_ENDIAN
EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) $(PROJ_DEFS) -UVALIDATE_PARMS -Dbdmf_trace=printf -D__BCM_MAP_PART_H

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6856_),)
EXTRA_CFLAGS += -DPHYS_ADDR_64BIT
endif

ifeq ("$(CONFIG_XRDP_GPL_GEN)", "1")
EXTRA_CFLAGS += -DCONFIG_GPL_RDP_GEN
endif

xrdp_full-objs := $(patsubst %.c, %.o, $(shell cd $(CUR_DIR) && ls *.c))
obj-y += xrdp_full.o

$(obj)/xrdp_full.o: prepare_links
#	$(CC) $(KBUILD_CPPFLAGS) $(EXTRA_CFLAGS) $(shell ls $(CUR_DIR)*.c) $(LDFLAGS) -o $(obj)/xrdp_full.o

.PHONY: clean
clean:
	$(RM) *.o;
	find . -name "*.[c,h]" | grep -v Makefile | grep -v bcmbca_xrdp_api_full.c | xargs $(RM)

.PHONY: prepare_links
prepare_links:
	$(foreach file,$(shell grep "\.h" $(PROJ_FW_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(file) $(CUR_DIR)))
	$(foreach file,$(shell grep "drivers/rdd" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(file) $(CUR_DIR)))
	$(foreach file,$(shell grep "firmware/.*\.h" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(file) $(CUR_DIR)))
	$(foreach file,$(shell grep "firmware/.*rdd_.*\.c" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(file) $(CUR_DIR)))
	$(foreach file,$(shell grep "drivers/rdp_subsystem" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(file) $(CUR_DIR)))
	$(foreach file,$(shell grep "firmware_bin/" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(file) $(CUR_DIR)))
	$(foreach file,$(shell grep "rdp_drv" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(file) $(CUR_DIR)))
	$(foreach file,$(shell grep "drivers/bdmf" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile" | grep -v "*.c"),$(shell $(LN) $(TOP_DIR)/$(file) $(CUR_DIR)))
	$(foreach file,$(shell grep "drivers/rdpa_gpl/include" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(file) $(CUR_DIR)))

