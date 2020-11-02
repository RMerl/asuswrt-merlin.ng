# Top level makefile for hndrte based standalone programs.
#
# Copyright 2020 Broadcom
#
# This program is the proprietary software of Broadcom and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
# WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
# THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use
# all reasonable efforts to protect the confidentiality thereof, and to
# use this information only in connection with your use of Broadcom
# integrated circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
# REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
# OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
# DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
# ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
# OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
# BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
# SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
# IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
# IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
# ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
# OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
# NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
# $Id$
#
# Descend to each subdir or specified subdir(s) and invoke subdir's Makefile.
#
# This is the front end of the wl dongle make system.
# The wl dongle make system consists of the following components:
#
# - Makefile and *.mk in each subdir
#   Makefile is customized for each subdir. Include Makefile.sub of the upper directory
#   in the subdir Makefile to create a generic subdir level make file.
# - Makefile.top, Makefile.sub
#   These are common make files either invoked or included from the Makefile in each subdir
# - Makeconf, Makerules
#   These are common wl config generation and comon make rules
# - src/wl/config/wl.mk, src/wl/config/wlconfig_xxxx
#   These are wl config to create wl driver file list and compile flags.
#
# Special targets:
#	help: display help text
#	showsubdirs: display all subdirs
#	showbrands: display all brands
#	<subdir>/showbrands: display all brands in the <subdir>
#	showtgts: display all targets
#	<subdir>/showtgts: display all targets in the <subdir>
#	clean: clean all targets
#	<subdir>/clean: clean all targets in the <subdir>
#	<subdir>/<brand>/clean: clean all targets in the <brand> in the <subdir>
#	<subdir>/<brand>/<target>/clean: clean specified target in the <brand> in the <subdir>
#
# Usage:
# 	make <subdir> | <subdir>/<brand> | <subdir>/<brand>/<target>
#

# Validate version of make.
min_version := 3.81
$(if $(filter $(min_version),$(firstword $(sort $(MAKE_VERSION) $(min_version)))),,\
  $(error This makefile requires GNU make version $(min_version). Please use /tools/bin/make))

# run silently by default
V ?= 0
ifeq ($(V),0)
ifeq ($(filter -s,$(MAKEFLAGS)),)
MAKEFLAGS += -s
endif
endif	# V

# run with no builtin rules
ifeq ($(filter -r,$(MAKEFLAGS)),)
MAKEFLAGS += -r
endif

# announce a few things everyone should know
export TOPDIR := $(shell /bin/pwd)
export TOPMK := Makefile.top

TOPBLDDIR ?= $(abspath $(TOPDIR)/../../../../build/dongle)

export REMAIN := 0

# image info
IMGSTAT ?= imgstat.log

# exclude from make target 'all' prerequisites
# romlsym will be built by relevant romlsim/roml builds
EXCLUDE_SUBDIRS_FROM_ALL := romlsym
# release subdir needs to be built explicitly
EXCLUDE_SUBDIRS_FROM_ALL += release

# figure out all subdirs we can work with
SUBDIRS := $(wildcard */Makefile)
SUBDIRS := $(SUBDIRS:/Makefile=)

############################################################################
# special rules/targets
############################################################################

