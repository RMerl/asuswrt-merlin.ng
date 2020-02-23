#***********************************************************************
#
#  Copyright (c) 2014  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

all: 
	$(MAKE) $(BLD_TARGET) BLD=$(BLD)

TOP_DIR := $(shell pwd)


ifeq ($(strip $(PROJECT)),)
ifeq ($(strip $(TAR)),)
ifeq ($(strip $(ALL)),)
ifeq ($(strip $(XRDP)),)
ifeq ($(strip $(RDP)),)
$(error You need to specify project/tarball/ut/ut_performance name 'make PROJECT=<project name>' or 'make TAR=<cms|cable> <build|rem>' or 'make ALL=<ut|fw|drv|vrf|clean>' or 'make XRDP=<ut|fw|drv|vrf|clean>')
endif
endif
endif
endif
endif

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

fw:
	$(MAKE) -C $(PROJECT_DIR) BLD=$(BLD) firmware

vrf:
	$(MAKE) -C $(PROJECT_DIR) BLD=$(BLD) vrf_drivers sim OS_PLATFORM=linux

drv:
	$(MAKE) -j -C $(PROJECT_DIR) BLD=$(BLD) drivers

ut:
	$(MAKE) -j -C $(PROJECT_DIR) BLD=$(BLD) ut $@
    
ut_performance:
	$(MAKE) -j -C $(PROJECT_DIR) TEST_PERFORMANCE=1 BLD=$(BLD) ut    

hal:
	$(MAKE) -C $(PROJECT_DIR) BLD=$(BLD) hal_generator

clean_objs:
	for f in `ls $(PROJECT_DIR)/target*/ | grep -v ut` ; do rm -rf $(PROJECT_DIR)/target*/$$f ; done 

clean:
	rm -rf $(PROJECT_DIR)/target*/* 
	rm -rf $(PROJECT_DIR)/firmware_bin/runner_fw_*.bak
	find $(TOP_DIR) -name "*.pyc" | xargs rm -f
	rm -rf $(RDPA_SRC_DIR)/rdp_version.h.bak
	$(MAKE) -C $(RDPA_SRC_DIR) clean

tarball_cms: tarball_prepare_dirs
	@echo "********************************************"
	@echo "Tarball for CMS, includes OREN, G9991, DSL  "
	@echo "********************************************"
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/PON_6838/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/PON_6838_G9991/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/DSL_63138/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/DSL_63148/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/WL4908/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/WL4908_EAP/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))
	cp drivers/bdmf/sources.list.CMS cp_bdmf.sh
	grep $(TOOLS_BIN)/cms_convert_input.txt -e "s/" | xargs -0 -I {} $(TOOLS_BIN)/bsed.sh -r "{}" cp_bdmf.sh
	mv cp_bdmf.sh $(TARBALL_DIR)/
	cp $(PROJECTS_DIR)/PON_6838_G9991/sources.list cp_src_PON_6838_G9991.sh
	grep $(TOOLS_BIN)/cms_convert_input.txt -e "s/" | xargs -0 -I {} $(TOOLS_BIN)/bsed.sh -r "{}" cp_src_PON_6838_G9991.sh
	mv cp_src_PON_6838_G9991.sh $(TARBALL_DIR)/
	cp $(PROJECTS_DIR)/PON_6838/sources.list cp_src_PON_6838.sh
	grep $(TOOLS_BIN)/cms_convert_input.txt -e "s/" | xargs -0 -I {} $(TOOLS_BIN)/bsed.sh -r "{}" cp_src_PON_6838.sh
	mv cp_src_PON_6838.sh $(TARBALL_DIR)/
	cp $(PROJECTS_DIR)/DSL_63138/sources.list cp_src_DSL_63138.sh
	grep $(TOOLS_BIN)/cms_convert_input.txt -e "s/" | xargs -0 -I {} $(TOOLS_BIN)/bsed.sh -r "{}" cp_src_DSL_63138.sh
	mv cp_src_DSL_63138.sh $(TARBALL_DIR)/
	cp $(PROJECTS_DIR)/DSL_63148/sources.list cp_src_DSL_63148.sh
	grep $(TOOLS_BIN)/cms_convert_input.txt -e "s/" | xargs -0 -I {} $(TOOLS_BIN)/bsed.sh -r "{}" cp_src_DSL_63148.sh
	mv cp_src_DSL_63148.sh $(TARBALL_DIR)/
	cp $(PROJECTS_DIR)/WL4908/sources.list cp_src_WL4908.sh
	grep $(TOOLS_BIN)/cms_convert_input.txt -e "s/" | xargs -0 -I {} $(TOOLS_BIN)/bsed.sh -r "{}" cp_src_WL4908.sh
	mv cp_src_WL4908.sh $(TARBALL_DIR)/
	cp $(PROJECTS_DIR)/WL4908_EAP/sources.list cp_src_WL4908_EAP.sh
	grep $(TOOLS_BIN)/cms_convert_input.txt -e "s/" | xargs -0 -I {} $(TOOLS_BIN)/bsed.sh -r "{}" cp_src_WL4908_EAP.sh
	mv cp_src_WL4908_EAP.sh $(TARBALL_DIR)/
	chmod a+x $(TARBALL_DIR)/*.sh

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
	$(MAKE) PROJECT=PON_6838 clean $(ALL)
	$(MAKE) PROJECT=PON_6838_G9991 clean $(ALL)
	$(MAKE) PROJECT=PON_6858 clean $(ALL)
	$(MAKE) PROJECT=PON_6858_G9991 clean $(ALL)
	$(MAKE) PROJECT=PON_6846 clean $(ALL)
	$(MAKE) PROJECT=PON_6878 clean $(ALL)
	$(MAKE) PROJECT=PON_6856 clean $(ALL)
	$(MAKE) PROJECT=DSL_63158 clean $(ALL)
	$(MAKE) PROJECT=DSL_63138 clean $(ALL)
	$(MAKE) PROJECT=DSL_63148 clean $(ALL)
	$(MAKE) PROJECT=WL4908 clean $(ALL)
	$(MAKE) PROJECT=WL4908_EAP clean $(ALL)
	
run_XRDP:
	@echo "********************************************"
	@echo "Running $(XRDP) for XRDP platforms "
	@echo "********************************************"
	$(MAKE) PROJECT=PON_6858 clean $(XRDP)
	$(MAKE) PROJECT=PON_6858_G9991 clean $(XRDP)
	$(MAKE) PROJECT=PON_6846 clean $(XRDP)
	$(MAKE) PROJECT=PON_6878 clean $(XRDP)
	$(MAKE) PROJECT=PON_6856 clean $(XRDP)
	$(MAKE) PROJECT=DSL_63158 clean $(XRDP)
	
run_RDP:
	@echo "********************************************"
	@echo "Running $(RDP) for RDP platforms "
	@echo "********************************************"
	$(MAKE) PROJECT=PON_6838 clean $(RDP)
	$(MAKE) PROJECT=PON_6838_G9991 clean $(RDP)
	$(MAKE) PROJECT=DSL_63138 clean $(RDP)
	$(MAKE) PROJECT=DSL_63148 clean $(RDP)
	$(MAKE) PROJECT=WL4908 clean $(RDP)
	$(MAKE) PROJECT=WL4908_EAP clean $(RDP)
	
opt:
	$(MAKE) $(BLD_TARGET) BLD=_opt

ut_opt:
	$(MAKE) $(BLD_TARGET) BLD=_opt ut

