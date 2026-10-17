all:
	$(MAKE) $(BLD_TARGET) BLD=$(BLD)

TOP_DIR := $(shell pwd)

BUILD_HOST := $(shell hostname --fqdn)
ifneq ($(wildcard /projects/bca/tools/nice_build/$(BUILD_HOST)),)
NICE_BUILD := nice -n 10
else
NICE_BUILD := 
endif

ifeq ($(strip $(PROFILE)),)
ifeq ($(strip $(PROJECT)),)
ifeq ($(strip $(ALL)),)
ifeq ($(strip $(CFE)),)
$(error You need to specify project/tarball/ut/ut_performance name 'make PROJECT=<project name>' or 'make ALL=<ut|fw|drv|vrf|clean>' or 'make CFE=<fw|clean>')
endif
endif
endif
endif

PROJ_SRC_LIST_FLAGS := $(patsubst %,-D %,$(MAKEOVERRIDES))
export PROJ_SRC_LIST_FLAGS

ifneq ($(strip $(PROJECT)),)
ifeq ($(strip $(PROJECT)), PON_6888_G9991)
$(error PON_6888_G9991 is excluded from build)
endif
include $(TOP_DIR)/make.common
include $(TOP_DIR)/make.dyn_targets
BLD_TARGET = -C $(PROJECT_DIR)
endif

ifneq ($(strip $(ALL)),)
BLD_TARGET = run_ALL
ALL_LIST= \
  PON_6858_FC    \
  PON_6855_FC    \
  PON_6837_FC    \
  PON_68375_FC   \
  PON_6888_B0_FC \
  PON_6856_FC    \
  DSL_63158      \
  BCM63146       \
  BCM4912        \
  BCM6813        \
  BCM6813_FPI
endif

ifneq ($(strip $(CFE)),)
BLD_TARGET = run_CFE
endif

NPROC ?= $(shell nproc)
export NPROC

fw:
	$(NICE_BUILD) $(MAKE) -j$(NPROC) -C $(PROJECT_DIR) BLD=$(BLD) firmware

vrf:
	$(NICE_BUILD) $(MAKE) -C $(PROJECT_DIR) BLD=$(BLD) vrf_drivers sim OS_PLATFORM=linux

drv:
	$(NICE_BUILD) $(MAKE) -j$(NPROC) -C $(PROJECT_DIR) BLD=$(BLD) drivers

ut:
	$(NICE_BUILD) $(MAKE) -j$(NPROC) -C $(PROJECT_DIR) BLD=$(BLD) ut 

ut_performance:
	$(NICE_BUILD) $(MAKE) -j$(NPROC) -C $(PROJECT_DIR) TEST_PERFORMANCE=1 BLD=$(BLD) ut

qut:
	$(NICE_BUILD) $(MAKE) -C $(TOP_DIR)/../tools/qemu/ut/tests DEVEL_DIR=${TOP_DIR}/.. PROFILE=${PROFILE} ut

qut_only:
	$(NICE_BUILD) $(MAKE) -C $(TOP_DIR)/../tools/qemu/ut/tests DEVEL_DIR=${TOP_DIR}/.. PROFILE=${PROFILE} test

hal:
	$(NICE_BUILD) $(MAKE) -C $(PROJECT_DIR) BLD=$(BLD) hal_generator

clean_objs:
	for f in `ls $(PROJECT_DIR)/target*/ | grep -v ut` ; do rm -rf $(PROJECT_DIR)/target*/$$f ; done

clean:
	rm -rf $(PROJECT_DIR)/target*/*
	rm -rf $(PROJECT_DIR)/firmware_bin/runner_fw_*.bak
	rm -rf $(PROJECT_DIR)/firmware_bin/runner_fw_*_*.c
	find $(DRIVERS_PROJ_DIR) -name "*.o" | xargs rm -f
	find $(TOP_DIR) -name "*.pyc" | xargs rm -f
	$(NICE_BUILD) $(MAKE) -C $(RDPA_SRC_DIR) clean

tarball_cable: tarball_prepare_dirs
	@echo "********************************************"
	@echo "Tarball for Cable Modem, includes CM3390"
	@echo "********************************************"
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/CM3390/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))

ifdef ALL
run_ALL:
ifeq ($(ALL),tools_rebuild)
	@echo "********************************************"
	@echo "Smart ALL=tools_rebuild"
	@echo "asm_tools_rebuild once per RGEN"
	@echo "sim_rebuild ones per CHIP_TYPE"
	@echo "********************************************"
	@for i in $(ALL_LIST); do \
		echo $$i `awk -F ':?=[[:space:]]?' ' \
		/^CHIP_TYPE[[:space:]:=]+/ { chip_type=$$2; } \
		/^RGEN[[:space:]:=]+/ { rgen=$$2; } \
		/^GEN[[:space:]:=]+/ { gen=$$2; } \
		END { if (!gen) gen=4; if (!rgen) rgen=gen; print chip_type " " rgen; }' \
		projects/$$i/make.proj_flags` ; \
	done | awk '{ if ($$2 in chip_types); else { \
		chip_types[$$2]=1; \
		if ($$3 in gens) command = "sim_rebuild"; \
		else { gens[$$3]=1; command = "tools_rebuild"; } \
		print " PROJECT=" $$1 " " command; }}' \
	| xargs -n 2 $(MAKE)
else
	@echo "********************************************"
	@echo "Running $(ALL) for all platforms "
	@echo "********************************************"
	@for i in $(ALL_LIST); do \
		echo $(MAKE) PROJECT=$$i clean $(ALL); \
		$(MAKE) PROJECT=$$i clean $(ALL); \
	done
endif
endif

run_CFE:
	@echo "********************************************"
	@echo "Running $(CFE) for RDP platforms "
	@echo "********************************************"
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=6858 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=6856 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=63146 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=4912 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=6855 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=6837 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=68375 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=6813 clean $(CFE)
	$(MAKE) PROJECT=XRDP_CFE2 BRCM_CHIP=68880 clean $(CFE)


opt:
	$(NICE_BUILD) $(MAKE) $(BLD_TARGET) BLD=_opt

ut_opt:
	$(NICE_BUILD) $(MAKE) $(BLD_TARGET) BLD=_opt ut

