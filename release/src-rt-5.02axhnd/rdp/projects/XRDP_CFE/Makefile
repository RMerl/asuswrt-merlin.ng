#***********************************
#
#  Copyright (c) 2014  Broadcom
#  All Rights Reserved
#
#**********************************/

#This is needed because firmware parallel build is broken.
#This causes this instance of Make to execute -j1.
.NOTPARALLEL:

all: firmware drivers

include make.proj_flags
include $(TOP_DIR)/make.common
include $(TOP_DIR)/make.dyn_targets

ut: hw_dts_gen coverage sim firmware drivers
	$(MAKE) -C $(PROJ_UT_TARGET) $@ GEN=$(GEN)
	
cp_output: project_prepare_dirs
	$(MV) $(CHIP_OUTPUT)/hal/*_common_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/
	$(MV) $(CHIP_OUTPUT)/hal/*_bbh_rx_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/bbh_rx/
	$(MV) $(CHIP_OUTPUT)/ru_gen/ru_chip.h $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/
	$(MV) $(CHIP_OUTPUT)/hal/*_psram_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/psram/
	$(MV) $(CHIP_OUTPUT)/hal/*_fpm_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/fpm/
	$(MV) $(CHIP_OUTPUT)/hal/*_cnpl_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/cnpl/
	$(MV) $(CHIP_OUTPUT)/hal/*_bbh_tx_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/bbh_tx/
	$(MV) $(CHIP_OUTPUT)/hal/*_dma_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/dma/
	$(MV) $(CHIP_OUTPUT)/hal/*_sbpm_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/sbpm/
	$(MV) $(CHIP_OUTPUT)/hal/*_qm_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/qm/
	$(MV) $(CHIP_OUTPUT)/hal/*_dqm_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/dqm/
	$(MV) $(CHIP_OUTPUT)/hal/*_tcam_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/tcam/
	$(MV) $(CHIP_OUTPUT)/hal/*_hash_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/hash/
	$(MV) $(CHIP_OUTPUT)/hal/*_rnr_quad_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr_quad/
	$(MV) $(CHIP_OUTPUT)/hal/*_rnr_inst_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr/
	$(MV) $(CHIP_OUTPUT)/hal/*_rnr_mem_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr/
	$(MV) $(CHIP_OUTPUT)/hal/*_rnr_pred_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr/
	$(MV) $(CHIP_OUTPUT)/hal/*_rnr_cntxt_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr/
	$(MV) $(CHIP_OUTPUT)/hal/*_rnr_regs_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr/
	$(MV) $(CHIP_OUTPUT)/hal/*_natc_*ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/natc/
	$(MV) $(CHIP_OUTPUT)/hal/*_dsptchr_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/dis_reor/
	$(MV) $(CHIP_OUTPUT)/hal/*_ubus_mstr_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/ubus_mstr/
	$(MV) $(CHIP_OUTPUT)/hal/*_ubus_slv_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/ubus_slv/
	$(MV) $(CHIP_OUTPUT)/hal/*_xlif_*_ag.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/xlif/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_AG.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_AG.h $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_BBH_RX*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/bbh_rx/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_BBH_TX*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/bbh_tx/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_DMA*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/dma/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_QM*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/qm/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_DQM*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/dqm/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_TCAM*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/tcam/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_PSRAM*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/psram/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_FPM*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/fpm/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_CNPL*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/cnpl/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_SBPM*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/sbpm/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_HASH*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/hash/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_RNR_QUAD*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr_quad/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_RNR*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_NATC*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/natc/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_DSPTCHR*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/dis_reor/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_UBUS_MSTR*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/ubus_mstr/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_UBUS_SLV*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/ubus_slv/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_XLIF*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/xlif/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_DEBUG_BUS*.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/xlif/


