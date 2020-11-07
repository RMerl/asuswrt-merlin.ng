default: all

#
# Figure out where we are so we can include make.common.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /data-model, /data-model,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common


#
# Local defines for the data-model directory.
#
GENERATE     := ./generate_from_dm.pl
PERL_MODULES := GenObjectNode.pm GenParamNode.pm Utils.pm


#
# DMFILE is the hybrid data model (TR98 + some TR181 parts)
#
DMFILE        := cms-data-model-merged.xml
BBF_DMFILE    := bbf-data-model-1.xml
MERGE_IGD_DIR := merge-igd.d

#
# DMFILE2 is the Pure TR181 data model.
# Retail Wifi routers use a smaller version of TR181.
#
ifneq ($(strip $(BUILD_DM_WLAN_RETAIL)),)
DMFILE2        := cms-data-model-retail-merged2.xml
BBF_DMFILE2    := bbf-data-model-retail-2.xml
MERGE_DEV2_DIR := merge-retail2.d
else
DMFILE2        := cms-data-model-merged2.xml
BBF_DMFILE2    := bbf-data-model-2.xml
MERGE_DEV2_DIR := merge-dev2.d
endif


#
# There are 2 versions of TR104: v1 and v2.  Copy the right version
# to the file that will be used.
#
DMFILE_TR104 := cms-dm-tr104-voice.xml
$(DMFILE_TR104): cms-dm-tr104v$(if $(BUILD_DM_TR104_V2),2,1)-voice.xml
	cp -f $< $@
#
# We need TR181 version of TR104 profile seperated 
# because Baseline:1 defined in TR104 would be changed to
# Device2_Baseline:1 in TR181 merged file
#
DMFILE_TR104_TR181 := cms-dm-tr104-voice-tr181.xml
$(DMFILE_TR104_TR181): cms-dm-tr104v$(if $(BUILD_DM_TR104_V2),2,1)-voice.xml
	cat $< | sed 's/Baseline:1/Device2_Baseline:1/g' > $(DMFILE_TR104_TR181)


# Packet cable (voice) stuff
MERGEFILE_SNMPPKTCBL_IGD  := $(MERGE_IGD_DIR)/8000-bcm-snmppktcbl.txt
MERGEFILE_SNMPPKTCBL_DEV2 := $(MERGE_DEV2_DIR)/8000-bcm-snmppktcbl.txt
ifeq ($(strip $(BRCM_PKTCBL_SUPPORT)),y)
    $(shell cp -f $(MERGEFILE_SNMPPKTCBL_IGD).tmp $(MERGEFILE_SNMPPKTCBL_IGD))
    $(shell cp -f $(MERGEFILE_SNMPPKTCBL_DEV2).tmp $(MERGEFILE_SNMPPKTCBL_DEV2))
endif

# Dual TR69 support.
MERGEFILE_BCM_E2E_DEV2 := $(MERGE_DEV2_DIR)/50000-bcm-e2e.txt
ifeq ($(strip $(BRCM_MULTIPLE_TR69C_SUPPORT)),2)
    $(shell cp -f $(MERGEFILE_BCM_E2E_DEV2).tmp $(MERGEFILE_BCM_E2E_DEV2))
endif

# Support Customer customizations
ifeq ($(strip $(BUILD_CUSTOMER)),y)

MERGEFILE_CUSTOMER := 8100-$(BUILD_CUSTOMER_NAME)-features.txt
$(shell cp -f customers/$(BUILD_CUSTOMER_NAME)/$(MERGEFILE_CUSTOMER) $(MERGE_IGD_DIR))

clean_customer_files:
	rm -f $(MERGE_IGD_DIR)/$(MERGEFILE_CUSTOMER)

else
# nothing to clean.
clean_customer_files:

endif


# Choose the right TR181 wifi file to use.
DMFILE_TR181_WIFI := cms-dm-tr181-wifi.xml
ifeq ($(BUILD_HND_EAP),y)
    $(shell cp -pf cms-dm-tr181-wifi-eap.xml $(DMFILE_TR181_WIFI))
else ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
    $(shell cp -pf cms-dm-tr181-wifi-unfwlcfg.xml $(DMFILE_TR181_WIFI))
else
    $(shell cp -pf cms-dm-tr181-wifi-generic.xml $(DMFILE_TR181_WIFI))
endif


# The DMFILE must be regenerated anytime one of the data model files changes.
# For this detection mechanism to work correctly, all data model files
# must match the pattern of cms-dm*.xml
DM_FILES := $(shell ls -w 5000 -x cms-dm*.xml) $(DMFILE_TR104) $(DMFILE_TR104_TR181)

#
# Generate all the files from the data model file.
#
all:
	echo "skip..."

#all: $(DMFILE) $(DMFILE2) $(BBF_DMFILE) $(BBF_DMFILE2) \
     $(BUILD_DIR)/userspace/public/include/mdm_objectid.h  \
     $(BUILD_DIR)/userspace/public/include/mdm_object.h \
     $(BUILD_DIR)/userspace/public/include/mdm_validstrings.h \
     $(BUILD_DIR)/userspace/public/include/mdm_params.h \
     $(BUILD_DIR)/router-sysdep/mdm/Baseline_1.c \
     $(BUILD_DIR)/router-sysdep/mdm2/Device2_Baseline_1.c \
     $(BUILD_DIR)/router-sysdep/cms_core/mdm_oidInfoArray.c \
     $(BUILD_DIR)/router-sysdep/cms_core/mdm2_oidInfoArray.c \
     $(BUILD_DIR)/router-sysdep/cms_core/rcl.h \
     $(BUILD_DIR)/router-sysdep/cms_core/stl.h

$(DMFILE): $(GENERATE) $(PERL_MODULES) $(MERGE_IGD_DIR) $(DM_FILES)
	$(GENERATE) merge $(BUILD_DIR) $(MERGE_IGD_DIR) $(DMFILE)

$(DMFILE2): $(GENERATE) $(PERL_MODULES) $(MERGE_DEV2_DIR) $(DM_FILES)
	$(GENERATE) merge2 $(BUILD_DIR) $(MERGE_DEV2_DIR) $(DMFILE2)

$(BUILD_DIR)/router-sysdep/mdm/Baseline_1.c: $(DMFILE)
	$(GENERATE) mdm $(BUILD_DIR) $(DMFILE)

$(BUILD_DIR)/router-sysdep/mdm2/Device2_Baseline_1.c: $(DMFILE2)
	$(GENERATE) mdm2 $(BUILD_DIR) $(DMFILE2)

$(BUILD_DIR)/userspace/public/include/mdm_params.h: $(DMFILE) $(DMFILE2)
	$(GENERATE) mdmparams $(BUILD_DIR) $(DMFILE) $(DMFILE2)

$(BUILD_DIR)/userspace/public/include/mdm_objectid.h: $(DMFILE) $(DMFILE2)
	$(GENERATE) objectid $(BUILD_DIR) $(DMFILE) $(DMFILE2)

$(BUILD_DIR)/userspace/public/include/mdm_object.h: $(DMFILE) $(DMFILE2)
	$(GENERATE) object $(BUILD_DIR) $(DMFILE) $(DMFILE2)

$(BUILD_DIR)/userspace/public/include/mdm_validstrings.h: $(DMFILE) $(DMFILE2)
	$(GENERATE) validstrings $(BUILD_DIR) $(DMFILE) $(DMFILE2)

$(BUILD_DIR)/router-sysdep/cms_core/rcl.h $(BUILD_DIR)/router-sysdep/cms_core/stl.h: $(DMFILE) $(DMFILE2)
	$(GENERATE) prototypes $(BUILD_DIR) $(DMFILE) $(DMFILE2)

$(BUILD_DIR)/router-sysdep/cms_core/mdm_oidInfoArray.c: $(DMFILE)
	$(GENERATE) oidinfo $(BUILD_DIR) $(DMFILE)
	
