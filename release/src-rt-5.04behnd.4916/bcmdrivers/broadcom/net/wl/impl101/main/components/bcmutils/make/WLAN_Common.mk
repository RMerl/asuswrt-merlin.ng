# General-purpose GNU make include file
#
# Copyright (C) 2023, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
# <<Broadcom-WL-IPTag/Dual:>>

ifndef _WLAN_COMMON_MK
_WLAN_COMMON_MK := 1
unexport _WLAN_COMMON_MK        # in case of make -e

################################################################
# Summary and Namespace Rules
################################################################
#   This is a special makefile fragment intended for common use.
# It defines macros which help implement the component model:
# https://confluence.broadcom.net/display/WLAN/Adding+a+New+Component+Using+The+WCC+Component+Model
# The important design principle is that it defines variables
# and macros only within a tightly controlled namespace.
# If a make include file was used to define rules, pattern rules,
# vpaths, or well-known variables like CFLAGS it could have unexpected
# effects on the including makefile with the result that people
# would either stop including it or stop improving it.
# Therefore, in order to ensure this file can be safely included
# by any other makefile at any time, it may only define variables
# and only within a clearly defined namespace.
#   The preferred namespace is "WLAN_CamelCase" for normal variables,
# ".lowercase" for macros, and WLAN_UPPERCASE for boolean
# "constants" (these are all really just make variables; only the
# usage patterns differ).
#   Internal (logically file-scoped) variables are prefixed with
# "-" or "_" and have no other namespace restrictions.
#   Every variable defined here should match one of these patterns.

################################################################
# Store this makefile's path before MAKEFILE_LIST gets changed.
################################################################

WLAN_Common := $(lastword $(MAKEFILE_LIST))

################################################################
# Derive the including makefile since it's a little tricky.
################################################################

WLAN_Makefile := $(abspath $(lastword $(filter-out \
  $(WLAN_Common),$(MAKEFILE_LIST))))
WLAN_MakefileDir := $(patsubst %/,%,$(dir $(WLAN_Makefile)))

################################################################
# Reserve "all" as the default target.
################################################################

.DEFAULT_GOAL = all

################################################################
# Pick up the even-lower-level shared global utilities file
# unless it was explicitly included earlier.
################################################################

ifeq ($(filter utils.mk,$(notdir $(MAKEFILE_LIST))),)
  include $(abspath $(dir $(WLAN_Common))utils.mk)
endif # utils.mk

################################################################
# Standard make variables
################################################################

# The 'A' variables are absolute paths and each has a relative 'R' variant.
# WLAN_TreeBaseA points to the root of the current build tree.

# A custom value of WLAN_TreeBaseA may be set before including this file.
# This may be helpful in pre-mogrification scenarios, for instance, where we
# want to mogrify early and do all remaining work in the mogrified tree.
WLAN_TreeBaseA := $(abspath \
  $(or $(WLAN_TreeBaseA),$(dir $(WLAN_Common))../../..))

# The *R variables are relativized versions of the *A variables.
WLAN_TreeBaseR = $(call .relpath,$(WLAN_TreeBaseA))

# For backward compatibility. Deprecated, don't use.
WLAN_CheckoutBaseA := $(WLAN_TreeBaseA)
WLAN_CheckoutBaseR = $(WLAN_TreeBaseR)

# Relative path from tree base to current working directory.
WLAN_RelCwd := $(patsubst /%,%,$(subst $(WLAN_TreeBaseA),,$(CURDIR)))

################################################################
# Define the current list of components. This list is used in
# makefiles to set vpath search and include (-I) paths.
# NOTE: when adding new components you may also want to add
# them to the cstyle filelist.
################################################################

define WLAN_Component_Paths
  components/apf
  components/arm_debug
  components/aslr_code
  components/bcmcrypto
  components/bcmhal
  components/bcmlinux
  components/bcmotp
  components/bcmrng
  components/bcmsm
  components/bcmtrace
  components/bcmutils
  components/bcmwifi
  components/bootrom
  components/chre
  components/clm-api
  components/coex
  components/dnglfrmwk
  components/dnglrte
  components/dngltest
  components/edv
  components/etd
  components/ftm
  components/fw_debug
  components/fwpreinit
  components/fwsign
  components/host_chiputil
  components/keys
  components/math
  components/mesh
  components/msch
  components/nan
  components/nanho
  components/ndis
  components/osl
  components/owe
  components/pasn
  components/phy
  components/ppr
  components/proto
  components/radio_data
  components/sae
  components/sdtc
  components/sib
  components/spmi
  components/swpaging
  components/wlioctl
  src/dongle
  src/pciedev
  src/rte
  src/sdpcmdev
  src/shared
  src/wl/ate
  src/wl/chctx
  src/wl/coex_cpu
  src/wl/dump
  src/wl/encode
  src/wl/fmf
  src/wl/gas
  src/wl/iocv
  src/wl/keymgmt
  src/wl/mbo_oce
  src/wl/natoe
  src/wl/olpc
  src/wl/proxd
  src/wl/randmac
  src/wl/rel_mcast
