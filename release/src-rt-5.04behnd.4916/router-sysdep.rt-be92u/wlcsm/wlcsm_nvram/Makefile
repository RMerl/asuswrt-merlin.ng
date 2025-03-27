#
# In most cases, you only need to modify this first section.
#

default:all

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
GENERATE = $(CURR_DIR)/generate_from_dm_wl.pl
include $(BUILD_DIR)/make.common

DMFILE2 := cms-dm-tr181-wifi-generic.xml
export PERL5LIB=$(BUILD_DIR)/data-model

DM_GENERATED_FILES := $(CURR_DIR)/../src/wlcsm_nvram_mngr_mapping.c
DM_GENERATED_FILES += $(CURR_DIR)/../include/wlcsm_nvram_mngr_mapping.h

all:$(DM_GENERATED_FILES)

$(DM_GENERATED_FILES): $(BUILD_DIR)/data-model/$(DMFILE2) $(GENERATE) 
	-for fl in $(DM_GENERATED_FILES); do ( rm -rf $$fl; ); done
	$(GENERATE) wlcsm $(BUILD_DIR) $(DMFILE2) $(DMFILE2)
	@touch $@

include $(BUILD_DIR)/make.deprules
