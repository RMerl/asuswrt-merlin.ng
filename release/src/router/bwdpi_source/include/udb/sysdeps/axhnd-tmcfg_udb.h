/*
 * Copyright 2014 Trend Micro Incorporated
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

/*
 * Automatically generated make config: don't edit
 * Date: Thu May 24 11:00:44 2018
 */
#ifndef __TMCFG__UDB_AUTOCONF_OUTPUT_H_
#define __TMCFG__UDB_AUTOCONF_OUTPUT_H_

#ifndef __TMCFG__AUTOCONF_OUTPUT_H_
#include "tdts/tmcfg.h"
#endif

#undef TMCFG_APP_U_TEMPLATE
#undef TMCFG_APP_U_UDB_SAMPLE
#undef TMCFG_APP_U_EXTRA_LDFLAGS
#undef TMCFG_APP_U_EXTRA_CFLAGS
#undef TMCFG_APP_U_TC_PFX
#undef TMCFG_APP_U_TC_OBJDUMP
#undef TMCFG_APP_U_TC_PFX
#undef TMCFG_APP_U_TC_STRIP
#undef TMCFG_APP_U_TC_PFX
#undef TMCFG_APP_U_TC_RANLIB
#undef TMCFG_APP_U_TC_PFX
#undef TMCFG_APP_U_TC_LD
#undef TMCFG_APP_U_TC_PFX
#undef TMCFG_APP_U_TC_AR
#undef TMCFG_APP_U_TC_PFX
#undef TMCFG_APP_U_TC_CC
#undef TMCFG_APP_U_TC_PFX
#undef TMCFG_APP_K_TEMPLATE
#undef TMCFG_APP_K_TDTS_NFFW
#undef TMCFG_APP_K_EXTRA_CFLAGS
#undef TMCFG_DBG_HIT_RATE_TEST
#undef TMCFG_DBG_VERBOSE_CC_MSG
#undef TMCFG_HOST_TC_RUN_STRIP
#undef TMCFG_HOST_TC_PFX
#undef TMCFG_HOST_TC_STRIP
#undef TMCFG_HOST_TC_PFX
#undef TMCFG_HOST_TC_CC
#undef TMCFG_HOST_TC_PFX
#undef TMCFG_TC_RUN_STRIP
#undef TMCFG_TC_EXTRA_LDFLAGS
#undef TMCFG_TC_EXTRA_CFLAGS
#undef TMCFG_TC_PFX
#undef TMCFG_TC_OBJDUMP
#undef TMCFG_TC_PFX
#undef TMCFG_TC_STRIP
#undef TMCFG_TC_PFX
#undef TMCFG_TC_RANLIB
#undef TMCFG_TC_PFX
#undef TMCFG_TC_AR
#undef TMCFG_TC_PFX
#undef TMCFG_TC_CC
#undef TMCFG_TC_BIT_FIELD_ORDER_BIG_ENDIAN
#undef TMCFG_TC_BIT_FIELD_ORDER_LITTLE_ENDIAN
#undef TMCFG_TC_PFX
#undef TMCFG_CPU_64BITS
#undef TMCFG_CPU_32BITS
#undef TMCFG_KERN_ARCH
#undef TMCFG_KERN_DIR
#undef TMCFG_CPU_LITTLE_ENDIAN
#undef TMCFG_CPU_BIG_ENDIAN
#undef TMCFG_ARCH_ARM
#undef TMCFG_ARCH_MIPS
#undef TMCFG_ARCH_X86
#undef TMCFG_KERN_SPACE
#undef TMCFG_MODEL
#undef TMCFG_BRAND

#define TMCFG_BRAND_ASUS 1 // y
#define TMCFG_BRAND "asus"
#define TMCFG_MODEL_RT_AX88U 1 // y
#define TMCFG_MODEL "rt-ax88u"
#define TMCFG_OEM_SRC 1 // y
#define TMCFG_OEM_SRC_BRCM_FC 1 // y

/*
 * Target device information
 */
#define TMCFG_KERN_SPACE 1 // y
#define TMCFG_ARCH_X86 0 // n
#define TMCFG_ARCH_MIPS 0 // n
#define TMCFG_ARCH_ARM 1 // y
#define TMCFG_CPU_32BITS 0 // n
#define TMCFG_CPU_64BITS 1 // y
#define TMCFG_CPU_BIG_ENDIAN 0 // n
#define TMCFG_CPU_LITTLE_ENDIAN 1 // y
#define TMCFG_KERN_DIR "/opt/ASUS/asuswrt_AX88U_GPL_384_1474/release/src-rt-5.02axhnd/kernel/linux-4.1"
#define TMCFG_KERN_ARCH "arm64"