endef

$(call .require_sorted,WLAN_Component_Paths)

# Ensure sorted, strip newlines, convert to simple expansion.
WLAN_Component_Paths := $(sort $(WLAN_Component_Paths))

# Usage: "make -fWLAN_Common.mk wlan_component_paths".
.PHONY: wlan_component_paths
wlan_component_paths:
	@:$(info $(WLAN_Component_Paths))

################################################################
# Calculate paths to requested components.
################################################################

# This macro uses pattern matching to pull component paths from
# their basenames (e.g. src/wl/xyz => xyz).
# It also strips out component paths which don't currently exist.
# This may be required due to our "DEPS" build styles and
# the fact that linux mkdep throws an error when a directory
# specified with -I doesn't exist.
define _common-component-names-to-rel-paths
$(strip \
  $(patsubst $(WLAN_TreeBaseA)/%,%,$(wildcard $(addprefix $(WLAN_TreeBaseA)/, \
  $(sort $(foreach name,$1,$(filter %/$(name),$(WLAN_Component_Paths))))))))
endef

# These are modeled as components but they are needed globally so we
# make them defaults. For cleanliness, disallow their explicit use.
WLAN_ImplicitComponents := bcmhal bcmotp bcmutils osl proto shared wlioctl
-wlan_implicit_overlap := $(filter \
  $(WLAN_ImplicitComponents),$(WLAN_ComponentsInUse))
$(if $(-wlan_implicit_overlap),$(call \
  .die,implicit: "$(-wlan_implicit_overlap)"))
WLAN_ComponentsInUse += $(WLAN_ImplicitComponents)

# Sort/unique the component request and turn it into a simple assignment.
WLAN_ComponentsInUse := $(sort $(WLAN_ComponentsInUse))

# Translate the resulting list of components names to paths.
WLAN_ComponentPathsInUse := $(call \
  _common-component-names-to-rel-paths,$(WLAN_ComponentsInUse))

############################################################################
#   Loop through all components in use. If a cfg-xyz.mk file exists at the base
# of component xyz's subtree, include it and use its contents to modify the
# list of include and src dirs. Otherwise use the defaults for that component.
#   While our architecture and policy normally disallow -I paths pointing
# into private (typically "src") include areas, some of our build models
# require "flattening" source files into a single directory which in turn
# may require private areas to be found via explicit -I flags vs the default
# compiler behavior of searching relative to the .c file. To support this
# we allow an override list called WLAN_ExposePrivateHeaders.
define .wlan_parse_component_configs

$(call .evalx,WLAN_ComponentIncPathsInUse :=)
$(call .evalx,WLAN_ComponentSrcPathsInUse :=)

$(foreach p,$(WLAN_ComponentPathsInUse), \
  $(if $(wildcard $(WLAN_TreeBaseA)/$p/cfg-$(notdir $p).mk), \
    $(call .include,$(WLAN_TreeBaseA)/$p/cfg-$(notdir $p).mk)) \
  $(call .evalx,$(notdir $p)_IncDirs ?= include) \
  $(call .evalx,$(notdir $p)_SrcDirs ?= src) \
  $(call .evalx,WLAN_ComponentIncPathsInUse += \
    $(addprefix $p/,$($(notdir $p)_IncDirs))) \
  $(if $(filter $(notdir $p),$(WLAN_ExposePrivateHeaders)), \
    $(call .evalx,WLAN_ComponentIncPathsInUse += \
      $(addprefix $p/,$($(notdir $p)_SrcDirs)))) \
  $(call .evalx,WLAN_ComponentSrcPathsInUse += \
    $(addprefix $p/,$($(notdir $p)_SrcDirs))) \
)

# Sorting has two benefits: it uniqifies the list, which may have
# gotten some double entries earlier, and it makes for prettier and
# more predictable log output.
$(call .evalx,WLFILES_SRC := $$(sort $$(WLFILES_SRC)))

endef

# Invoke this right away to incorporate all relevant component source files.
# It may be called again later if feature flags are set by subsequent
# make/config files.
$(call .wlan_parse_component_configs)

############################################################################