# print all subdirs and show help text
help: FORCE
	@( \
	echo "The following subdirs are available:"; \
	$(foreach _SUBDIR,$(SUBDIRS),echo -e "\t$(_SUBDIR)";) echo -e ""; \
	echo "The following make variables are overridable:"; \
	echo -e "\tTOPBLDDIR=$(TOPBLDDIR)\n"; \
	echo "To build all subdirs:"; \
	echo -e "\tmake all\n"; \
	echo "To build all brands in a subdir:"; \
	echo -e "\tmake <subdir>\n"; \
	echo "To build a brand in a subdir:"; \
	echo -e "\tmake <subdir>/<brand>\n"; \
	echo "To build a target in a brand in a subdir:"; \
	echo -e "\tmake <subdir>/<brand>/<target>\n"; \
	echo "To list all subdirs:"; \
	echo -e "\tmake showsubdirs\n"; \
	echo "To list all brands:"; \
	echo -e "\tmake showbrands\n"; \
	echo "To list all default targets in all subdir:"; \
	echo -e "\tmake showtgts\n"; \
	echo "To list all default targets in a subdir:"; \
	echo -e "\tmake <subdir>/showtgts\n"; \
	echo "To list all default targets in a brand:"; \
	echo -e "\tmake <subdir>/<brand>/showtgts\n"; \
	echo "To clean all subdirs:"; \
	echo -e "\tmake clean\n"; \
	echo "To clean a subdir:"; \
	echo -e "\tmake <subdir>/clean\n"; \
	echo "To clean all targets in a brand:"; \
	echo -e "\tmake <subdir>/<brand>/clean\n"; \
	echo "To clean a target in a brand:"; \
	echo -e "\tmake <subdir>/<brand>/<target>/clean\n"; \
	)

# show all subdirs
showsubdirs: FORCE
	@$(foreach _SUBDIR,$(SUBDIRS),echo "$(_SUBDIR)";)

# show all brands
.PHONY: showbrands
showbrands: $(SUBDIRS:=/showbrands)
# show subdir specific brands
$(SUBDIRS:=/showbrands): _SUBDIR = $(firstword $(subst /, ,$@))
$(SUBDIRS:=/showbrands): FORCE
	$(MAKE) -C $(_SUBDIR) TOPBLDDIR=$(TOPBLDDIR) $(@:$(_SUBDIR)/%=%)

# show all build targets
.PHONY: showtgts
showtgts: $(SUBDIRS:=/showtgts)
# show subdir specific build targets
$(SUBDIRS:=/showtgts): _SUBDIR = $(firstword $(subst /, ,$@))
$(SUBDIRS:=/showtgts): FORCE
	$(MAKE) -C $(_SUBDIR) TOPBLDDIR=$(TOPBLDDIR) $(@:$(_SUBDIR)/%=%)
# show subdir specific build targets
$(SUBDIRS:=/%/showtgts): _SUBDIR = $(firstword $(subst /, ,$@))
$(SUBDIRS:=/%/showtgts): FORCE
	$(MAKE) -C $(_SUBDIR) TOPBLDDIR=$(TOPBLDDIR) $(@:$(_SUBDIR)/%=%)

# show all maxsized build targets
.PHONY: showmaxsize_tgts
showmaxsize_tgts: $(SUBDIRS:=/showmaxsize_tgts)
# show subdir specific maxsized build targets
$(SUBDIRS:=/showmaxsize_tgts): _SUBDIR = $(firstword $(subst /, ,$@))
$(SUBDIRS:=/showmaxsize_tgts): FORCE
	$(MAKE) -C $(_SUBDIR) TOPBLDDIR=$(TOPBLDDIR) $(@:$(_SUBDIR)/%=%)
# show subdir specific maxsized build targets
$(SUBDIRS:=/%/showmaxsize_tgts): _SUBDIR = $(firstword $(subst /, ,$@))
$(SUBDIRS:=/%/showmaxsize_tgts): FORCE
	$(MAKE) -C $(_SUBDIR) TOPBLDDIR=$(TOPBLDDIR) $(@:$(_SUBDIR)/%=%)

# clean all subdirs
.PHONY: clean
clean: $(SUBDIRS:=/clean)
	@# remove flat build subdir (for backward compatible mode)
ifeq ($(SUBBLDDIR),)
	$(MAKE) SUBBLDDIR=. clean
	-$(RM) $(IMGSTAT)
endif
# clean a specific subdir
$(SUBDIRS:=/clean): _SUBDIR = $(firstword $(subst /, ,$@))
$(SUBDIRS:=/clean): FORCE
	$(MAKE) -C $(_SUBDIR) TOPBLDDIR=$(TOPBLDDIR) $(@:$(_SUBDIR)/%=%)
