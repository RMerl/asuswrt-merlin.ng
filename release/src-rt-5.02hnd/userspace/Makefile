#***********************************************************************
#
#  Copyright (c) 2007  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

# GNUMAKEFLAGS := --output-sync=target

all:	public-include router_all

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
export BUILD_DIR
include $(BUILD_DIR)/make.common

ifeq ($(wildcard ../targets/config.in),)
CONSUMER_BUILD=1
endif

MODSW_GLOBAL=1
include $(BUILD_DIR)/make.modsw
include $(SRCBASE)/router/common.mak
include $(SRCBASE)/.config
include $(HND_SRC)/targets/$(PROFILE)/$(PROFILE)

export TOPBUILD := y
	
router_all    : public-include
	$(MAKE) -C $(HND_SRC)/router all
	$(MAKE) -C $(HND_SRC)/router install
	$(MAKE) -C $(HND_SRC)/router reinstall

$(AUTODIRS) : public-include

public-include:
	$(MAKE) -C public/include
	$(MAKE) -C ../bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router fsbuild

.PHONY: public-include

.PHONY: router_all

clean:
	@for dir in $(AUTODIRS); do \
	  $(MAKE) -C $$dir clean; \
	done
	@rm -f makefile.modsw.autogen
	@$(MAKE) -C public/include clean


