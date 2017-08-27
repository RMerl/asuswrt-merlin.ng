#***********************************************************************
#
#  Copyright (c) 2014  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

all dynamic install: conditional_build


#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common

# Download from http://git.libwebsockets.org/cgi-bin/cgit/libwebsockets/snapshot/libwebsockets-1.6.2.tar.gz
APP = libwebsockets
VERSION = 1.6.2
LIB = $(APP).so
INCLUDE_DIR = $(BCM_FSBUILD_DIR)/public/include/websockets

.PHONY: check_untar_patch_configure conditional_build check_versions clean distclean

ifneq ($(strip $(BUILD_WEB_SOCKETS)),)
ifneq ($(strip $(DO_BUILD_OPENSSL)),)
conditional_build: $(APP)

# Depend on tar file
# Use $(APP)/lib/Makefile to mark tar progress is completed
$(APP)/lib/Makefile:
	#if [ ! -e $(APP) ]; then \
	    mkdir $(APP); cd $(APP); \
	    tar --strip-components=1 -xzf ../$(APP)-$(VERSION).tar.gz; \
	    cp -d ../Makefile.bcm lib/Makefile; \
	    cp -d ../lws_config.h lib/lws_config.h; \
	    cp -d ../lws_config_private.h lib/lws_config_private.h; \
	    cd ..; \
	fi;
	touch -c $(APP)/lib/Makefile; \
	#echo "$(APP) is untarred"

# Depend on $(APP)/lib/Makefile so that tar progress should be done only once
check_config: $(APP)/lib/Makefile

# Depend on check_config so that make progress can continue where it is stopped before
$(APP): check_config
	make -C $(APP)/lib/;
	mkdir -p $(INSTALL_DIR)/lib/
	cp -T $(APP)/lib/$(LIB) $(INSTALL_DIR)/lib/$(LIB);
	$(STRIP) $(INSTALL_DIR)/lib/$(LIB);
	if [ ! -e $(INCLUDE_DIR) ]; then \
	    mkdir $(INCLUDE_DIR); \
	fi;
	cp -f $(APP)/lib/lws_config.h $(INCLUDE_DIR)/.;
	cp -f $(APP)/lib/libwebsockets.h $(INCLUDE_DIR)/.;
	@echo "Done building $(APP)"

else
conditional_build: sanity_check
	@echo "Error $(APP) requires OpenSSL" 1>&2
	exit 1
endif
else
conditional_build: sanity_check
	@echo "skipping $(APP) (not configured)"
endif

# NOTE: make clean from within app does not do a proper job, so wiping out
# entire directory to ensure consistency.
clean:
	rm -f $(INSTALL_DIR)/lib/$(LIB);
	#rm -rf $(APP) $(INCLUDE_DIR);

# The next line is a hint to our release scripts
# GLOBAL_RELEASE_SCRIPT_CALL_DISTCLEAN
distclean: clean

bcm_dorel_distclean: distclean

shell:
	bash -i
