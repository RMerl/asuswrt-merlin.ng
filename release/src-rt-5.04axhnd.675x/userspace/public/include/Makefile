#***********************************************************************
#
#  Copyright (c) 2008  Broadcom Corporation
#  All Rights Reserved
#
#  <:label-BRCM:2012:DUAL/GPL:standard
#
#  Unless you and Broadcom execute a separate written software license
#  agreement governing use of this software, this software is licensed
#  to you under the terms of the GNU General Public License version 2
#  (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
#  with the following added to such license:
#
#     As a special exception, the copyright holders of this software give
#     you permission to link this software with independent modules, and
#     to copy and distribute the resulting executable under terms of your
#     choice, provided that you also meet, for each linked independent
#     module, the terms and conditions of the license of that module.
#     An independent module is a module which is not derived from this
#     software.  The special exception does not apply to any modifications
#     of the software.
#
#  Not withstanding the above, under no circumstances may you combine
#  this software in any way with any other Broadcom software provided
#  under a license other than the GPL, without Broadcom's express prior
#  written consent.
#
#  :>
#
#***********************************************************************/

VERSION_FILE=cms_version.h
DHCP_CONFIG_KEY_FILE=dhcp_config_key.h
DHCP_CONFIG_KEY=$(shell date +%N)
EXTERNAL_HEADERS := $(VERSION_FILE) linux/number_defs.h

all: install

clean: force

force:
	rm -f $(VERSION_FILE)
	rm -f $(DHCP_CONFIG_KEY_FILE)
	rm -f $(addprefix $(HEADER_INSTALL_DIR)/,$(notdir $(EXTERNAL_HEADERS)))

modsw_clean: generic_modsw_base_clean


#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common


HEADER_INSTALL_DIR := $(BCM_FSBUILD_DIR)/public/include
#
# We should probably install all headers in fs.build instead of having other
# apps include from the public/include dir.  But for now, just install a few.
#
install: $(VERSION_FILE) $(DHCP_CONFIG_KEY_FILE)
	mkdir -p $(HEADER_INSTALL_DIR)
	install -p -t $(HEADER_INSTALL_DIR) $(EXTERNAL_HEADERS)

#
# Dynamically generate the version.h file.
#
cms_version.h: $(BUILD_DIR)/$(VERSION_MAKE_FILE)
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