/*
 * Toolchain (TC) configurations
 */
#define TMCFG_TC_PFX "/opt/toolchains/crosstools-aarch64-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/usr/bin/aarch64-linux-"

/*
 * Advanced Build Options
 */
#define TMCFG_TC_BIT_FIELD_ORDER_LITTLE_ENDIAN 1 // y
#define TMCFG_TC_BIT_FIELD_ORDER_BIG_ENDIAN 0 // n
#define TMCFG_TC_CC "$(TMCFG_TC_PFX)gcc"
#define TMCFG_TC_AR "$(TMCFG_TC_PFX)ar"
#define TMCFG_TC_LD "$(TMCFG_TC_PFX)ld"
#define TMCFG_TC_RANLIB "$(TMCFG_TC_PFX)ranlib"
#define TMCFG_TC_STRIP "$(TMCFG_TC_PFX)strip"
#define TMCFG_TC_OBJDUMP "$(TMCFG_TC_PFX)objdump"
#define TMCFG_TC_EXTRA_CFLAGS "-fsigned-char"
#define TMCFG_TC_EXTRA_LDFLAGS ""
#define TMCFG_TC_RUN_STRIP 1 // y

/*
 * Local host tool chain
 */
#define TMCFG_HOST_TC_PFX ""
#define TMCFG_HOST_TC_CC "$(TMCFG_HOST_TC_PFX)gcc"
#define TMCFG_HOST_TC_STRIP "$(TMCFG_HOST_TC_PFX)strip"
#define TMCFG_HOST_TC_RUN_STRIP 1 // y

/*
 * Debug
 */
#define TMCFG_DBG_VERBOSE_CC_MSG 1 // y
#define TMCFG_DBG_HIT_RATE_TEST 0 // n

/*
 * UDB
 */
