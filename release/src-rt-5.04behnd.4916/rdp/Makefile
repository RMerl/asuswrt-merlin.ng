all:
	$(MAKE) $(BLD_TARGET) BLD=$(BLD)

TOP_DIR := $(shell pwd)


ifeq ($(strip $(PROJECT)),)
ifeq ($(strip $(TAR)),)
ifeq ($(strip $(ALL)),)
ifeq ($(strip $(XRDP)),)
ifeq ($(strip $(RDP)),)
ifeq ($(strip $(CFE)),)
$(error You need to specify project/tarball/ut/ut_performance name 'make PROJECT=<project name>' or 'make TAR=<cms|cable> <build|rem>' or 'make ALL=<ut|fw|drv|vrf|clean>' or 'make XRDP=<ut|fw|drv|vrf|clean> or 'make CFE=<fw|clean>')
endif
endif
endif
endif
endif
endif

PROJ_SRC_LIST_FLAGS := $(patsubst %,-D %,$(MAKEOVERRIDES))
export PROJ_SRC_LIST_FLAGS

ifneq ($(strip $(PROJECT)),)
include $(TOP_DIR)/make.common
include $(TOP_DIR)/make.dyn_targets
BLD_TARGET = -C $(PROJECT_DIR)
endif

ifneq ($(strip $(ALL)),)
BLD_TARGET = run_ALL
endif

ifneq ($(strip $(XRDP)),)
BLD_TARGET = run_XRDP
endif

ifneq ($(strip $(RDP)),)
BLD_TARGET = run_RDP
endif

ifneq ($(strip $(CFE)),)
BLD_TARGET = run_CFE
endif

NPROC := $(shell nproc)

fw:
	$(MAKE) -j$(NPROC) -C $(PROJECT_DIR) BLD=$(BLD) firmware

vrf:
	$(MAKE) -C $(PROJECT_DIR) BLD=$(BLD) vrf_drivers sim OS_PLATFORM=linux

drv:
	$(MAKE) -j$(NPROC) -C $(PROJECT_DIR) BLD=$(BLD) drivers

ut:
	$(MAKE) -j$(NPROC) -C $(PROJECT_DIR) BLD=$(BLD) ut

ut_performance:
	$(MAKE) -j$(NPROC) -C $(PROJECT_DIR) TEST_PERFORMANCE=1 BLD=$(BLD) ut

hal:
	$(MAKE) -C $(PROJECT_DIR) BLD=$(BLD) hal_generator

clean_objs:
	for f in `ls $(PROJECT_DIR)/target*/ | grep -v ut` ; do rm -rf $(PROJECT_DIR)/target*/$$f ; done

clean:
	rm -rf $(PROJECT_DIR)/target*/*
	rm -rf $(PROJECT_DIR)/firmware_bin/runner_fw_*.bak
	find $(TOP_DIR) -name "*.pyc" | xargs rm -f
	$(MAKE) -C $(RDPA_SRC_DIR) clean

tarball_cable: tarball_prepare_dirs
	@echo "********************************************"
	@echo "Tarball for Cable Modem, includes CM3390"
	@echo "********************************************"
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/CM3390/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))


build: tarball_$(TAR)
	$(CP) --parent -r drivers/bdmf $(TARBALL_DIR)
	@echo "Removing trailing white spaces..."
	find $(TARBALL_DIR) -type f |xargs $(SED) -i 's/[[:space:]]*$$//'
	@echo "Removing spaces before tabs..."
	find $(TARBALL_DIR) -type f |xargs $(SED) -i 's/ *\t */\t/g'
	@echo "Removing empty lines before EOF..."
	find $(TARBALL_DIR) -type f | xargs $(PERL) -pi -e 'chomp if eof'
	find $(TARBALL_DIR) -type f -name '*.bak' -exec rm -f {} \;
	rm $(TARBALL_DIR)/drivers/bdmf/sources.list.CMS
	@echo "Compressing Tarball"
	cd $(TARBALL_DIR) && chmod -R a+r . && tar cfz ../rdp_$(TAR)-$(shell date  --rfc-3339='date').tar.gz *

rem:
	$(RM) -rf $(TARBALL_DIR)
	$(RM) $(DELIVERIES_DIR)/rdp_$(TAR)-*.tar.gz

run_ALL:
	@echo "********************************************"
	@echo "Running $(ALL) for all platforms "
	@echo "********************************************"
	$(MAKE) PROJECT=PON_6858_FC clean $(ALL)
	$(MAKE) PROJECT=PON_6858_PRV clean $(ALL)
	$(MAKE) PROJECT=PON_6858_G9991 clean $(ALL)
	$(MAKE) PROJECT=PON_6846_FC clean $(ALL)
	$(MAKE) PROJECT=PON_6846_PRV clean $(ALL)
	$(MAKE) PROJECT=PON_6878_FC clean $(ALL)
	$(MAKE) PROJECT=PON_6855_FC clean $(ALL)
	$(MAKE) PROJECT=PON_6837_FC clean $(ALL)
	$(MAKE) PROJECT=PON_6888_B0_FC clean $(ALL)
	$(MAKE) PROJECT=PON_6888_G9991 clean $(ALL)
	$(MAKE) PROJECT=PON_6856_FC clean $(ALL)
	$(MAKE) PROJECT=PON_6856_PRV clean $(ALL)
	$(MAKE) PROJECT=DSL_63158 clean $(ALL)
	$(MAKE) PROJECT=DSL_63138 clean $(ALL)
	$(MAKE) PROJECT=DSL_63148 clean $(ALL)
	$(MAKE) PROJECT=BCM63146 clean $(ALL)
	$(MAKE) PROJECT=WL4908 clean $(ALL)
	$(MAKE) PROJECT=WL4908_EAP clean $(ALL)
	$(MAKE) PROJECT=BCM4912 clean $(ALL)
	$(MAKE) PROJECT=BCM4912_FPI clean $(ALL)
	$(MAKE) PROJECT=BCM6813 clean $(ALL)
	$(MAKE) PROJECT=BCM6813_FPI clean $(ALL)

run_XRDP:
	@echo "********************************************"
	@echo "Running $(XRDP) for XRDP platforms "
	@echo "********************************************"
	$(MAKE) PROJECT=PON_6858_FC clean $(XRDP)
	$(MAKE) PROJECT=PON_6858_PRV clean $(XRDP)
	$(MAKE) PROJECT=PON_6858_G9991 clean $(XRDP)
	$(MAKE) PROJECT=PON_6846_FC clean $(XRDP)
	$(MAKE) PROJECT=PON_6846_PRV clean $(XRDP)
	$(MAKE) PROJECT=PON_6878_FC clean $(XRDP)
	$(MAKE) PROJECT=PON_6855_FC clean $(XRDP)
	$(MAKE) PROJECT=PON_6837_FC clean $(XRDP)
	$(MAKE) PROJECT=PON_6888_B0_FC clean $(XRDP)
	$(MAKE) PROJECT=PON_6888_G9991 clean $(XRDP)
	$(MAKE) PROJECT=PON_6856_FC clean $(XRDP)
	$(MAKE) PROJECT=PON_6856_PRV clean $(XRDP)
	$(MAKE) PROJECT=DSL_63158 clean $(XRDP)
	$(MAKE) PROJECT=BCM63146 clean $(XRDP)
	$(MAKE) PROJECT=BCM4912 clean $(XRDP)
	$(MAKE) PROJECT=BCM4912_FPI clean $(XRDP)
	$(MAKE) PROJECT=BCM6813 clean $(XRDP)
	$(MAKE) PROJECT=BCM6813_FPI clean $(XRDP)

run_RDP:
	@echo "********************************************"
	@echo "Running $(RDP) for RDP platforms "
	@echo "********************************************"
	$(MAKE) PROJECT=DSL_63138 clean $(RDP)
	$(MAKE) PROJECT=DSL_63148 clean $(RDP)
	$(MAKE) PROJECT=WL4908 clean $(RDP)
	$(MAKE) PROJECT=WL4908_EAP clean $(RDP)

run_CFE:
	@echo "********************************************"
	@echo "Running $(CFE) for RDP platforms "
	@echo "********************************************"
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=6878 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=6846 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=6858 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=6856 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=63146 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=4912 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=6855 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=6837 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=6813 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=68880 clean $(CFE)


opt:
	$(MAKE) $(BLD_TARGET) BLD=_opt

ut_opt:
	$(MAKE) $(BLD_TARGET) BLD=_opt ut