# Global include/source paths.
# IMPORTANT: this represents backward compatibility for traditional, nonstandard
# locations. New components should NOT be added here. In particular, the "src"
# areas of components should NOT be added to the WLAN_SrcIncDirs list.
# A component's private header area must be _private_, meaning it does not
# appear on -I paths and is not visible to other components.
# Private headers should be used only by their own component's .c files
# and it's the job of the .c files to find private headers via e.g.
# 'include "../src/pvt_hdr.h"'. See discussion of WLAN_ExposePrivateHeaders
# above for more on private header policy.
WLAN_StdSrcDirs = src/wl/sys

WLAN_StdIncDirs =

WLAN_SrcIncDirs = src/wl/sys

WLAN_StdSrcDirsR         = $(addprefix $(WLAN_TreeBaseR)/,$(WLAN_StdSrcDirs))
WLAN_StdIncDirsR         = $(addprefix $(WLAN_TreeBaseR)/,$(WLAN_StdIncDirs))
WLAN_SrcIncDirsR         = $(addprefix $(WLAN_TreeBaseR)/,$(WLAN_SrcIncDirs))
WLAN_StdIncPathR         = $(addprefix -I,$(wildcard $(WLAN_StdIncDirsR)))
WLAN_IncDirsR            = $(WLAN_StdIncDirsR) $(WLAN_SrcIncDirsR)
WLAN_IncPathR            = $(addprefix -I,$(wildcard $(WLAN_IncDirsR)))

WLAN_StdSrcDirsA         = $(addprefix $(WLAN_TreeBaseA)/,$(WLAN_StdSrcDirs))
WLAN_StdIncDirsA         = $(addprefix $(WLAN_TreeBaseA)/,$(WLAN_StdIncDirs))
WLAN_SrcIncDirsA         = $(addprefix $(WLAN_TreeBaseA)/,$(WLAN_SrcIncDirs))
WLAN_StdIncPathA         = $(addprefix -I,$(wildcard $(WLAN_StdIncDirsA)))
WLAN_IncDirsA            = $(WLAN_StdIncDirsA) $(WLAN_SrcIncDirsA)
WLAN_IncPathA            = $(addprefix -I,$(wildcard $(WLAN_IncDirsA)))

# Public convenience variables based on WLAN_ComponentPathsInUse list.
WLAN_ComponentSrcDirsR   = $(addprefix \
  $(WLAN_TreeBaseR)/,$(WLAN_ComponentSrcPathsInUse))
WLAN_ComponentIncDirsR   = $(addprefix \
  $(WLAN_TreeBaseR)/,$(WLAN_ComponentIncPathsInUse))
WLAN_ComponentIncPathR   = $(addprefix \
  -I,$(wildcard $(WLAN_ComponentIncDirsR)))

WLAN_ComponentSrcDirsA   = $(addprefix \
  $(WLAN_TreeBaseA)/,$(WLAN_ComponentSrcPathsInUse))
WLAN_ComponentIncDirsA   = $(addprefix \
  $(WLAN_TreeBaseA)/,$(WLAN_ComponentIncPathsInUse))
WLAN_ComponentIncPathA   = $(addprefix \
  -I,$(wildcard $(WLAN_ComponentIncDirsA)))

################################################################

# This is here to (a) standardize things and (b) prevent
# spurious "ignoring old recipe" warnings. If each makefile
# defines its own help target then as they include each other
# they may step on each other's toes, so we declare it phony
# here once and including makefiles need only define a usage()
# variable. Variable redefinitions do not trigger warnings and
# they can even extend each other.

define usage ?=
Unfortunately, no one has written a help message for this makefile!
endef

.PHONY: help
help:
	@:$(info $(usage))

################################################################

# Usage: "make -fWLAN_Common.mk wlan_common_test".
.PHONY: wlan_common_test
wlan_common_test:
	@:$(info == THIS FILE ==)
	$(info WLAN_Common=$(WLAN_Common))
	$(info )
	$(info == BASE OF ENTIRE CHECKOUT TREE ==)
	$(info WLAN_TreeBaseR=$(WLAN_TreeBaseR))
	$(info WLAN_TreeBaseA=$(WLAN_TreeBaseA))
	$(info )
	$(info == RELATIVE PATH TO CURRENT DIRECTORY ==)
	$(info WLAN_RelCwd=$(WLAN_RelCwd))

################################################################

endif   # _WLAN_COMMON_MK

# This comment block must stay at the bottom of the file.
# Local Variables:
# mode: GNUmakefile
# fill-column: 80
# End:
#
# vim: filetype=make sw=2 tw=80 cc=+1 noet
