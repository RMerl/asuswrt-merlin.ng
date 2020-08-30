# General-purpose GNU make include file
#
# Copyright (C) 2020, Broadcom. All Rights Reserved.
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
# <<Broadcom-WL-IPTag/Open:>>
#
# $Id: Makefile 506823 2014-10-07 12:59:32Z $

ifdef _WLAN_COMMON_MK
  $(if $D,$(info Info: Avoiding redundant include ($(MAKEFILE_LIST))))
else	# _WLAN_COMMON_MK
  _WLAN_COMMON_MK := 1
unexport _WLAN_COMMON_MK	# in case of make -e

################################################################
# Summary and Namespace Rules
################################################################
#   This is a special makefile fragment intended for common use.
# It defines macros which help implement the component model:
# http://confluence.broadcom.com/display/WLAN/Adding+a+New+Component+Using+The+WCC+Component+Model
# The important design principle is that it defines variables
# and functions only within a tightly controlled namespace.
# If a make include file was used to set rules, pattern rules,
# or well-known variables like CFLAGS, it could have unexpected
# effects on the including makefile with the result that people
# would either stop including it or stop improving it.
# Therefore, the only way to keep this a file which can be
# safely included by any GNU makefile and extended at will is
# to allow it only to set variables and only in its own namespace.
#   The namespace is "WLAN_CamelCase" for normal variables,
# "wlan_lowercase" for functions, and WLAN_UPPERCASE for boolean
# "constants" (these are all really just make variables; only the
# usage patterns differ).
#   Internal (logically file-scoped) variables are prefixed with "-"
# and have no other namespace restrictions.
#   Every variable defined here should match one of these patterns.

################################################################
# Enforce required conditions
################################################################

ifneq (,$(filter 3.7% 3.80,$(MAKE_VERSION)))
  $(error $(MAKE): Error: version $(MAKE_VERSION) too old, 3.81+ required)
endif

################################################################
# Store this makefile's path before MAKEFILE_LIST gets changed.
################################################################

WLAN_Common := $(lastword $(MAKEFILE_LIST))

################################################################
# Derive including makefile since it's a little tricky.
################################################################

WLAN_Makefile := $(abspath $(lastword $(filter-out $(lastword $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))

################################################################
# Host type determination
################################################################

_common-uname-s := $(shell uname -s)

# Typically this will not be tested explicitly; it's the default condition.
WLAN_HOST_TYPE := unix

ifneq (,$(filter Linux,$(_common-uname-s)))
  WLAN_LINUX_HOST := 1
else ifneq (,$(filter CYGWIN%,$(_common-uname-s)))
  WLAN_CYGWIN_HOST := 1
  WLAN_WINDOWS_HOST := 1
else ifneq (,$(filter Darwin,$(_common-uname-s)))
  WLAN_MACOS_HOST := 1
  WLAN_BSD_HOST := 1
else ifneq (,$(filter FreeBSD NetBSD,$(_common-uname-s)))
  WLAN_BSD_HOST := 1
else ifneq (,$(filter SunOS%,$(_common-uname-s)))
  WLAN_SOLARIS_HOST := 1
endif

################################################################
# Utility variables
################################################################

empty :=
space := $(empty) $(empty)
comma := ,

################################################################
# Utility functions
################################################################

# Provides enhanced-format messages from make logic.
wlan_die = $(error Error: $1)
wlan_warning = $(warning Warning: $1)
wlan_info = $(info Info: $1)

# Debug function to enable make verbosity.
wlan_dbg = $(if $D,$(call wlan_info,$1))

# Debug function to expose values of the listed variables.
wlan_dbgv = $(foreach _,$1,$(call wlan_dbg,$_=$($_)))

# Make-time assertion.
define wlan_assert
  ifeq (,$(findstring clean,$(MAKECMDGOALS)))
    $(if $1,,$(call wlan_die,$2))
  endif
endef

# Checks for the presence of an option in an option string
# like "aaa-bbb-ccc-ddd".
wlan_opt = $(if $(findstring -$1-,-$2-),1,0)

# The purpose of this macro is to avoid the need for full directory
# paths to be created via mkdir -p since mkdir -p has an inherent
# race in parallel use cases. Interestingly, though it exists to
# make -p unnecessary, it actually uses -p. Why is that? It's to
# take advantage of a useful side effect of -p which is that it
# doesn't complain if the directory already exists. In other words
# it creates directories one at a time to avoid creating its own
# race but still uses mkdir -p to protect itself against races with
# unrelated make processes.
# Usage: $(call wlan_target_needs_dir,<target>,<dir-path>)
define wlan_target_needs_dir
$(eval
$$1: | $$2
$$2: | $$(filter-out .,$$(patsubst %/,%,$$(dir $$2))); mkdir -p $$@
ifneq (,$$(findstring /,$$2))
  $$(call wlan_target_needs_dir,$$1,$$(patsubst %/,%,$$(dir $$2)))
endif
)
endef

# Compares two dotted numeric strings (e.g 2.3.16.1) for $1 >= $2
define wlan_version_ge
$(findstring TRUE,$(shell bash -c 'sort -cu -t. -k1,1nr -k2,2nr -k3,3nr -k4,4nr <(echo -e "$2\n$1") 2>&1 || echo TRUE'))
endef

# This is a useful macro to wrap around a compiler command line,
# e.g. "$(CC) $(call wlan_cc,$(CFLAGS))". It organizes flags in a
# readable way while taking care not to change any ordering
# which matters. It also provides a hook for externally
# imposed C flags which can be passed in from the top level.
# NOTE: this doesn't work with clang when using -isysroot etc
# because the "=" form e.g. -isysroot=<dir> is not supported.
define wlan_cc
$(filter-out -D% -I%,$1) $(sort $(filter -D%,$1)) $(filter -I%,$1) $(WLAN_EXTERNAL_CFLAGS)
endef

# Applies the standard cygpath translation for a path on Cygwin.
define wlan_cygpath
$(if $(WLAN_CYGWIN_HOST),$(shell cygpath -m $1),$1)
endef

# Change case without requiring a shell.
wlan_tolower = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,\
$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,\
$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,\
$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

wlan_toupper = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,\
$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,\
$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,\
$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))

