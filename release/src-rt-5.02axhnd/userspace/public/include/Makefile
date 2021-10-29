#***********************************************************************
#
#  Copyright (c) 2008  Broadcom Corporation
#  All Rights Reserved
#
#  <:label-BRCM:2012:DUAL/GPL:standard
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License, version 2, as published by
#  the Free Software Foundation (the "GPL").
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  
#  A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
#  writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#  Boston, MA 02111-1307, USA.
#  
#  :>
#
#***********************************************************************/

VERSION_FILE=cms_version.h
DHCP_CONFIG_KEY_FILE=dhcp_config_key.h
DHCP_CONFIG_KEY=$(shell date +%N)

all install: $(VERSION_FILE) $(DHCP_CONFIG_KEY_FILE)

clean: force $(VERSION_FILE)

force:
	rm -f $(VERSION_FILE)
	rm -f $(DHCP_CONFIG_KEY_FILE)

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



#
# We only need one rule in this Makefile, which is the one for dynamically
# generating the version.h file.
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
