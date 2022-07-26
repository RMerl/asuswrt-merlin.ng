default: all


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common

SCRIPTS := scripts/rtpolicy

RT_SETTINGS_FILES_INSTALL_DIR :=$(BCM_FSBUILD_DIR)/etc/rt_policy_info.d
RT_POLICY_INFO_METADATA := rt_settings_metadata
GEN_METADATA_FILE := ./scripts/gen_rt_settings_metadata.pl

export RT_SETTINGS_FILES_INSTALL_DIR

# Final location of scripts for system image.  Only the BRCM build system needs to
# know about this.
FINAL_RT_SETTINGS_FILES_INSTALL_DIR := $(INSTALL_DIR)/etc/rt_policy_info.d

# RT Settings files in the scripts_new directory
RT_SETTINGS_FILES = rt_settings_kthreads.txt \
                    rt_settings_apps.txt \
                    rt_settings_interrupts.txt \
                    symbol_table.txt


all install: $(SCRIPTS)
	install -p $(SCRIPTS) $(INSTALL_DIR)/bin
	$(MAKE) install
	mkdir -p $(FINAL_RT_SETTINGS_FILES_INSTALL_DIR)
	install -p -m 444 -t $(FINAL_RT_SETTINGS_FILES_INSTALL_DIR) $(addprefix $(RT_SETTINGS_FILES_INSTALL_DIR)/,$(RT_SETTINGS_FILES)) 

gen_metadata: $(GEN_METADATA_FILE)
	$(GEN_METADATA_FILE) $(FINAL_RT_SETTINGS_FILES_INSTALL_DIR) $(FINAL_RT_SETTINGS_FILES_INSTALL_DIR)/$(RT_POLICY_INFO_METADATA)

clean:
	rm -rf $(FINAL_RT_SETTINGS_FILES_INSTALL_DIR)
	-$(MAKE) clean

shell:
	bash -i
