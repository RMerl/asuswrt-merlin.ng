#***********************************************************************
#
#  Copyright (c) 2014  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

all: 
	$(MAKE) $(TARGET)

TOP_DIR := $(shell pwd)


ifeq ($(strip $(PROJECT)),)
ifeq ($(strip $(TAR)),)
ifeq ($(strip $(ALL)),)
ifeq ($(strip $(XRDP)),)
$(error You need to specify project/tarball/ut name 'make PROJECT=<project name>' or 'make TAR=<cms|cable> <build|rem>' or 'make ALL=<ut|fw|drv|vrf|clean>' or 'make XRDP=<ut|fw|drv|vrf|clean>')
endif
endif
endif
endif

ifneq ($(strip $(PROJECT)),)
include $(TOP_DIR)/make.common
include $(TOP_DIR)/make.dyn_targets
TARGET = -C $(PROJECT_DIR)
endif

ifneq ($(strip $(ALL)),)
TARGET = run_$(ALL)
endif

ifneq ($(strip $(XRDP)),)
TARGET = run_$(XRDP)
endif

fw:
	$(MAKE) -C $(PROJECT_DIR) firmware

vrf:
	$(MAKE) -C $(PROJECT_DIR) vrf_drivers sim OS_PLATFORM=linux

drv:
	$(MAKE) -C $(PROJECT_DIR) drivers

ut:
	$(MAKE) -C $(PROJECT_DIR) ut $@

hal:
	$(MAKE) -C $(PROJECT_DIR) hal_generator

clean_objs:
	for f in `ls $(PROJECT_DIR)/target/ | grep -v ut` ; do rm -rf $(PROJECT_DIR)/target/$$f ; done 

clean:
	rm -rf $(PROJECT_DIR)/target/* $(AGH_DONE)
	rm -rf $(PROJECT_DIR)/firmware_bin/runner_fw_*.bak
	rm -rf $(RDPA_SRC_DIR)/rdp_version.h.bak

tarball_cms: tarball_prepare_dirs
	@echo "********************************************"
	@echo "Tarball for CMS, includes OREN, G9991, DSL  "
	@echo "********************************************"
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/PON_6838/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/PON_6838_G9991/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/DSL_63138/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/DSL_63148/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))
	$(foreach src,$(shell grep "/" $(PROJECTS_DIR)/WL4908/sources.list),$(shell $(CP) --parent $(src) $(TARBALL_DIR)/))
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

run_$(ALL):
	@echo "********************************************"
	@echo "Running $(ALL) for all platforms "
	@echo "********************************************"
	$(MAKE) PROJECT=PON_6838 clean $(ALL)
	$(MAKE) PROJECT=PON_6838_G9991 clean $(ALL)
	$(MAKE) PROJECT=DSL_63138 clean $(ALL)
	$(MAKE) PROJECT=DSL_63148 clean $(ALL)
	$(MAKE) PROJECT=PON_6858 clean $(ALL)
	$(MAKE) PROJECT=PON_6858_G9991 clean $(ALL)
	$(MAKE) PROJECT=WL4908 clean $(ALL)
	
run_$(XRDP):
	@echo "********************************************"
	@echo "Running $(XRDP) for XRDP platforms "
	@echo "********************************************"
	$(MAKE) PROJECT=PON_6858 clean $(XRDP)
	$(MAKE) PROJECT=PON_6858_G9991 clean $(XRDP)
	$(MAKE) PROJECT=PON_68360 clean $(XRDP)
	
opt:
	$(RM) -rf $(PROJECT_DIR)_OPT
	$(CP) -r $(PROJECT_DIR) $(PROJECT_DIR)_OPT
