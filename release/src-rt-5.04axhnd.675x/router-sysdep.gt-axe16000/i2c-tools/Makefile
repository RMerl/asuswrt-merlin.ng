# Makefile to build i2c tools
#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

all dynamic install:

-include $(BUILD_DIR)/make.common

APP = i2c-tools-4.0

#ifneq ($(strip $(BRCM_DRIVER_I2C)),)

all dynamic install: conditional_build

clean:
	@if [ -e $(APP)/Makefile ]; then \
		cd $(APP); $(MAKE) clean ; \
	fi

distclean: clean
	echo "skip distclean"
	#rm -rf $(APP)

check_untar_patch_configure: sanity_check
	if [ ! -e $(APP)/Makefile.in ]; then \
		echo "Untarring source and overrides..." ; \
		(tar xfj $(APP).tar.bz2 2> /dev/null || true) ; \
		cd $(APP); \
	fi

#conditional_build: check_untar_patch_configure
conditional_build: 
	echo "Making $(APP)"

	cd $(APP); \
	make; \
	make install DESTDIR=$(BCM_FSBUILD_DIR) INSTALL="$(INSTALL)"
	mv -f $(BCM_FSBUILD_DIR)/usr/local/lib/libi2c*.so* $(INSTALL_DIR)/lib/
	mv -f $(BCM_FSBUILD_DIR)/usr/local/sbin/i2c* $(INSTALL_DIR)/bin/
	$(STRIP) $(INSTALL_DIR)/lib/libi2c*.so*
bcm_dorel_distclean: distclean

#else

#all dynamic install clean distclean:
#	@echo "i2c-tools not configured. Skipping."

#endif

# NOTE: make clean from within app does not do a proper job, so wiping out
# entire directory to ensure consistency.
clean:
	@if [ -e $(APP)/Makefile ]; then \
		cd $(APP); $(MAKE) clean ; \
	fi
	rm -rf $(APP)

# The next line is a hint to our release scripts
# GLOBAL_RELEASE_SCRIPT_CALL_DISTCLEAN
distclean: clean
	
bcm_dorel_distclean: distclean