$(BUILD_DIR)/router-sysdep/cms_core/mdm2_oidInfoArray.c: $(DMFILE2)
	$(GENERATE) oidinfo2 $(BUILD_DIR) $(DMFILE2)


#
# This is for TR69 reporting of supported data model.
#
ifneq ($(strip $(BUILD_PROFILE_SUPPORTED_DATA_MODEL)),)

WEBS_DATA_MODEL_DIR := $(INSTALL_DIR)/webs/data-model
BBF_BUILD           := $(BUILD_DIR)/hostTools/cms2bbf/build.xml

$(BBF_DMFILE): $(DMFILE) bbf_dmfile_build bbf_dmfile_install
	@echo "Done building $(BBF_DMFILE)"

bbf_dmfile_build:
	ant -f $(BBF_BUILD) run -Darg0=$(CURR_DIR)/$(DMFILE) -Darg1=$(CURR_DIR)/$(BBF_DMFILE)

ifeq ($(strip $(BUILD_PURE181_PROFILES)),)
bbf_dmfile_install:
	-mkdir -p $(WEBS_DATA_MODEL_DIR)
	cp -d $(BBF_DMFILE) $(WEBS_DATA_MODEL_DIR)/$(BBF_DMFILE)
else
bbf_dmfile_install:
endif

$(BBF_DMFILE2): $(DMFILE2) bbf_dmfile2_build bbf_dmfile2_install
	@echo "Done building $(BBF_DMFILE2)"

bbf_dmfile2_build:
	ant -f $(BBF_BUILD) run -Darg0=$(CURR_DIR)/$(DMFILE2) -Darg1=$(CURR_DIR)/$(BBF_DMFILE2)

ifneq ($(strip $(BUILD_PURE181_PROFILES)),)
bbf_dmfile2_install:
	-mkdir -p $(WEBS_DATA_MODEL_DIR)
	cp -d $(BBF_DMFILE2) $(WEBS_DATA_MODEL_DIR)/$(BBF_DMFILE2)
else
bbf_dmfile2_install:
endif

clean_bbf_files:
	rm -f $(BBF_DMFILE) $(BBF_DMFILE2)
	rm -rf $(WEBS_DATA_MODEL_DIR)

else

$(BBF_DMFILE):
	@echo "skipping $(BBF_DMFILE) (not configured)"

$(BBF_DMFILE2):
	@echo "skipping $(BBF_DMFILE2) (not configured)"

clean_bbf_files:
	@echo "skipping clean BBF data model files (not configured)"

endif


report: $(DMFILE) $(DMFILE2)
	$(GENERATE) report $(BUILD_DIR) $(DMFILE)
	$(GENERATE) report2 $(BUILD_DIR) $(DMFILE2)


clean: clean_bbf_files clean_customer_files
	rm -f $(DMFILE) $(DMFILE2) $(DMFILE_TR104) $(DMFILE_TR104_TR181) $(MERGEFILE_SNMPPKTCBL_IGD) $(MERGEFILE_SNMPPKTCBL_DEV2) $(MERGEFILE_BCM_E2E_DEV2) $(DMFILE_TR181_WIFI)
	rm -f $(BUILD_DIR)/userspace/public/include/mdm_objectid.h
	rm -f $(BUILD_DIR)/userspace/public/include/mdm_object.h
	rm -f $(BUILD_DIR)/userspace/public/include/mdm_params.h
	rm -f $(BUILD_DIR)/userspace/public/include/mdm_validstrings.h
	rm -f $(BUILD_DIR)/router-sysdep/cms_core/mdm_oidInfoArray.c
	rm -f $(BUILD_DIR)/router-sysdep//cms_core/mdm2_oidInfoArray.c
	rm -f $(BUILD_DIR)/router-sysdep/cms_core/rcl.h
	rm -f $(BUILD_DIR)/router-sysdep/cms_core/stl.h
	rm -f $(BUILD_DIR)/router-sysdep/mdm/*.c
	rm -f $(BUILD_DIR)/router-sysdep/mdm2/*.c

