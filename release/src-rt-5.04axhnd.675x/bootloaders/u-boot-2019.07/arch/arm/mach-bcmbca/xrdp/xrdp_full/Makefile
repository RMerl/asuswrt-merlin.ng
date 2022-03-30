# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#

CUR_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
BLD_DIR = $(shell pwd)
BLD_DIR :=$(subst /bootloaders/, /bootloaders/,$(BLD_DIR))$
BLD_DIR :=$(word 1,$(BLD_DIR))
TOP_DIR :=$(BLD_DIR)/rdp
PROJECT_DIR := $(TOP_DIR)/projects/XRDP_CFE2
CHIP_ID := $(patsubst "bcm%",%,$(CONFIG_SYS_SOC))

KBUILD_CPPFLAGS += -DBCM$(CHIP_ID) -DXRDP_CFE -D_CFE_REDUCED_XRDP_
KBUILD_CPPFLAGS += -include common.h -I$(BLD_DIR)/shared/opensource/include/bcm963xx
PROJ_DRV_SRC_LIST := $(PROJECT_DIR)/BCM$(CHIP_ID)_A0_UBOOT_sources.list
PROJ_FW_SRC_LIST := $(PROJECT_DIR)/BCM$(CHIP_ID)_A0_fw_sources.list

LN = ln -sf
RM = rm -f

ifneq ($(findstring _$(strip $(CHIP_ID))_,_63146_),)
KBUILD_CPPFLAGS += -DBCM_DSL_XRDP
else
KBUILD_CPPFLAGS += -DBCM_PON_XRDP
endif

ifneq ($(findstring _$(strip $(CHIP_ID))_,_6878_63146_),)
KBUILD_CPPFLAGS += -DDUAL_ISSUE
endif

ifneq ($(findstring _$(strip $(CHIP_ID))_,_63146_),)
KBUILD_CPPFLAGS += -DXRDP_RGEN6

# temporary
KBUILD_CPPFLAGS += -DCONFIG_BRCM_IKOS
endif

obj-y += xrdp_full.o

xrdp_full-objs := XRDP_BAC_IF_AG.o XRDP_BBH_RX_AG.o
xrdp_full-objs += XRDP_BBH_TX_AG.o XRDP_CNPL_AG.o XRDP_DMA_AG.o
xrdp_full-objs += XRDP_DQM_AG.o XRDP_DSPTCHR_AG.o XRDP_FPM_AG.o
xrdp_full-objs += XRDP_HASH_AG.o XRDP_NATC_AG.o XRDP_NATC_CFG_AG.o
xrdp_full-objs += XRDP_NATC_CTRS_AG.o XRDP_NATC_DDR_CFG_AG.o XRDP_NATC_ENG_AG.o
xrdp_full-objs += XRDP_NATC_INDIR_AG.o XRDP_QM_AG.o XRDP_RNR_CNTXT_AG.o
xrdp_full-objs += XRDP_RNR_INST_AG.o XRDP_RNR_MEM_AG.o XRDP_RNR_PRED_AG.o
xrdp_full-objs += XRDP_RNR_REGS_AG.o XRDP_RNR_QUAD_AG.o XRDP_TCAM_AG.o
xrdp_full-objs += XRDP_UBUS_MSTR_AG.o XRDP_UBUS_SLV_AG.o XRDP_UNIMAC_MISC_AG.o
xrdp_full-objs += XRDP_PSRAM_AG.o XRDP_SBPM_AG.o XRDP_UNIMAC_RDP_AG.o

xrdp_full-objs += rdd_init.o rdd_runner_tasks_auto.o rdp_common.o
xrdp_full-objs += rdd_ag_cpu_rx.o
xrdp_full-objs += rdp_drv_bbh_rx.o rdp_drv_bbh_tx.o rdp_drv_dis_reor.o
xrdp_full-objs += rdp_drv_dma.o rdp_drv_rnr.o rdp_drv_sbpm.o

xrdp_full-objs += xrdp_drv_bac_if_ag.o xrdp_drv_bbh_rx_ag.o xrdp_drv_bbh_tx_ag.o
xrdp_full-objs += xrdp_drv_cnpl_ag.o xrdp_drv_dma_ag.o xrdp_drv_dqm_ag.o
xrdp_full-objs += xrdp_drv_dsptchr_ag.o xrdp_drv_unimac_misc_ag.o
xrdp_full-objs += xrdp_drv_fpm_ag.o xrdp_drv_hash_ag.o xrdp_drv_natc_ag.o
xrdp_full-objs += xrdp_drv_natc_ctrs_ag.o xrdp_drv_natc_ddr_cfg_ag.o
xrdp_full-objs += xrdp_drv_natc_eng_ag.o xrdp_drv_natc_indir_ag.o
xrdp_full-objs += xrdp_drv_qm_ag.o xrdp_drv_rnr_cntxt_ag.o
xrdp_full-objs += xrdp_drv_rnr_inst_ag.o xrdp_drv_rnr_mem_ag.o
xrdp_full-objs += xrdp_drv_rnr_pred_ag.o xrdp_drv_rnr_quad_ag.o
xrdp_full-objs += xrdp_drv_rnr_regs_ag.o xrdp_drv_tcam_ag.o
xrdp_full-objs += xrdp_drv_ubus_mstr_ag.o xrdp_drv_ubus_slv_ag.o
xrdp_full-objs += xrdp_drv_psram_ag.o xrdp_drv_sbpm_ag.o
xrdp_full-objs += xrdp_drv_unimac_rdp_ag.o

xrdp_full-objs += fw_binary_auto.o predict_runner_fw_0.o runner_fw_0.o

xrdp_full-objs += data_path_init.o


prepare_xrdp_links:
	@echo "got here!! prepare_xrdp_links"
	$(foreach src,$(shell grep "\.h" $(PROJ_FW_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) $(CUR_DIR)))
	$(foreach src,$(shell grep "drivers/rdd" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) $(CUR_DIR)))
	$(foreach src,$(shell grep "firmware/.*\.h" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) $(CUR_DIR)))
	$(foreach src,$(shell grep "firmware/.*rdd_.*\.c" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) $(CUR_DIR)))
	$(foreach src,$(shell grep "drivers/rdp_subsystem" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) $(CUR_DIR)))
	$(foreach src,$(shell grep "firmware_bin/" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) $(CUR_DIR)))
	$(foreach src,$(shell grep "rdp_drv" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) $(CUR_DIR)))
	$(foreach src,$(shell grep "drivers/bdmf" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile" | grep -v "*.c"),$(shell $(LN) $(TOP_DIR)/$(src) $(CUR_DIR)))
	$(foreach src,$(shell grep "drivers/rdpa_gpl/include" $(PROJ_DRV_SRC_LIST) | grep -v "^#" | grep -v "Makefile"),$(shell $(LN) $(TOP_DIR)/$(src) $(CUR_DIR)))
#	$(foreach src,$(shell ls *.[c,h]),$(shell if [[ ! -f $(src) ]]; then $(RM) $(src); fi))


$(obj)/xrdp_full.o: prepare_xrdp_links


# FIXME!! how to add clean. we can simply delete all the .c/.h
clean-files += $(CUR_DIR)/*.c $(CUR_DIR)/*.h

