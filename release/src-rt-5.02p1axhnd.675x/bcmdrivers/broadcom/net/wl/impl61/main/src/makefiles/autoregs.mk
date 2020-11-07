#
# autoregs.mk  make file - Geneates core regs files [similar to d11shm counterpart]
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
# $Id: $

AUTOREGS_IFLAGS ?= -I.
AUTOREGS_CFLAGS ?= -I.
AUTOREGS_TEMPDIR ?= .
AUTOREGS_NAMED_INIT ?= 1
AUTOREGS_SRCBASE ?= $(SRCBASE)

AUTOREGS_TOOLS = $(AUTOREGS_SRCBASE)/../components/shared/autoregs/scripts
AUTOREGS_OFFSETS = $(AUTOREGS_SRCBASE)/../components/shared/autoregs/coreregoffs

AUTOREGS_DEPS = $(AUTOREGS_SRCBASE)/shared/autoregstpl/%regs.tpl
AUTOREGS_DEPS += $(AUTOREGS_SRCBASE)/makefiles/autoregs.mk
AUTOREGS_DEPS += $(AUTOREGS_TOOLS)/autoregs_rev.c
AUTOREGS_DEPS += $(AUTOREGS_TOOLS)/autoregs.pl
AUTOREGS_DEPS += $(AUTOREGS_TOOLS)/autoregs_c.pl
AUTOREGS_DEPS += $(AUTOREGS_TOOLS)/autoregs_func.pl
AUTOREGS_DEPS += $(AUTOREGS_TOOLS)/autoregs_gen_offs_struct.pl
AUTOREGS_DEPS += $(AUTOREGS_OFFSETS)/%regoffs.h

AUTOREGS_IFLAGS += -I$(AUTOREGS_SRCBASE)/include
AUTOREGS_CFLAGS += -I$(AUTOREGS_SRCBASE)/include

AUTOREGS_CFLAGS_COREREVS := $(shell echo $(AUTOREGS_CFLAGS) | egrep -o '\-DBCMPCIE[A-Z]+=[0-9]+')

# Arguments for the autoregs perl scripts
AUTOREGS_FUNC_ARGS = --core_name=$(1)\
			--i_autoregs_func_partial=$(AUTOREGS_TEMPDIR)/$(1)regs_func_ucode_partial.c \
			--o_autoregs_main_functions=$(AUTOREGS_TEMPDIR)/$(1)regs_main_functions.c \
			--o_autoregs_func_decl=$(AUTOREGS_TEMPDIR)/$(1)regs_func_decl.h

AUTOREGS_C_ARGS =  --core_rev=$$CORE_REV \
			--core_name=$(1)\
			--i_autoregs_count=$(3) \
			--i_autoregs_partial=$(AUTOREGS_TEMPDIR)/$(1)regs_partial.c \
			--o_autoregs_c=$(AUTOREGS_TEMPDIR)/$(1)regsoffs.c

AUTOREGS_ARGS = --core_rev=$$CORE_REV --named_init=$(AUTOREGS_NAMED_INIT) \
			--core_name=$(1)\
			--i_autoregs_template=$(AUTOREGS_SRCBASE)/shared/autoregstpl/$(1)regs.tpl \
			--i_autoregs_count=$(3) \
			--o_autoregs_defaults=$(AUTOREGS_TEMPDIR)/$(1)regs_defaults.h \
			--o_autoregs_structs_decl=$(AUTOREGS_TEMPDIR)/$(1)regs_structs_decl.h \
			--o_autoregs_regdefs_t=$(AUTOREGS_TEMPDIR)/$(1)regs_regdefs_t.h \
			--o_autoregs_structs_inits=$(AUTOREGS_TEMPDIR)/$(1)regs_structs_inits.c \
			--o_autoregs_main_structs=$(AUTOREGS_TEMPDIR)/$(1)regs_main_structs.c \
			--o_autoregs_declarations=$(AUTOREGS_TEMPDIR)/$(1)regs_declarations.h \
			--o_autoregs_hdr=$(AUTOREGS_TEMPDIR)/$(1)regsoffs.h \
			--o_autoregs_func_partial=$(AUTOREGS_TEMPDIR)/$(1)regs_func_ucode_partial.c

AUTOREGS_PPFLAGS = $(AUTOREGS_CFLAGS) -P -DAUTOREGS_COREREV=$$CORE_REV \
			-include $(AUTOREGS_OFFSETS)/$(1)regoffs.h

AUTOREGS_PPFLAGS_WIN = $(AUTOREGS_CFLAGS) -I. -I$(AUTOREGS_TEMPDIR) -DAUTOREGS_COREREV=$$CORE_REV \
			/FI $(AUTOREGS_OFFSETS)/$(1)regoffs.h

