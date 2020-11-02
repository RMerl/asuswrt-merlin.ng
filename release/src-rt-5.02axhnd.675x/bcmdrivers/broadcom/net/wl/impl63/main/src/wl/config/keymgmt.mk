#
# Helper makefile for building keymgmt included in wl.mk
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
# $Id: keymgmt.mk 774175 2019-04-15 03:32:26Z $

KEYMGMT_SRC_DIR = src/wl/keymgmt/src
BCMCRYPTO_SRC_DIR = components/bcmcrypto/src

KEYMGMT_SRC_FILES = \
	$(KEYMGMT_SRC_DIR)/km_key_aes.c \
	$(KEYMGMT_SRC_DIR)/km_key.c \
	$(KEYMGMT_SRC_DIR)/km_key_none.c \
	\
	$(KEYMGMT_SRC_DIR)/km_alloc.c \
	$(KEYMGMT_SRC_DIR)/km_bsscfg.c \
	$(KEYMGMT_SRC_DIR)/km_scb.c \
	$(KEYMGMT_SRC_DIR)/km_util.c \
	\
	$(KEYMGMT_SRC_DIR)/km_hw_algo.c \
	$(KEYMGMT_SRC_DIR)/km_hw_algo_hwktab.c \
	$(KEYMGMT_SRC_DIR)/km_key_wep.c \
	\
	$(KEYMGMT_SRC_DIR)/km.c \
	$(KEYMGMT_SRC_DIR)/km_b4m4.c \
	$(KEYMGMT_SRC_DIR)/km_event.c \
	$(KEYMGMT_SRC_DIR)/km_hw.c \
	$(KEYMGMT_SRC_DIR)/km_hw_alloc.c \
	$(KEYMGMT_SRC_DIR)/km_ioctl.c \
	$(KEYMGMT_SRC_DIR)/km_iovars.c \
	$(KEYMGMT_SRC_DIR)/km_notify.c \
	$(KEYMGMT_SRC_DIR)/km_tkip.c \

KEYMGMT_SRC_CRYPTO_FILES = \
	$(BCMCRYPTO_SRC_DIR)/wep.c \
	$(BCMCRYPTO_SRC_DIR)/aes.c \
	$(BCMCRYPTO_SRC_DIR)/rc4.c \
	$(BCMCRYPTO_SRC_DIR)/rijndael-alg-fst.c \
	$(BCMCRYPTO_SRC_DIR)/aesgcm.c \
	$(BCMCRYPTO_SRC_DIR)/gcm.c \

#ifdef LINUX_CRYPTO
# LINUX_CRYPTO provides only tkip
ifeq ($(LINUX_CRYPTO), 1)
	KEYMGMT_SRC_FILES += $(KEYMGMT_SRC_DIR)/km_key_tkip_linux.c
endif
#endif // endif

#ifndef LINUX_CRYPTO
ifneq ($(LINUX_CRYPTO),1)
	KEYMGMT_SRC_FILES += $(KEYMGMT_SRC_DIR)/km_key_tkip.c
	KEYMGMT_SRC_COMMON_CRYPTO_FILES += $(BCMCRYPTO_SRC_DIR)/tkmic.c \
		$(BCMCRYPTO_SRC_DIR)/tkhash.c
endif
#endif // endif

#ifdef MFP
ifeq ($(MFP),1)
KEYMGMT_SRC_FILES += $(KEYMGMT_SRC_DIR)/km_key_aes_mcmfp.c
endif
#endif // endif

# XXX once we figure out the right set of flags, we can turn off
# the default for KEYMGMT_DUMP below to 0. Many builds seem to be
# currently dependent on dump functionality by including the compile
# flags but not build configuration flags.
KEYMGMT_DUMP := 1
ifeq ($(DEBUG),1)
	KEYMGMT_DUMP := 1
endif

# for winxx these dump files must be included because the
# set of sources is the same for checked and free builds
ifneq ($(EXTSTA)),)
	KEYMGMT_DUMP := 1
endif

ifneq ($(BCMDBG),)
	KEYMGMT_DUMP := 1
endif

ifeq ($(AP),1)
	KEYMGMT_DUMP := 1
endif

ifeq ($(KEYMGMT_DUMP),1)
	KEYMGMT_SRC_FILES += $(KEYMGMT_SRC_DIR)/km_dump.c
	KEYMGMT_SRC_FILES += $(KEYMGMT_SRC_DIR)/km_hw_dump.c
endif

# XXX See comment above related to KEYMGMT_DUMP. Same comment applies to KEYMMGT_IVTW
KEYMGMT_IVTW := 1
ifneq ($(BRCMAPIVTW),)
	KEYMGMT_IVTW := 1
endif
ifeq ($(AMPDU_HOSTREORDER), 1)
	KEYMGMT_IVTW := 1
endif
ifeq ($(KEYMGMT_IVTW), 1)
	KEYMGMT_SRC_FILES +=  $(KEYMGMT_SRC_DIR)/km_ivtw.c
endif

#ifdef WOWL
ifeq ($(WOWL),1)
	KEYMGMT_SRC_FILES += $(KEYMGMT_SRC_DIR)/km_wowl_hw.c
endif
#endif // endif

#ifdef BCMULP
ifeq ($(BCMULP),1)
	KEYMGMT_SRC_FILES += $(KEYMGMT_SRC_DIR)/km_ulp.c
endif
#endif // endif

KEYMGMT_SRC_FILES += $(KEYMGMT_SRC_CRYPTO_FILES)

##
