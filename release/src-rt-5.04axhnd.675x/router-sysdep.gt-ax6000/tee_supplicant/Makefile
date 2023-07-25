# TEE Supplicant makefile
OPTEE_VER := 3.2.0
OPTEE_TAR := $(OPTEE_VER).tar.gz
OPTEE_DIR := optee_client-$(OPTEE_VER)
OPTEE_OUT := $(OPTEE_DIR)/out/

all: conditional_build

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR := $(HND_SRC)
include $(BUILD_DIR)/make.common


# ifeq ($(strip $(BCM_OPTEE)),y)
conditional_build: tee-supplicant
clean: tee_supplicant_clean
# else
# conditional_build clean: ;
# endif

$(OPTEE_DIR)/Makefile: $(OPTEE_TAR)
	@rm -rf $(OPTEE_DIR)
	@echo "Untarring source and overrides..."
	@tar xf $(OPTEE_TAR) 2> /dev/null
	@patch -p1 -b -N -s -d$(OPTEE_DIR) < client-$(OPTEE_VER).patch

tee-supplicant: $(OPTEE_DIR)/Makefile
	$(MAKE) -C $(OPTEE_DIR) build-tee-supplicant
	@cp $(OPTEE_OUT)/tee-supplicant/tee-supplicant $(INSTALL_DIR)/bin/

tee_supplicant_clean:
	$(MAKE) -C $(OPTEE_DIR) clean
	-@rm -f $(INSTALL_DIR)/bin/tee-supplicant

bcm_dorel_distclean: distclean

distclean:
	-rm -rf $(OPTEE_DIR)

