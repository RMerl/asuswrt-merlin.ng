
VERSION_FILE=cms_version.h
DHCP_CONFIG_KEY_FILE=dhcp_config_key.h
DHCP_CONFIG_KEY=$(shell date +%N)

# These headers have no home in any lib, so install them from here.
# Headers for cms_util cms_msg etc should be moved into and installed from
# their lib dirs.
EXTERNAL_HEADERS := $(VERSION_FILE) $(DHCP_CONFIG_KEY_FILE) \
                    linux/number_defs.h linux/os_defs.h \
                    bcm_retcodes.h bcm_generic_hal_defs.h \
                    cms.h cms_retcodes.h cms_params.h cms_params_modsw.h \
                    cms_actionlog.h \
                    bdk.h bdk_dbus.h \
                    beep.h beep_container.h openplat.h \
                    bdmf_api.h spdt_api.h vlan_subif.h \
                    bcm_mcast_api_public.h bcm_optrx.h bcm_queue.h \
                    log_colors.h \
                    omci_msg.h \
                    uuid.h

all: install

clean: force

force:
	rm -f $(VERSION_FILE)
	rm -f $(DHCP_CONFIG_KEY_FILE)
	rm -f $(addprefix $(HEADER_INSTALL_DIR)/,$(notdir $(EXTERNAL_HEADERS)))



install: $(VERSION_FILE) $(DHCP_CONFIG_KEY_FILE)
	mkdir -p $(HEADER_INSTALL_DIR)
	$(INSTALL_HEADERS_WITH_CP) $(EXTERNAL_HEADERS) $(HEADER_INSTALL_DIR)

#
# Dynamically generate the version.h file.
#
cms_version.h:
	@echo "generating $(VERSION_FILE)"
	@echo "/* this is an automatically generated file.  Do not modify. */" > $(VERSION_FILE)
	@echo "/* modify $(VERSION_MAKE_FILE) at the top. */" >> $(VERSION_FILE)
	@echo "#ifndef _CMS_VERSION_H_" >> $(VERSION_FILE)
	@echo "#define _CMS_VERSION_H_" >> $(VERSION_FILE)
	@echo -n "#define CMS_RELEASE_VERSION \"" >> $(VERSION_FILE)
	@echo -n $(BRCM_VERSION) >> $(VERSION_FILE)
	@echo -n "." >> $(VERSION_FILE)
	@echo -n $(BRCM_RELEASE) >> $(VERSION_FILE)
	@echo -n "L." >> $(VERSION_FILE)
	@echo -n $(BRCM_EXTRAVERSION) >> $(VERSION_FILE)
	@echo "\"" >> $(VERSION_FILE)
	@echo "#endif" >> $(VERSION_FILE)

#
# For dynamically generating the DHCP config key, which will be used to access
# DHCP configurations
#
$(DHCP_CONFIG_KEY_FILE):
	@echo "generating $(DHCP_CONFIG_KEY_FILE)"
	@echo "/* this is an automatically generated file.  Do not modify. */" > $(DHCP_CONFIG_KEY_FILE)
	@echo "#define BRCM_DHCP_CONFIG_KEY    \"$(DHCP_CONFIG_KEY)\"" >> $(DHCP_CONFIG_KEY_FILE)
	@echo "static int dhcpEncryptCfgFile(char *buff, int buffLen, const char *key) { " >> $(DHCP_CONFIG_KEY_FILE)
	@echo "    int i, k, keyLen = strlen(key);                                       " >> $(DHCP_CONFIG_KEY_FILE)
	@echo "    for (i = 0; i < buffLen; i++) {                                       " >> $(DHCP_CONFIG_KEY_FILE)
	@echo "        for (k = 0; k < keyLen; k++) {                                    " >> $(DHCP_CONFIG_KEY_FILE)
	@echo "            buff[i] = buff[i] ^ key[k];                                   " >> $(DHCP_CONFIG_KEY_FILE)
	@echo "        }                                                                 " >> $(DHCP_CONFIG_KEY_FILE)
	@echo "    }                                                                     " >> $(DHCP_CONFIG_KEY_FILE)
	@echo "    return 0;                                                             " >> $(DHCP_CONFIG_KEY_FILE)
	@echo "}                                                                         " >> $(DHCP_CONFIG_KEY_FILE)
