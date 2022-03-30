#***********************************************************************
#
#  Copyright (c) 2007  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

# GNUMAKEFLAGS := --output-sync=target

all: router_all

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
export BUILD_DIR
include $(BUILD_DIR)/make.common
# export INSTALL from make.common to implicitly affect all Makefiles/configures below
export INSTALL

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

#
# The build sequence of userspace is:
# public-include, circular_dep_check, autobuild_dirs
# circular_dep_check will also fail if it detects non-existent targets, see
# ouptput.txt for full output.
#
public-include: $(BUILD_DIR)/cms_dmp_flags.h
	mkdir -p $(BCM_FSBUILD_DIR)/public/include
	mkdir -p $(BCM_FSBUILD_DIR)/public/lib
	$(MAKE) -C public/include
	$(MAKE) -C ../bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router fsbuild

CIRC.MAKE   := makefile.circ
CIRC.OUTPUT := circ_output.txt

circular_dep_check: public-include
	@echo "include makefile.modsw.autogen.mini" > $(CIRC.MAKE)
	@echo "" >> $(CIRC.MAKE)
	@echo '$$(AUTODIRS) :' >> $(CIRC.MAKE)
	@echo "" >> $(CIRC.MAKE)
	@echo 'circ_check: $$(AUTODIRS)' >> $(CIRC.MAKE)
	make -f $(CIRC.MAKE) circ_check > $(CIRC.OUTPUT) 2>&1
	@if grep Circular $(CIRC.OUTPUT) ; then \
		echo "ERROR!!  Circular dependency detected!  Abort build."; \
		exit 1; \
	fi

$(AUTODIRS) : circular_dep_check

.PHONY: public-include circular_dep_check router_all

clean:
	@for dir in $(AUTODIRS); do \
	  $(MAKE) -C $$dir clean; \
	done
	@rm -f $(CIRC.MAKE) $(CIRC.OUTPUT)
	@rm -f makefile.modsw.autogen makefile.modsw.autogen.mini
	@$(MAKE) -C public/include clean

