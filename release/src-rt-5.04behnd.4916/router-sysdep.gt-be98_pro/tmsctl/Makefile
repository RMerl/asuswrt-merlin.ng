
# GLOBAL_RELEASE_SCRIPT_NOTOUCH

default: conditional_build

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
export BUILD_DIR

include $(BUILD_DIR)/make.common


ifneq ($(strip $(BUILD_TMS)),)

conditional_build: tmsctl


else

conditional_build:
	@echo "skipping TMS ( TMS not configured)"

endif

make_links:
	@if [ -e $(KERNEL_DIR)/.config ] ; then \
	tmsimpl=`egrep -h '^CONFIG_BCM_TMS_IMPL=' $(KERNEL_DIR)/.config 2>/dev/null | head -1` ; \
	tmsimpl=impl$${tmsimpl#CONFIG_BCM_TMS_IMPL=} ; \
	else \
	tmsimpl=impl1; \
	fi; \
	rm -f tmsctl/bsp; ln -sfn $(BRCMDRIVERS_DIR)/broadcom/char/tms/$$tmsimpl/bsp tmsctl/bsp ; \
	rm -f tmsctl/nci; ln -sfn $(BRCMDRIVERS_DIR)/broadcom/char/tms/$$tmsimpl/nci tmsctl/nci

.PHONY: make_links

tmsctl: make_links
	$(MAKE) -C tmsctl

.PHONY: tmsctl

clean: make_links
	$(MAKE) -C tmsctl clean

binaryonly_dist_clean:
	$(MAKE) -C tmsctl $@ 

.PHONY: binaryonly_dist_clean

shell:
	bash -i

install:
	$(MAKE) -C tmsctl install