# clean a specific subdir
$(SUBDIRS:=/%/clean): _SUBDIR = $(firstword $(subst /, ,$@))
$(SUBDIRS:=/%/clean): FORCE
	$(MAKE) -C $(_SUBDIR) TOPBLDDIR=$(TOPBLDDIR) $(@:$(_SUBDIR)/%=%)

############################################################################
# rules to make target(s)
############################################################################

# remove trailing '/'
%/: FORCE
	$(MAKE) $(@:/=)

# remove trailing '.mk'
%.mk: FORCE
	$(MAKE) $(@:.mk=)

# build all subdirs except those listed in $(EXCLUDE_SUBDIRS_FROM_ALL)
.PHONY: all
all: $(filter-out $(EXCLUDE_SUBDIRS_FROM_ALL),$(SUBDIRS))

.PHONY: imgstat
imgstat:
	stats=`find $(TOPBLDDIR) -name *.sz`; \
	echo "DBG: Concatenating image size files '$$stats' into $(IMGSTAT)"
	( \
	echo "Images size info:"; \
	echo "==========================================="; \
	stats=`find $(TOPBLDDIR) -name *.sz`; \
	for stat in $$stats; do \
	   info=`grep "INFO:\|ERROR:" $$stat`; \
	   if [ "$$info" != "" ]; then \
	      cat $$stat; \
	      echo "--------------------------------------------"; \
	   fi; \
	done \
	) > $(IMGSTAT)

# build all brands in a subdir
$(SUBDIRS): FORCE
ifeq ($(SUBBLDDIR),)
	$(MAKE) -C $@ SUBDIR=$@ SUBBLDDIR=$@ TOPBLDDIR=$(TOPBLDDIR)
else
	$(MAKE) -C $@ SUBDIR=$@ SUBBLDDIR=$(SUBBLDDIR) TOPBLDDIR=$(TOPBLDDIR)
endif

# build a specific brand in a subdir; or
# build a specific brand/target in a subdir
$(SUBDIRS:=/%): _SUBDIR = $(firstword $(subst /, ,$@))
$(SUBDIRS:=/%): FORCE
ifeq ($(SUBBLDDIR),)
	$(MAKE) -C $(_SUBDIR) SUBDIR=$(_SUBDIR) SUBBLDDIR=$(_SUBDIR) TOPBLDDIR=$(TOPBLDDIR) $(@:$(_SUBDIR)/%=%)
else
	$(MAKE) -C $(_SUBDIR) SUBDIR=$(_SUBDIR) SUBBLDDIR=$(SUBBLDDIR) TOPBLDDIR=$(TOPBLDDIR) $(@:$(_SUBDIR)/%=%)
endif

############################################################################
# rules to make the build directory structure backward compatible
############################################################################

# Remove "builds" prefix from the target name to facilitate
# the use of shell tab filename completion.
$(TOPBLDDIR)/%: FORCE
	$(MAKE) $(@:$(TOPBLDDIR)/%=%)

.PHONY: Makefile
Makefile:

# catch all rule for a make target that is not explicitly listed above,
# guess the subdir by assuming the first segment of the target is a brand.
# usb flat build subdir (for backward compatible mode)
%: _FIRSTWORD = $(firstword $(subst /, ,$@))
%: _BRAND = $(filter $(SUBDIRS:=/$(_FIRSTWORD)),\
	   $(foreach _SUBDIR,$(SUBDIRS),\
	   $(patsubst %.mk,%,$(wildcard $(_SUBDIR)/*.mk))))
%: _REMAIN = $(@:$(_FIRSTWORD)%=%)
%::FORCE
	$(eval REMAIN=$(_REMAIN))
	$(if $(_BRAND),\
	$(MAKE) SUBBLDDIR=. $(_BRAND)$(_REMAIN),\
	$(error No rule to make target `$@')\
	)

export REMAIN
.PHONY: FORCE
FORCE:
