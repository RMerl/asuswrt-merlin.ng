
# RT Settings files in the scripts_new directory
RT_SETTINGS_FILES = rt_settings_kthreads.txt \
                    rt_settings_apps.txt \
                    rt_settings_interrupts.txt

SYMBOL_TAB := symbol_table.txt
REALTIME_LEVELS := realtime_levels.txt


ifneq (,$(wildcard $(KERNEL_DIR)/include/linux/bcm_realtime.h))
$(REALTIME_LEVELS):
	@grep '#define BCM_RTPRIO_' $(KERNEL_DIR)/include/linux/bcm_realtime.h | sed -e 's/#define//g' > $@
else
# Look for bcm_realtime.h in its new location in new kernel.
# All this stuff will be moved out of CMS for 5.04L.01.
$(REALTIME_LEVELS):
	@grep '#define BCM_RTPRIO_' $(BUILD_DIR)/kernel/bcmkernel/include/uapi/linux/bcm_realtime.h | sed -e 's/#define//g' > $@
endif

$(SYMBOL_TAB): $(REALTIME_LEVELS)
	@echo Generating $@
	@cat $(REALTIME_LEVELS) >> $@


all: $(SYMBOL_TAB)

SCRIPTS := scripts/rtpolicy
RT_SETTINGS_FILES_INSTALL_DIR :=$(BCM_FSBUILD_DIR)/etc/rt_policy_info.d
RT_POLICY_INFO_METADATA := rt_settings_metadata
GEN_METADATA_FILE := ./scripts/gen_rt_settings_metadata.pl
FINAL_RT_SETTINGS_FILES_INSTALL_DIR := $(INSTALL_DIR)/etc/rt_policy_info.d

install: all
	install -p $(SCRIPTS) $(INSTALL_DIR)/bin
	mkdir -p $(RT_SETTINGS_FILES_INSTALL_DIR)
	install -p -m 444 -t $(RT_SETTINGS_FILES_INSTALL_DIR) $(SYMBOL_TAB) $(addprefix $(mdir)/scripts/,$(RT_SETTINGS_FILES))
	$(GEN_METADATA_FILE) $(RT_SETTINGS_FILES_INSTALL_DIR) $(RT_SETTINGS_FILES_INSTALL_DIR)/$(RT_POLICY_INFO_METADATA)
	rm -rf $(INSTALLDIR)/rom/etc/rt_policy_info.d
	mkdir -p $(INSTALLDIR)/rom/etc
	cp -rf $(RT_SETTINGS_FILES_INSTALL_DIR) $(INSTALLDIR)/rom/etc/

clean:
	rm -rf $(RT_SETTINGS_FILES_INSTALL_DIR)
	rm -f $(SYMBOL_TAB) $(REALTIME_LEVELS)


# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
