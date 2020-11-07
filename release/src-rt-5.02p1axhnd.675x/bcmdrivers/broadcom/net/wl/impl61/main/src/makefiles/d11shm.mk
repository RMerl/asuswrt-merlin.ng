#
# d11shm.mk  make file - Geneates d11 shm files
# <<Broadcom-WL-IPTag/Proprietary:>>
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
# $Id: d11shm.mk $

# By default trunk ucode is used. This can be overridden by defining UCODE_RAM/UCODE_ROM in the config files.

D11SHM_SRCBASE ?= $(SRCBASE)
D11SHM_WLCONF_PATH ?= .

ifdef BCMULP
D11SHM_ULP ?= 1
else
ifdef WOWL
D11SHM_WOWL ?= 1
endif
endif

$(info WL_EAP_UCODE: $(WL_EAP_UCODE))
ifdef WL_EAP_UCODE
D11SHM_EAP ?= 1
endif

ifeq ($(UCODE_IN_ROM),1)
ifndef UCODE_ROM
$(error UCODE_ROM is not defined in config file!)
endif
endif

ifeq ($(UCODE_IN_ROM),1)
	D11SHMDEFS_WOWL = $(D11SHM_SRCBASE)/../components/ucode/dot11_releases/trunk/$(UCODE_ROM)/rom
	ifeq ($(ULP_DS1ROM_DS0RAM),1)
		ifdef UCODE_RAM
			D11SHMDEFS = $(D11SHM_SRCBASE)/../components/ucode/dot11_releases/trunk/$(UCODE_RAM)/ram
		endif
	else
		D11SHMDEFS = $(D11SHM_SRCBASE)/../components/ucode/dot11_releases/trunk/$(UCODE_ROM)/rom
	endif
else
	ifdef UCODE_RAM
		D11SHMDEFS = $(D11SHM_SRCBASE)/../components/ucode/dot11_releases/trunk/$(UCODE_RAM)/ram
		D11SHMDEFS_WOWL = $(D11SHM_SRCBASE)/../components/ucode/dot11_releases/trunk/$(UCODE_RAM)/ram
	endif
endif

D11SHMDEFS ?= $(D11SHM_SRCBASE)/wl/sys
D11SHMDEFS_WOWL ?= $(D11SHM_SRCBASE)/wl/sys

D11SHM_TOOLS = $(D11SHM_SRCBASE)/../components/shared/d11shm
D11SHM_TPL = $(D11SHM_SRCBASE)/wl/sys/d11shm.tpl

D11SHM_DEPS = $(D11SHM_TPL)
D11SHM_DEPS += $(D11SHM_SRCBASE)/makefiles/d11shm.mk
D11SHM_DEPS += $(D11SHM_TOOLS)/d11shm_rev.c
D11SHM_DEPS += $(D11SHM_TOOLS)/d11shm.pl
D11SHM_DEPS += $(D11SHM_TOOLS)/d11shm_c.pl
D11SHM_DEPS += $(D11SHM_TOOLS)/d11shm_func.pl
D11SHM_DEPS += $(D11SHM_TOOLS)/d11shm_offsets.pl
D11SHM_DEPS += $(D11SHMDEFS)/d11ucode_shmdefs_std.h
ifdef D11SHM_EAP
D11SHM_DEPS += $(D11SHMDEFS)/d11ucode_shmdefs_eap.h
endif
ifdef D11SHM_ULP
D11SHM_DEPS += $(D11SHMDEFS_WOWL)/d11ucode_shmdefs_ulp.h
endif
ifdef D11SHM_WOWL
D11SHM_DEPS += $(D11SHMDEFS_WOWL)/d11ucode_shmdefs_wowl.h
endif

D11SHM_IFLAGS ?= -I.
D11SHM_CFLAGS ?= -I.
D11SHM_IFLAGS += -I$(D11SHM_SRCBASE)/include
D11SHM_CFLAGS += -I$(D11SHM_SRCBASE)/include

D11SHM_TEMPDIR ?= .
D11SHM_NAMED_INIT ?= 1

# Arguments for the d11shm perl scripts

D11SHM_HEADER = $(D11SHM_TEMPDIR)/d11shm.h
D11SHM_FUNC_ARGS = --ucode_type=ucode_$(1)\
			--i_d11shm_func_partial=$(D11SHM_TEMPDIR)/d11shm_func_ucode_$(1)_partial.c \
			--o_d11shm_main_functions=$(D11SHM_TEMPDIR)/d11shm_main_functions.c \
			--o_d11shm_func_decl=$(D11SHM_TEMPDIR)/d11shm_func_decl.h

D11SHM_C_ARGS = --ucode_type=ucode_$(1) --d11rev=$$D11REV \
			--i_d11shm_partial=$(D11SHM_TEMPDIR)/d11shm_partial.c \
			--o_d11shm_c=$(D11SHM_TEMPDIR)/d11shm.c

