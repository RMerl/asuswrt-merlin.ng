###############################################################################
#   Copyright (C) 2019 Broadcom.
#   The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to
#   the terms and conditions of a separate, written license agreement executed
#   between you and Broadcom (an "Authorized License").  Except as set forth in
#   an Authorized License, Broadcom grants no license (express or implied),
#   right to use, or waiver of any kind with respect to the Software, and
#   Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#   THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#   IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization,
#   constitutes the valuable trade secrets of Broadcom, and you shall use all
#   reasonable efforts to protect the confidentiality thereof, and to use this
#   information only in connection with your use of Broadcom integrated circuit
#   products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#   "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#   OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#   RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#   IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#   A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#   ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#   THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#   OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#   INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#   RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#   HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#   EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#   WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#   FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
###############################################################################

B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
    DEBUG_SUFFIX=debug
else
    DEBUG_SUFFIX=release
endif

ifndef B_REFSW_OBJ_ROOT
$(error B_REFSW_OBJ_ROOT must be defined)
endif

ifndef B_REFSW_TOP
$(error B_REFSW_TOP must be defined)
endif

include $(B_REFSW_TOP)/BSEAV/opensource/openssl/openssl.inc
include $(B_REFSW_TOP)/BSEAV/opensource/curl/curl.inc
include $(B_REFSW_TOP)/BSEAV/opensource/cjson/cjson.inc
include $(B_REFSW_TOP)/BSEAV/opensource/civetweb/civetweb.inc

BP3_LDFLAGS:= -L$(abspath ${B_REFSW_OBJ_ROOT}/nexus/bin) $(NEXUS_LDFLAGS) -lm -lbp3_host $(B_OPENSSL_LDFLAGS) $(CJSON_LDFLAGS)  $(CURL_LDFLAGS)

# addition needed for nexus_tee_bp3_bin_path.h
BP3_CFLAGS += -I$(NEXUS_TOP)/modules/tee/src/linuxuser

# This is used to get status only
BP3_STATUS_LDFLAGS:= $(B_REFSW_OBJ_ROOT)/BSEAV/tools/bp3/bp3_status.a