AUTOREGS_GEN_OFFS_STRUCT_ARGS = --named_init=$(AUTOREGS_NAMED_INIT) \
			--core_name=$*\
			--i_autoregs_offs_c=$(AUTOREGS_TEMPDIR)/$*regsoffs.c \
			--i_autoregs_structs_inits=$(AUTOREGS_TEMPDIR)/$*regs_structs_inits.c \
			--i_autoregs_count=$$(echo `wc -w $(AUTOREGS_TEMPDIR)/$*regs_revs.txt`) \
			--o_autoregs_offs_struct_h=$(AUTOREGS_TEMPDIR)/$*regsoffs.h

ifeq ($(AUTOREGS_WIN), 1)
SUBSYSTEM = windows
REQUIRE_MSDEV = 1
REQUIRE_WDM7600 = 1
REQUIRE_SDK60 = 1
include $(AUTOREGS_SRCBASE)/makefiles/env.mk
WIN_LDFLAGS = $(foreach p,$(LIBVPATH.W),"/LIBPATH:$p")
WIN_INCLFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSDEV.INCLUDE)))
AUTOREGS_PPCMD = cl -nologo $(AUTOREGS_PPFLAGS_WIN) /EP $(AUTOREGS_TEMPDIR)/$(1)regs_structs_inits.c > $(AUTOREGS_TEMPDIR)/$(1)regs_partial.c
else
AUTOREGS_PPCMD = $(CC) $(AUTOREGS_PPFLAGS) -E $(AUTOREGS_TEMPDIR)/$(1)regs_structs_inits.c -o $(AUTOREGS_TEMPDIR)/$(1)regs_partial.c
endif

define autoregs_func
	for CORE_REV in $$(<$2); do \
		perl $(AUTOREGS_TOOLS)/autoregs.pl $(AUTOREGS_ARGS) && \
		$(AUTOREGS_PPCMD) && \
		perl $(AUTOREGS_TOOLS)/autoregs_c.pl $(AUTOREGS_C_ARGS); \
	done
	perl $(AUTOREGS_TOOLS)/autoregs_func.pl $(AUTOREGS_FUNC_ARGS)
endef

$(AUTOREGS_TEMPDIR)/%regsoffs.h: $(AUTOREGS_DEPS) wlconf.h
	rm -f $(AUTOREGS_TEMPDIR)/$*regs*.*
	@echo "Generating $* regs, using coreregoffs file from $(AUTOREGS_OFFSETS) blddir $(AUTOREGS_TEMPDIR)"
	echo $(AUTOREGS_CFLAGS)
	echo $(AUTOREGS_CFLAGS_COREREVS)

ifeq ($(AUTOREGS_WIN), 1)
	cl -c -nologo -D_CRT_SECURE_NO_DEPRECATE -DUNRELEASEDCHIP=1 -I$(AUTOREGS_SRCBASE)/wl/sys/wlc_cfg.h $(AUTOREGS_IFLAGS) $(AUTOREGS_CFLAGS_COREREVS) -I. $(WIN_INCLFLAGS) /FI$(D11REGS_CFGFILE) /Fo$(AUTOREGS_TEMPDIR)/$*regs_rev.obj $(AUTOREGS_TOOLS)/autoregs_rev.c && \
	link -nologo -MACHINE:i386  -subsystem:console $(WIN_LDFLAGS) -OUT:$(AUTOREGS_TEMPDIR)/$*regs_rev.exe $(AUTOREGS_TEMPDIR)/$*regs_rev.obj
else
	gcc -I$(AUTOREGS_SRCBASE)/wl/sys/ $(AUTOREGS_IFLAGS) $(AUTOREGS_CFLAGS_COREREVS) -DUNRELEASEDCHIP=1 -I. -include $(AUTOREGS_SRCBASE)/wl/sys/wlc_cfg.h \
	$(AUTOREGS_TOOLS)/autoregs_rev.c -o $(AUTOREGS_TEMPDIR)/$*regs_rev
endif
	$(AUTOREGS_TEMPDIR)/$*regs_rev $* > $(AUTOREGS_TEMPDIR)/$*regs_revs.txt

	#Adding regsdefs of standard ucode to {core_name}regsoffs.c
	$(call autoregs_func,$*,$(AUTOREGS_TEMPDIR)/$*regs_revs.txt,$$(echo `wc -w $(AUTOREGS_TEMPDIR)/$*regs_revs.txt`))
	cat $(AUTOREGS_TEMPDIR)/$*regs_main_structs.c $(AUTOREGS_TEMPDIR)/$*regs_main_functions.c >> $(AUTOREGS_TEMPDIR)/$*regsoffs.c
	perl $(AUTOREGS_TOOLS)/autoregs_gen_offs_struct.pl $(AUTOREGS_GEN_OFFS_STRUCT_ARGS)

$(AUTOREGS_TEMPDIR)/d11regsoffs.c: $(AUTOREGS_TEMPDIR)/d11regsoffs.h
$(AUTOREGS_TEMPDIR)/pcieregsoffs.c: $(AUTOREGS_TEMPDIR)/pcieregsoffs.h