D11SHM_ARGS = --ucode_type=ucode_$(1) --d11rev=$$D11REV --named_init=$(D11SHM_NAMED_INIT) \
			--i_d11shm_template=$(D11SHM_TPL) \
			--o_d11shm_defaults=$(D11SHM_TEMPDIR)/d11shm_defaults.h \
			--o_d11shm_structs_decl=$(D11SHM_TEMPDIR)/d11shm_structs_decl.h \
			--o_d11shm_shmdefs_t=$(D11SHM_TEMPDIR)/d11shm_shmdefs_t.h \
			--o_d11shm_structs_inits=$(D11SHM_TEMPDIR)/d11shm_structs_inits.c \
			--o_d11shm_main_structs=$(D11SHM_TEMPDIR)/d11shm_main_structs.c \
			--o_d11shm_declarations=$(D11SHM_TEMPDIR)/d11shm_declarations.h \
			--o_d11shm_hdr=$(D11SHM_HEADER) \
			--o_d11shm_func_partial=$(D11SHM_TEMPDIR)/d11shm_func_ucode_$(1)_partial.c

D11SHM_PPFLAGS = $(D11SHM_CFLAGS) -DD11_REV=$$D11REV -P \
			-include $(D11SHMDEFS)/d11ucode_shmdefs_$(1).h

D11SHM_PPFLAGS_WIN = $(D11SHM_CFLAGS) -I. -I$(D11SHM_TEMPDIR) -DD11_REV=$$D11REV \
			/FI $(D11SHMDEFS)/d11ucode_shmdefs_$(1).h

ifeq ($(D11SHM_WIN), 1)
SUBSYSTEM = windows
REQUIRE_MSDEV = 1
REQUIRE_WDM7600 = 1
REQUIRE_SDK60 = 1
include $(D11SHM_SRCBASE)/makefiles/env.mk
WIN_LDFLAGS = $(foreach p,$(LIBVPATH.W),"/LIBPATH:$p")
WIN_INCLFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSDEV.INCLUDE)))
D11SHM_PPCMD = cl -nologo $(D11SHM_PPFLAGS_WIN) /EP $(D11SHM_TEMPDIR)/d11shm_structs_inits.c > $(D11SHM_TEMPDIR)/d11shm_partial.c
else
D11SHM_PPCMD = $(CC) $(D11SHM_PPFLAGS) -E $(D11SHM_TEMPDIR)/d11shm_structs_inits.c -o $(D11SHM_TEMPDIR)/d11shm_partial.c
endif

define d11shm_func
	for D11REV in $$(<$2); do \
		perl $(D11SHM_TOOLS)/d11shm.pl $(D11SHM_ARGS) && \
		$(D11SHM_PPCMD) && \
		perl $(D11SHM_TOOLS)/d11shm_c.pl $(D11SHM_C_ARGS); \
	done
	perl $(D11SHM_TOOLS)/d11shm_func.pl $(D11SHM_FUNC_ARGS)
endef

$(D11SHM_HEADER): $(D11SHM_DEPS) wlconf.h
	rm -f $(D11SHM_TEMPDIR)/d11shm*.*
	@echo "Generating D11 SHM, using shmdefs files from STD:$(D11SHMDEFS) EAP:$(D11SHMDEFS) WOWL/ULP:$(D11SHMDEFS_WOWL) blddir $(D11SHM_TEMPDIR)"
ifeq ($(D11SHM_WIN), 1)
	cl -c -nologo -D_CRT_SECURE_NO_DEPRECATE -DUNRELEASEDCHIP=1 -I$(D11SHM_SRCBASE)/wl/sys/wlc_cfg.h $(D11SHM_IFLAGS) -I. $(WIN_INCLFLAGS) /FI$(D11SHM_CFGFILE) /Fo$(D11SHM_TEMPDIR)/d11shm_rev.obj $(D11SHM_TOOLS)/d11shm_rev.c && \
	link -nologo -MACHINE:i386  -subsystem:console $(WIN_LDFLAGS) -OUT:$(D11SHM_TEMPDIR)/d11shm_rev.exe $(D11SHM_TEMPDIR)/d11shm_rev.obj
else
	gcc -I$(D11SHM_SRCBASE)/wl/sys/ $(D11SHM_IFLAGS) -DUNRELEASEDCHIP=1 -I. -include $(D11SHM_SRCBASE)/wl/sys/wlc_cfg.h \
	$(D11SHM_TOOLS)/d11shm_rev.c -o $(D11SHM_TEMPDIR)/d11shm_rev
endif
	cd ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/wl && ./d11shm_rev > d11shm_revs.txt

	#Adding shmdefs of standard ucode to d11shm.c
	$(call d11shm_func,std,$(D11SHM_TEMPDIR)/d11shm_revs.txt)

ifdef D11SHM_EAP
	#Adding shmdefs of EAP ucode to d11shm.c
	$(call d11shm_func,eap,$(D11SHM_TEMPDIR)/d11shm_revs.txt)
endif

ifdef D11SHM_WOWL
	#Adding shmdefs of wowl ucode to d11shm.c
	$(call d11shm_func,wowl,$(D11SHM_TEMPDIR)/d11shm_revs.txt)
endif

ifdef D11SHM_ULP
	#Adding shmdefs of ulp ucode to d11shm.c
	$(call d11shm_func,ulp,$(D11SHM_TEMPDIR)/d11shm_revs.txt)
endif

	cat $(D11SHM_TEMPDIR)/d11shm_main_structs.c $(D11SHM_TEMPDIR)/d11shm_main_functions.c >> $(D11SHM_TEMPDIR)/d11shm.c

$(D11SHM_TEMPDIR)/d11shm.c: $(D11SHM_HEADER)
