all: firmware drivers

# Make sure that firmware is built before drivers when building ut
# This causes this instance of Make to execute -j1.
.NOTPARALLEL:

CMAKE_FW_BUILD ?= y

include make.proj_flags
include $(TOP_DIR)/make.common
include $(TOP_DIR)/make.dyn_targets

ut: hw_dts_gen sim firmware drivers
	$(MAKE) -C $(PROJ_UT_TARGET) $@ GEN=$(GEN) RGEN=$(RGEN)

cp_output: project_prepare_dirs
	#$(MV) $(CHIP_OUTPUT)/hal/*_xxxcommon_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/
	$(MV) $(CHIP_OUTPUT)/hal/*_common_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/
	$(MV) $(CHIP_OUTPUT)/hal/*_bbh_rx_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/bbh_rx/
	$(MV) $(CHIP_OUTPUT)/ru_gen/ru_chip.h $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/
	$(MV) $(CHIP_OUTPUT)/hal/*_psram_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/psram/
	$(MV) $(CHIP_OUTPUT)/hal/*_psram_mem_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/psram/
	$(MV) $(CHIP_OUTPUT)/hal/*_xlif_xrdp*_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/xlif/
	$(MV) $(CHIP_OUTPUT)/hal/*_fpm_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/fpm/
	$(MV) $(CHIP_OUTPUT)/hal/*_fpm_fpmini_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/fpm/
	$(MV) $(CHIP_OUTPUT)/hal/*_cnpl_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/cnpl/
	$(MV) $(CHIP_OUTPUT)/hal/*_bbh_tx_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/bbh_tx/
	$(MV) $(CHIP_OUTPUT)/hal/*_dma_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/dma/
	$(MV) $(CHIP_OUTPUT)/hal/*_sbpm_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/sbpm/
	$(MV) $(CHIP_OUTPUT)/hal/*_qm_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/qm/
	$(MV) $(CHIP_OUTPUT)/hal/*_dqm_*ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/dqm/
	$(MV) $(CHIP_OUTPUT)/hal/*_tcam_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/tcam/
	$(MV) $(CHIP_OUTPUT)/hal/*_hash_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/hash/
	$(MV) $(CHIP_OUTPUT)/hal/*_rnr_quad_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr_quad/
	$(MV) $(CHIP_OUTPUT)/hal/*_rnr_inst_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr/
	$(MV) $(CHIP_OUTPUT)/hal/*_rnr_mem_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr/
	$(MV) $(CHIP_OUTPUT)/hal/*_rnr_pred_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr/
	$(MV) $(CHIP_OUTPUT)/hal/*_rnr_cntxt_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr/
	$(MV) $(CHIP_OUTPUT)/hal/*_rnr_regs_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr/
	$(MV) $(CHIP_OUTPUT)/hal/*_natc_*ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/natc/
	$(MV) $(CHIP_OUTPUT)/hal/*_dsptchr_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/dis_reor/
	$(MV) $(CHIP_OUTPUT)/hal/*_ubus_requ_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/ubus_requ/
	$(MV) $(CHIP_OUTPUT)/hal/*_ubus_resp_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/ubus_resp/
	$(MV) $(CHIP_OUTPUT)/hal/*_bac_*_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/bac_if/
	$(MV) $(CHIP_OUTPUT)/hal/*_bufmng_ag.[ch] $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/bufmng/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_AG.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_AG.h $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/
	#$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_GPL_AG.c $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/
	#$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_GPL_AG.h $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_BBH_RX*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/bbh_rx/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_BBH_TX*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/bbh_tx/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_DMA*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/dma/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_QM*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/qm/
	$(MV) $(CHIP_OUTPUT)/ru_gen/*DQM*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/dqm/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_TCAM*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/tcam/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_PS*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/psram/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_FPM*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/fpm/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_CNPL*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/cnpl/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_SBPM*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/sbpm/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_HASH*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/hash/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_RNR_QUAD*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr_quad/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_RNR*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/rnr/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_NATC*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/natc/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_DSPTCHR*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/dis_reor/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_UBUS_REQU*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/ubus_requ/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_UBUS_RESP*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/ubus_resp/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_BAC_IF*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/bac_if/
	#$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_UNIMAC*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/unimac/
	#$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_XUMAC*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/unimac/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_XLIF_XRDP*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/xlif/
	$(MV) $(CHIP_OUTPUT)/ru_gen/XRDP_BUFMNG*.* $(RDP_SUBSYSTEM_DIR_PROJECT_AUTOGEN)/bufmng/