#define TMCFG_E_UDB_CORE 1 // y
#define TMCFG_E_UDB_CORE_MAJ_VER 0
#define TMCFG_E_UDB_CORE_MIN_VER 2
#define TMCFG_E_UDB_CORE_REV_VER 18
#define TMCFG_E_UDB_CORE_SHN_REV_NUM 0
#define TMCFG_E_UDB_CORE_USE_KBUILD 1 // y
#define TMCFG_E_UDB_CORE_EXTRA_CFLAGS ""
#define TMCFG_E_UDB_CORE_CONN_EXTRA 1 // y
#define TMCFG_E_UDB_CORE_RULE_FORMAT_V2 0 // n
#define TMCFG_E_UDB_CORE_URL_QUERY 1 // y
#define TMCFG_E_UDB_CORE_SHN_QUERY 0 // n
#define TMCFG_E_UDB_CORE_APP_WBL 0 // n
#define TMCFG_E_UDB_CORE_WBL 1 // y
#define TMCFG_E_UDB_CORE_WBL_MAJ_VER 2
#define TMCFG_E_UDB_CORE_WBL_MIN_VER 2
#define TMCFG_E_UDB_CORE_WBL_MAC_LIST 1 // y
#define TMCFG_E_UDB_CORE_WBL_WILDCARD 0 // n
#define TMCFG_E_UDB_CORE_WBL_VDR 0 // n
#define TMCFG_E_UDB_CORE_WBL_OP_TYPE_1 1 // y
#define TMCFG_E_UDB_CORE_WBL_OP_TYPE_2 0 // n
#define TMCFG_E_UDB_CORE_WBL_OP_TYPE_3 0 // n
#define TMCFG_E_UDB_CORE_WBL_OP_TYPE_4 0 // n
#define TMCFG_E_UDB_CORE_WBL_PROF_NUM 16
#define TMCFG_E_UDB_CORE_WBL_MAC_NUM 256
#define TMCFG_E_UDB_CORE_WBL_URL_NUM 1024
#define TMCFG_E_UDB_CORE_WEB_FUNC 1 // y
#define TMCFG_E_UDB_CORE_DC 1 // y
#define TMCFG_E_UDB_CORE_DC_UNKNOWN_DEVID 1 // y
#define TMCFG_E_UDB_CORE_ANOMALY_PREVENT 1 // y
#define TMCFG_E_UDB_CORE_VIRTUAL_PATCH 1 // y
#define TMCFG_E_UDB_CORE_SWNAT 0 // n
#define TMCFG_E_UDB_CORE_IQOS_SUPPORT 1 // y
#define TMCFG_E_UDB_CORE_IQOS_RSV_DEF_CLS 0 // n
#define TMCFG_E_UDB_CORE_GCTRL_SUPPORT 0 // n
#define TMCFG_E_UDB_CORE_HWNAT 0 // n
#define TMCFG_E_UDB_CORE_HWQOS 0 // n
#define TMCFG_E_UDB_CORE_APP_PATROL 1 // y
#define TMCFG_E_UDB_CORE_PATROL_TIME_QUOTA 1 // y
#define TMCFG_E_UDB_CORE_APP_REDIRECT_URL 1 // y
#define TMCFG_E_UDB_CORE_PATROL_TIME_GRP_NUM 32
#define TMCFG_E_UDB_CORE_PATROL_TIME_DEV_NUM 6
#define TMCFG_E_UDB_CORE_PROG_CTRL 1 // y
#define TMCFG_E_UDB_CORE_PROG_LIC_CTRL_NONE 1 // y
#define TMCFG_E_UDB_CORE_PROG_LIC_CTRL_V1 0 // n
#define TMCFG_E_UDB_CORE_PROG_LIC_CTRL_V2 0 // n
#define TMCFG_E_UDB_CORE_WPR_PAGE 1 // y
#define TMCFG_E_UDB_CORE_TMDBG 0 // n
#define TMCFG_E_UDB_CORE_MEMTRACK 0 // n
#define TMCFG_E_UDB_CORE_HTTP_REFER 0 // n
#define TMCFG_E_UDB_SHELL 1 // y
#define TMCFG_E_UDB_SHELL_EXTRA_CFLAGS ""
#define TMCFG_E_UDB_SHELL_KMOD_NAME "tdts_udb"
#define TMCFG_E_UDB_SHELL_IOCTL_DEV_NAME "idpfw"
#define TMCFG_E_UDB_SHELL_IOCTL_DEV_MAJ 191
#define TMCFG_E_UDB_SHELL_IOCTL_DEV_MIN 0
#define TMCFG_E_UDB_SHELL_IOCTL_DEV_MAGIC 191
#define TMCFG_E_UDB_SHELL_CT_MARK_RSV 1 // y
#define TMCFG_E_UDB_SHELL_CT_MARK_RSV_BITS 3
#define TMCFG_E_UDB_SHELL_PROCFS 1 // y
#define TMCFG_E_REL_PKG_MAJ_VER 2
#define TMCFG_E_REL_PKG_MIN_VER 0
#define TMCFG_E_REL_PKG_REV_VER 1
#define TMCFG_E_REL_PKG_LOCAL_VER "8f7e487"

/*
 * Accompany applications or modules
 */

/*
 * Kernel
 */
#define TMCFG_APP_K_EXTRA_CFLAGS "-I/opt/ASUS/asuswrt_AX88U_GPL_384_1474/release/src-rt-5.02axhnd/shared/opensource/include/bcm963xx -I/opt/ASUS/asuswrt_AX88U_GPL_384_1474/release/src-rt-5.02axhnd/bcmdrivers/broadcom/include/bcm963xx -I/opt/ASUS/asuswrt_AX88U_GPL_384_1474/release/src-rt-5.02axhnd/bcmdrivers/opensource/include/bcm963xx"
#define TMCFG_APP_K_TDTS_NFFW 0 // n
#define TMCFG_APP_K_TDTS_UDBFW 1 // y
#define TMCFG_APP_K_TDTS_UDBFW_EXTRA_SYMBOLS ""
#define TMCFG_APP_K_TDTS_UDBFW_CT_NOTIF 1 // y
#define TMCFG_APP_K_TDTS_UDBFW_FAST_PATH 1 // y
#define TMCFG_APP_K_TDTS_UDBFW_META_EXTRACT 1 // y
#define TMCFG_APP_K_TDTS_UDBFW_TC_WQ 0 // n
#define TMCFG_APP_K_TDTS_UDBFW_QOS_NETLINK_ID 21
#define TMCFG_APP_K_TDTS_UDBFW_WRS_NETLINK_ID 2
#define TMCFG_APP_K_TEMPLATE 0 // n