# This macro derives the base (aka root) of the entire "checkout tree" as well as the
# current "build tree" and "component tree" (svn or git checkout). See
# http://confluence.broadcom.com/display/WLAN/DEPS-Based+Checkout+Tree+Terminology
# for definitions of these terms.
# It returns a list: [checkout-tree, build-tree, component-tree]
# Notes:
# 1. There may be no SCM metadata at all, particularly in the case of a source package
# delivered to a customer. This is where guessing (as described below) comes in.
# 2. This macro always trusts a .wlbase file if present. It looks next for a .gclient
# file, then sparsefile_url.py, then .git or .svn. If none of these are found we must
# guess, starting from the location of this file. If a ../../../main directory exists
# relative to here, set the base at ../../..; otherwise set it at ../.. from here.
# 3. We've observed a bug, or at least a surprising behavior, in emake which causes
# $(realpath ...) to fail. Thus a shell fallback is used in case of emake use.
define wlan_basedirs
$(eval\
  _refdir := $$(realpath $$(or $1,$$(dir $$(lastword $$(MAKEFILE_LIST)))))
  _refdir ?= $$(shell cd $$(or $1,$$(dir $$(lastword $$(MAKEFILE_LIST)))) && pwd -P)
  _parts := $$(strip $$(subst /,$$(space),$$(subst \,/,$$(_refdir))))
  _paths := /
  $$(foreach _i,$$(_parts),$$(eval _paths += $$(addsuffix /$$(_i),$$(lastword $$(_paths)))))
  _paths := $$(patsubst //%,/%,$$(wordlist 2,$$(words $$(_paths)),$$(_paths)))
  _wlb := $$(patsubst %/,%,$$(dir $$(lastword $$(wildcard $$(addsuffix /.wlbase,$$(_paths))))))
  _dep := $$(patsubst %/,%,$$(dir $$(lastword $$(wildcard $$(addsuffix /.gclient,$$(_paths))))))
  ifndef _dep
    _sprs := $$(subst /.svn/sparsefile_url.py,,$$(lastword $$(wildcard $$(addsuffix /.svn/sparsefile_url.py,$$(_paths)))))
    ifndef _sprs
      _guess := $$(strip $$(if $$(wildcard $$(dir $$(abspath $$(WLAN_Common)))../../../main),\
        $$(abspath $$(dir $$(WLAN_Common))../../..),\
        $$(abspath $$(dir $$(WLAN_Common))../..)))
    endif  # _sprs
  endif  # _dep
  _arg3 := $$(patsubst %/,%,$$(dir $$(lastword $$(wildcard $$(addsuffix /.svn,$$(_paths)) $$(addsuffix /.git,$$(_paths))))))
  _arg2 := $$(abspath $$(_refdir)/../..)
  _arg1 := $$(or $$(_wlb),$$(_dep),$$(_sprs),$$(_guess),$$(_arg3))
)$(strip $(_arg1) $(_arg2) $(_arg3))
endef

################################################################
# Standard make variables
################################################################

# The WLAN_CheckoutBaseA variable points to the root of the entire
# checkout while WLAN_TreeBaseA points to the root of the current
# build tree and WLAN_ComponentBaseA points to the root of the
# component containing the including Makefile. They might all have
# the same values but in other cases such as DHDAP routers a single
# DEPS checkout may contain multiple build trees (potentially from
# different branches) with each build tree composed of multiple components.
# These 'A' variables are absolute paths; each has an 'R' variant
# which is relative.
ifndef WLAN_TreeBaseA
  _basedata := $(call wlan_basedirs)
  WLAN_CheckoutBaseA := $(word 1,$(_basedata))
  WLAN_TreeBaseA := $(word 2,$(_basedata))
  WLAN_ComponentBaseA := $(word 3,$(_basedata))
  ifdef WLAN_CYGWIN_HOST
    WLAN_TreeBaseA := $(shell cygpath -m -a $(WLAN_TreeBaseA))
  endif
endif

# Pick up the "relpath" make function from the same dir as this makefile.
include $(dir $(lastword $(MAKEFILE_LIST)))RelPath.mk

# The *R variables are relativized versions of the *A variables.
WLAN_CheckoutBaseR = $(call relpath,$(WLAN_CheckoutBaseA))
WLAN_TreeBaseR = $(call relpath,$(WLAN_TreeBaseA))
WLAN_ComponentBaseR = $(call relpath,$(WLAN_ComponentBaseA))

# For compatibility, due to the prevalence of $(SRCBASE)
WLAN_SrcBaseA := $(WLAN_TreeBaseA)/src
WLAN_SrcBaseR  = $(patsubst %/,%,$(dir $(WLAN_TreeBaseR)))

# Show makefile list before we start including things.
$(call wlan_dbgv, CURDIR MAKEFILE_LIST)

################################################################
# Pick up the "universal settings file" containing
# the list of all available software components.
################################################################

include $(dir $(lastword $(MAKEFILE_LIST)))../tools/release/WLAN.usf

################################################################
# Calculate paths to requested components.
################################################################

# This function uses pattern matching to pull component paths from
# their basenames (e.g. src/wl/xyz => xyz).
# It also strips out component paths which don't currently exist.
# This may be required due to our "sparse tree" build styles
# and the fact that linux mkdep throws an error when a directory
# specified with -I doesn't exist.
define _common-component-names-to-rel-paths
$(strip \
  $(patsubst $(WLAN_TreeBaseA)/%,%,$(wildcard $(addprefix $(WLAN_TreeBaseA)/,\
  $(sort $(foreach name,$1,$(filter %/$(name),$(WLAN_COMPONENT_PATHS))))))))
endef

# These are modeled as components but they are needed globally so we
# make them defaults. For cleanliness, disallow their explicit use.
WLAN_ImplicitComponents := proto wlioctl
-wlan_implicit_overlap := $(filter $(WLAN_ImplicitComponents),$(WLAN_ComponentsInUse))
$(if $(-wlan_implicit_overlap),$(call wlan_die,implied: "$(-wlan_implicit_overlap)"))
WLAN_ComponentsInUse += $(WLAN_ImplicitComponents)

# Sort/unique the component request and turn it into a simple assignment.
# For backward compatibility we accept certain obsolete component names
# and convert them to their updated names.
WLAN_ComponentsInUse := $(sort $(subst phymods,phy,$(WLAN_ComponentsInUse)))

# Translate the resulting list of components names to paths.
WLAN_ComponentPathsInUse := $(call _common-component-names-to-rel-paths,$(WLAN_ComponentsInUse))

#   Loop through all components in use. If a cfg-xyz.mk file exists at the base of
# component xyz's subtree, include it and use its contents to modify the list
# of include and src dirs. Otherwise, use the defaults for that component.
#   While our architecture and policy normally disallow -I paths pointing into private
# (typically "src") include areas, some of our build models require "flattening" source
# files into a single directory which in turn may require private areas to be found via
# explicit -I flags vs the default compiler behavior of searching relative to the .c
# file. To support this we allow an override list called WLAN_ExposePrivateHeaders.
#   Also, generate a WLAN_ComponentBaseDir_xyz variable for each component "xyz".
WLAN_ComponentIncPathsInUse :=
WLAN_ComponentSrcPathsInUse :=
$(foreach _path,$(WLAN_ComponentPathsInUse), \
  $(if $(wildcard $(WLAN_TreeBaseA)/$(_path)/cfg-$(notdir $(_path)).mk),$(eval include $(WLAN_TreeBaseA)/$(_path)/cfg-$(notdir $(_path)).mk)) \
  $(eval $(notdir $(_path))_IncDirs ?= include) \
  $(eval $(notdir $(_path))_SrcDirs ?= src) \
  $(eval WLAN_ComponentIncPathsInUse += $(addprefix $(_path)/,$($(notdir $(_path))_IncDirs))) \
  $(if $(filter $(notdir $(_path)),$(WLAN_ExposePrivateHeaders)), \
    $(eval WLAN_ComponentIncPathsInUse += $(addprefix $(_path)/,$($(notdir $(_path))_SrcDirs)))) \
  $(eval WLAN_ComponentSrcPathsInUse += $(addprefix $(_path)/,$($(notdir $(_path))_SrcDirs))) \
  $(eval WLAN_ComponentBaseDir_$$(notdir $(_path)) := $$(WLAN_TreeBaseA)/$(_path)) \
)

# Global include/source paths.
# IMPORTANT: these represent backward compatibility for traditional, nonstandard locations.
# New components should NOT be added here. In particular, the "src" areas of components
# should NOT be added to the WLAN_SrcIncDirs list. A component's private header area must
# be _private_, meaning it does not appear on -I paths and is not visible to other components.
# Private headers should be used only by their own component's .c files, and it's the job of
# the .c files to find private headers via e.g. 'include "../src/pvt_hdr.h"'.
WLAN_StdSrcDirs = src/shared src/wl/sys

WLAN_StdIncDirs = src/include components/shared

WLAN_SrcIncDirs = src/shared src/wl/sys

WLAN_StdSrcDirsR	 = $(addprefix $(WLAN_TreeBaseR)/,$(WLAN_StdSrcDirs))
WLAN_StdIncDirsR	 = $(addprefix $(WLAN_TreeBaseR)/,$(WLAN_StdIncDirs))
WLAN_SrcIncDirsR	 = $(addprefix $(WLAN_TreeBaseR)/,$(WLAN_SrcIncDirs))
WLAN_StdIncPathR	 = $(addprefix -I,$(wildcard $(WLAN_StdIncDirsR)))
WLAN_IncDirsR		 = $(WLAN_StdIncDirsR) $(WLAN_SrcIncDirsR)
WLAN_IncPathR		 = $(addprefix -I,$(wildcard $(WLAN_IncDirsR)))

WLAN_StdSrcDirsA	 = $(addprefix $(WLAN_TreeBaseA)/,$(WLAN_StdSrcDirs))
WLAN_StdIncDirsA	 = $(addprefix $(WLAN_TreeBaseA)/,$(WLAN_StdIncDirs))
WLAN_SrcIncDirsA	 = $(addprefix $(WLAN_TreeBaseA)/,$(WLAN_SrcIncDirs))
WLAN_StdIncPathA	 = $(addprefix -I,$(wildcard $(WLAN_StdIncDirsA)))
WLAN_IncDirsA		 = $(WLAN_StdIncDirsA) $(WLAN_SrcIncDirsA)
WLAN_IncPathA		 = $(addprefix -I,$(wildcard $(WLAN_IncDirsA)))

# Public convenience macros based on WLAN_ComponentPathsInUse list.
WLAN_ComponentSrcDirsR	 = $(addprefix $(WLAN_TreeBaseR)/,$(WLAN_ComponentSrcPathsInUse))
WLAN_ComponentIncDirsR	 = $(addprefix $(WLAN_TreeBaseR)/,$(WLAN_ComponentIncPathsInUse))
WLAN_ComponentIncPathR	 = $(addprefix -I,$(wildcard $(WLAN_ComponentIncDirsR)))

WLAN_ComponentSrcDirsA	 = $(addprefix $(WLAN_TreeBaseA)/,$(WLAN_ComponentSrcPathsInUse))
WLAN_ComponentIncDirsA	 = $(addprefix $(WLAN_TreeBaseA)/,$(WLAN_ComponentIncPathsInUse))
WLAN_ComponentIncPathA	 = $(addprefix -I,$(wildcard $(WLAN_ComponentIncDirsA)))

# These names are deprecated - please use the A or R variants (R preferred).
WLAN_ComponentSrcDirs	 = $(WLAN_ComponentSrcDirsA)
WLAN_ComponentIncDirs	 = $(WLAN_ComponentIncDirsA)
WLAN_ComponentIncPath	 = $(WLAN_ComponentIncPathA)

# Dump a representative sample of derived variables in debug mode.
$(call wlan_dbgv, WLAN_TreeBaseA WLAN_TreeBaseR WLAN_SrcBaseA WLAN_SrcBaseR \
    WLAN_ComponentPathsInUse WLAN_ComponentIncPath WLAN_ComponentIncPathR \
    WLAN_ComponentSrcDirs WLAN_ComponentSrcDirsR)

# Special case for Windows to reflect CL in the build log if used.
ifdef WLAN_WINDOWS_HOST
  ifdef CL
    $(info Info: CL=$(CL))
  endif
endif

# A big hammer for debugging each shell invocation.
# Warning: this can get lost if a sub-makefile sets SHELL explicitly, and
# in that case the parent should add $(WLAN_ShellDebugSHELL) to the call.
ifeq ($D,2)
  WLAN_ShellDebug := 1
endif
ifdef WLAN_ShellDebug
  ORIG_SHELL := $(SHELL)
  SHELL = $(strip $(warning Shell: ORIG_SHELL=$(ORIG_SHELL) PATH=$(PATH))$(ORIG_SHELL)) -x
  WLAN_ShellDebugSHELL := SHELL='$$(warning Shell: ORIG_SHELL=$$(ORIG_SHELL) PATH=$$(PATH))$(ORIG_SHELL) -x'
endif

# Variables of general utility.
WLAN_Perl := perl
WLAN_Python := python
WLAN_WINPFX ?= Z:

# These macros are used to stash an extra copy of generated source files,
# such that when a source release is made those files can be reconstituted
# from the stash during builds. Required if the generating tools or inputs
# are not shipped.
define wlan_copy_to_gen
  $(if $(filter $(CLM_TYPE),router),\
    $(if $(WLAN_COPY_GEN),&& mkdir -p $(subst $(abspath $2/..),$(abspath $2/$(WLAN_GEN_BASEDIR)),$(dir $(abspath $1))) && \
      cp -pv $1 $(subst $(abspath $2/..),$(abspath $2/$(WLAN_GEN_BASEDIR)),$(abspath $1).GEN)),\
    $(if $(WLAN_COPY_GEN),&& mkdir -p $(subst $(abspath $2),$(abspath $2/$(WLAN_GEN_BASEDIR)),$(dir $(abspath $1))) && \
      cp -pv $1 $(subst $(abspath $2),$(abspath $2/$(WLAN_GEN_BASEDIR)),$(abspath $1).GEN)))
endef

############################################################################
# CLM function: generates a rule to run ClmCompiler if the XML exists and
# to restore saved ClmCompiler output from a "generated" area if not.
# USAGE: $(call WLAN_GenClmCompilerRule,target-dir,src-base[,flags])
#     $1 is the directory where the generated .c file goes (it must exist)
#     $2 gives the base of the WL driver tree (SRCBASE).
#     $3 is an optional set of flags to pass to ClmCompiler.
#   The ClmCompiler options are concatenated from three places.
#     1) The optional $3 argument
#     2) The make variable CLMCOMPEXTFLAGS is by default --obfuscate
#        but can be overridden AFTER the WLAN_GenClmCompilerRule usage
#     3) --config $(CLM_TYPE) if ($CLM_TYPE) is defined.  CLM_TYPE
#        represent the basename, without the .clm, of a ClmCompiler
#        config that is expected to be in src/wl/clm/types directory.
#        If $(CLM_TYPE) is defined, a dependency to that .clm will
#        be added.
#   The created ClmCompiler rule will use/depend on a xml file.  This will be
# src/wl/clm/private/wlc_clm_data_$CLM_TYPE).xml if it exists else
# components/clm/clm-private/wlc_clm_data.xml will be used.  This allows you
# to use a xml file that corresponds to the .clm being used.
#   This macro uses GNU make's eval function to generate an
# explicit rule to generate a particular CLM data file each time
# it's called. Make variables which should be evaluated during eval
# processing get one $, those which must defer till "runtime" get $$.
#   The "clm_compiled" phony target is provided for makefiles which need
# to defer some other processing until CLM data is ready, and "clm_clean"
# and "CLM_DATA_FILES" make it easier for internal client makefiles to
# clean up CLM data (externally, this is treated as source and not removed).
#   The outermost conditional allows this rule to become a no-op
# in external settings where there is no XML input file while allowing
# it to turn back on automatically if an XML file is provided.
#   A vpath is used to find the XML input because this file is not allowed
# to be present in external builds. Use of vpath allows it to be "poached"
# from the internal build as necessary.
#   Note: the .c file is listed with and without a path due to the way the
# Linux kernel Makefiles generate .depend data.
CLMCOMPEXTFLAGS := --obfuscate
define -GenClmCompilerRule
.PHONY: clm_compiled clm_clean

# The "mkdir -p foo 2>/dev/null || test -d foo || mkdir -p foo" logic below is
# intended to defeat potential race conditions. If the first mkdir fails we
# check whether we just lost the race to create it; if it still doesn't exist
# there must be some other problem so we run mkdir again to generate
# the appropriate error message and die.
$1: ; mkdir -p $$@ 2>/dev/null || test -d $$@ || mkdir -p $$@
vpath wlc_clm_data.c $1 $$(abspath $1)
ifneq (,$(or $(wildcard $(addsuffix /../components/clm-private/wlc_clm_data.xml,$2 $2/../../src $2/../../../src)),$(wildcard $(addsuffix /wl/clm/private/wlc_clm_data_$(CLM_TYPE).xml,$2 $2/../../src $2/../../../src))))
  vpath wlc_clm_data.xml $(wildcard $(addsuffix /../components/clm-private,$2 $2/../../src $2/../../../src))
  vpath wlc_clm_data_$(CLM_TYPE).xml $(addsuffix /wl/clm/private,$2 $2/../../src $2/../../../src)
  vpath %.clm $(addsuffix /wl/clm/types,$2 $2/../../src $2/../../../src)
  $1/wlc_clm_data.c: \
      $$(if $$(wildcard $2/wl/clm/private/wlc_clm_data_$$(CLM_TYPE).xml),$2/wl/clm/private/wlc_clm_data_$$(CLM_TYPE).xml,wlc_clm_data.xml) \
      $2/../components/clm-api/include/wlc_clm_data.h \
      $$(wildcard $2/../components/clm-bin/ClmCompiler.py) \
      $$(if $$(CLM_TYPE),$$(CLM_TYPE).clm) \
      | $1; \
    @echo wlc_clm_data.c rule -- $$@: $$+ ;\
    $$(strip $$(firstword $$(wildcard $$(addsuffix /components/clm-bin/ClmCompiler.py, $2/.. $2/../.. $2/../../..))) \
      --clmapi_include_dir $$(firstword $$(wildcard $$(addsuffix components/clm-api/include, $2/../ $2/../../ $2/../../../ ))) \
      --print_options --bcmwifi_include_dir $$(firstword $$(wildcard $$(addsuffix shared/bcmwifi/include, $2/ $2/../../src/ $2/../../../src/ ))) \
      $$(if $$(CLM_TYPE),--config_file $$(lastword $$^)) $3 $(CLMCOMPEXTFLAGS) $$< $$@ $$(call wlan_copy_to_gen,$$@,$2))
else
  vpath %.GEN $(subst $(abspath $2),$(abspath $2/$(WLAN_GEN_BASEDIR)),$(abspath $1)) $(sort $(patsubst %/,%,$(dir $(wildcard $(subst $(abspath $2),$(abspath $2/$(WLAN_GEN_BASEDIR)),$(dir $(abspath $1)))*/*.GEN))))
  $1/%: %.GEN ; cp -pv $$< $$@
endif
  clm_compiled: $1/wlc_clm_data.c
  clm_clean:: ; $$(RM) $1/wlc_clm_data.c
  CLM_DATA_FILES += $1/wlc_clm_data.c
endef

# One helpful way of debugging this is to add "$(warning $(-GenClmCompilerRule))"
# in a continuation line below.
define WLAN_GenClmCompilerRule
  $(eval $(-GenClmCompilerRule))
endef

################################################################
# Generates rule that generates wlc_sar_tbl.c file
# USAGE: $(call WLAN_GenSarTblRule,target-dir,src-base,[sar-files])
# Arguments:
# target-dir -- Directory where generated wlc_sar_tbl.c shall be put.
# src-base   -- Top level 'src' directory. Strange manipulations with
#               it within macro expansion are due to some strange
#               peculiarities of Linux driver builds
# sar_files  -- Space-separated list of base names of .csv files that
#               contain SAR entries that shall be put to generated SAR
#               table. Empty means use all files in src/wl/sar directory
define WLAN_GenSarTblRule
$(eval\
$1/wlc_sar_tbl.c: ; $$(firstword $$(wildcard \
    $$(addsuffix /components/tools/bin/sar_conv.py, $2/.. $2/../.. $2/../../..))) create_c \
    --driver_dir $$(addsuffix /../.., $$(firstword $$(wildcard \
    $$(addsuffix /include, $2 $2/../../src $2/../../../src)))) \
    $$(addprefix $$(firstword $$(wildcard \
    $$(addsuffix /wl/sar, $2 $2/../../src $2/../../../src)))/, \
    $$(if $3,$3,*.csv)) $$@
)
endef

################################################################

endif	# _WLAN_COMMON_MK
