############################################################
#  Copyright (C) 2020 Broadcom.
#  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to
#  the terms and conditions of a separate, written license agreement executed
#  between you and Broadcom (an "Authorized License").  Except as set forth in
#  an Authorized License, Broadcom grants no license (express or implied),
#  right to use, or waiver of any kind with respect to the Software, and
#  Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use all
#  reasonable efforts to protect the confidentiality thereof, and to use this
#  information only in connection with your use of Broadcom integrated circuit
#  products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
############################################################
ifndef B_BP3_TARGETS_DEFINED
B_BP3_TARGETS_DEFINED := y

ifndef B_REFSW_TOP
$(error B_REFSW_TOP must be defined)
endif

# Specify any prerequisite subpackages here:
include $(B_REFSW_TOP)/BSEAV/opensource/openssl/b_openssl_targets.inc
B_BP3_SUB_BUILDS += b_openssl_target_build
B_BP3_SUB_CLEANS += b_openssl_target_clean
B_BP3_SUB_CLEAN_ALL += b_openssl_target_clean_all
B_BP3_BUILD_FILES += $(B_OPENSSL_BUILD_FILES)
B_OPENSSL_TARGET_MAKEFLAGS += $(B_BP3_TARGET_MAKEFLAGS)

include $(B_REFSW_TOP)/BSEAV/opensource/curl/b_curl_targets.inc
B_BP3_SUB_BUILDS += b_curl_target_build
B_BP3_SUB_CLEANS += b_curl_target_clean
B_BP3_SUB_CLEAN_ALL += b_curl_target_clean_all
B_BP3_BUILD_FILES += $(B_CURL_BUILD_FILES)
B_CURL_TARGET_MAKEFLAGS += $(B_BP3_TARGET_MAKEFLAGS)

include $(B_REFSW_TOP)/BSEAV/opensource/cjson/b_libcjson_targets.inc
B_BP3_SUB_BUILDS += b_cjson_target_build
B_BP3_SUB_CLEANS += b_cjson_target_clean
B_BP3_SUB_CLEAN_ALL += b_cjson_target_clean_all
B_BP3_BUILD_FILES += $(B_CJSON_BUILD_FILES)
B_CJSON_TARGET_MAKEFLAGS += $(B_BP3_TARGET_MAKEFLAGS)

include $(B_REFSW_TOP)/BSEAV/opensource/civetweb/b_civetweb_targets.inc
B_BP3_SUB_BUILDS += b_civetweb_target_build
B_BP3_SUB_CLEANS += b_civetweb_target_clean
B_BP3_SUB_CLEAN_ALL += b_civetweb_target_clean_all
B_BP3_BUILD_FILES += $(B_CIVETWEB_BUILD_FILES)
B_CIVETWEB_TARGET_MAKEFLAGS += $(B_BP3_TARGET_MAKEFLAGS)

ifneq ($(PRECOMPILED_NXCLIENT),y)
ifeq ($(NXCLIENT_SUPPORT),y)
    include $(NEXUS_TOP)/nxclient/server/b_nxserver_targets.inc
    B_BP3_SUB_BUILDS += b_nxserver_target_build
    B_BP3_SUB_CLEANS += b_nxserver_target_clean
    B_BP3_SUB_CLEAN_ALL += b_nxserver_target_clean_all
    B_BP3_BUILD_FILES += $(B_NXSERVER_BUILD_FILES)
    B_NXSERVER_TARGET_MAKEFLAGS += $(B_BP3_TARGET_MAKEFLAGS)
    B_BP3_TARGET_MAKEFLAGS += NXCLIENT_SUPPORT=y
else
    include $(NEXUS_TOP)/build/b_nexus_targets.inc
    B_BP3_SUB_BUILDS += b_nexus_target_build
    B_BP3_SUB_CLEANS += b_nexus_target_clean
    B_BP3_SUB_CLEAN_ALL += b_nexus_target_clean_all
    B_BP3_BUILD_FILES += $(B_NEXUS_BUILD_FILES)
    B_NEXUS_TARGET_MAKEFLAGS += $(B_BP3_TARGET_MAKEFLAGS)
endif
endif

.PHONY: b_bp3_target_dummy
b_bp3_target_dummy:
	@echo "Error - please define your default rule before including any b_xxx_targets.inc files"
	@false

.PHONY: b_bp3_target_build
b_bp3_target_build: $(B_BP3_SUB_BUILDS)
	@echo "[Package bp3: build starting...] by $(abspath $(word 1,$(MAKEFILE_LIST)))"
	$(Q_)$(MAKE) -C $(B_REFSW_TOP)/BSEAV/tools/bp3 $(sort $(B_BP3_TARGET_MAKEFLAGS)) build_without_subs
	@echo "[Package bp3: build complete...] by $(abspath $(word 1,$(MAKEFILE_LIST)))"

b_bp3_status_target_build:
	@echo "[Package bp3: build starting...] by $(abspath $(word 1,$(MAKEFILE_LIST)))"
	$(Q_)$(MAKE) -C $(B_REFSW_TOP)/BSEAV/tools/bp3 $(sort $(B_BP3_TARGET_MAKEFLAGS)) status_lib_only
	@echo "[Package bp3: build complete...] by $(abspath $(word 1,$(MAKEFILE_LIST)))"

.PHONY: b_bp3_target_clean
b_bp3_target_clean: $(B_BP3_SUB_CLEANS)
	@echo "[Package bp3: clean starting...] by $(abspath $(word 1,$(MAKEFILE_LIST)))"
	$(Q_)$(MAKE) -C $(B_REFSW_TOP)/BSEAV/tools/bp3 $(sort $(B_BP3_TARGET_MAKEFLAGS))  clean_without_subs
	@echo "[Package bp3: clean complete...] by $(abspath $(word 1,$(MAKEFILE_LIST)))"

.PHONY: b_bp3_target_clean_all
b_bp3_target_clean_all: b_bp3_target_clean $(B_BP3_SUB_CLEAN_ALL)

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${B_REFSW_TOP}/${B_REFSW_OBJ_DIR}

B_BP3_BUILD_FILES +=

endif # B_BP3_TARGETS_DEFINED