/*
 * Userland
 */

/*
 * Userspace toolchain
 */
#define TMCFG_APP_U_TC_PFX "/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/usr/bin/arm-linux-"
#define TMCFG_APP_U_TC_CC "$(TMCFG_APP_U_TC_PFX)gcc"
#define TMCFG_APP_U_TC_AR "$(TMCFG_APP_U_TC_PFX)ar"
#define TMCFG_APP_U_TC_LD "$(TMCFG_APP_U_TC_PFX)ld"
#define TMCFG_APP_U_TC_RANLIB "$(TMCFG_APP_U_TC_PFX)ranlib"
#define TMCFG_APP_U_TC_STRIP "$(TMCFG_APP_U_TC_PFX)strip"
#define TMCFG_APP_U_TC_OBJDUMP "$(TMCFG_APP_U_TC_PFX)objdump"
#define TMCFG_APP_U_EXTRA_CFLAGS ""
#define TMCFG_APP_U_EXTRA_LDFLAGS ""

/*
 * Select 3rd party libraries (import)
 */
#define TMCFG_APP_U_BUILD_OPENSSL 1 // y
#define TMCFG_APP_U_BUILD_LIBEVENT 1 // y
#define TMCFG_APP_U_BUILD_PROTOBUF 1 // y
#define TMCFG_APP_U_SHN_UTILS 1 // y

/*
 * SHN brand and model names
 */
#define TMCFG_APP_U_SHN_BRAND_NAME "ASUS"
#define TMCFG_APP_U_SHN_MODEL_NAME "__AUTO__"
#define TMCFG_APP_U_SHN_CUSTOM_NAME 0 // n
#define TMCFG_APP_U_DCD 1 // y

/*
 * Data Collection Daemon options
 */
#define TMCFG_APP_U_DCD_PROG_CTRL 1 // y
#define TMCFG_APP_U_DCD_SOCKET_DIRECTORY "/var"
#define TMCFG_APP_U_DCD_FB_HOST "ntd-asus-2014b-en.fbs20.trendmicro.com"
#define TMCFG_APP_U_DCD_FB_INFO_PROD_ID "ntd-asus-2014b"
#define TMCFG_APP_U_DCD_FB_STAT_PROD_ID "ntd-asus-2014b"
#define TMCFG_APP_U_DCD_FB_DEF_CERT "ntdasus2014.cert"
#define TMCFG_APP_U_DCD_FB_CUSTOM 0 // n
#define TMCFG_APP_U_TDTS_WRED 1 // y

/*
 * WRS Daemon Options
 */
#define TMCFG_APP_U_TDTS_WRED_DC_HEARTBEAT 1 // y
#define TMCFG_APP_U_TDTS_WRED_THREAD_POOL 1 // y
#define TMCFG_APP_U_TDTS_WRED_PROG_CTRL 1 // y
#define TMCFG_APP_U_WRS_LOCAL_CACHE 0x7D000
#define TMCFG_APP_U_WRED_TMUFE_LICENSE_ID "RGOM10"
#define TMCFG_APP_U_WRED_TMUFE_VENDOR_ID "ASUS"
#define TMCFG_APP_U_WRED_TMUFE_RS_HOST "rgom10-en.url.trendmicro.com"
#define TMCFG_APP_U_WRED_TMUFE_CUSTOM 0 // n
#define TMCFG_APP_U_TDTS_WRED_TMUFE21 0 // n
#define TMCFG_APP_U_WBL 1 // y

/*
 * Selected WBL Options
 */
#define TMCFG_APP_U_WBL_MAC_LIST 1 // y
#define TMCFG_APP_U_TDTS_SHNAGENT 0 // n
#define TMCFG_APP_U_SHN_CTRL 1 // y
#define TMCFG_APP_U_UDB_SAMPLE 1 // y
#define TMCFG_APP_U_TC_DAEMON 1 // y
#define TMCFG_APP_U_MTK 0 // n
#define TMCFG_APP_U_MTK_V2 0 // n
#define TMCFG_APP_U_PROG_CTRL 1 // y
#define TMCFG_APP_U_DEMO_GUI 0 // n
#define TMCFG_APP_U_DEMO_GUI_V22 0 // n
#define TMCFG_APP_U_TEMPLATE 0 // n

#endif // EOF

