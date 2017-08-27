
default: conditional_build 

# BRCM_SUPPORTS_MULTIARCH_BUILD

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common

#ifneq ($(strip $(BUILD_PPPD)),)
conditional_build: all
#else
#conditional_build:
#	@echo "skipping pppd (not configured)"
#endif


all: 
	#$(MAKE) -C ppp/pppoe $(BUILD_PPPD)
	$(MAKE) -C ppp/pppoe

clean:
	$(MAKE) -C ppp/pppoe clean


shell:
	bash -i

