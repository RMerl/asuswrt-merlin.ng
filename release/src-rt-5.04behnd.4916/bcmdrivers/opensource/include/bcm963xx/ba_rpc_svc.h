/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/****************************************************************************
 * Power RPC Driver
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/

#ifndef _BA_RPC_SVC_H_
#define _BA_RPC_SVC_H_

#include <linux/types.h>
#include <linux/platform_device.h>
#include <itc_rpc.h>

#define RPC_SERVICE_VER_BA_RUN_STATE_ID         0
#define RPC_SERVICE_VER_BA_XPORT_SET_PWR        0
#define RPC_SERVICE_VER_BA_XRDP_INIT            0
#define RPC_SERVICE_VER_BA_WAN_ZONE_PWR_CTRL    0
#define RPC_SERVICE_VER_BA_RNR_SRAM_DONE        0
#define RPC_SERVICE_VER_BA_WOL_INTR_EN          0
#define RPC_SERVICE_VER_BA_GET_SMCBL_VER        0
#define RPC_SERVICE_VER_BA_GET_SMCBL_VER_HASH   0
#define RPC_SERVICE_VER_BA_GET_SMCOS_VER        0
#define RPC_SERVICE_VER_BA_GET_SMCOS_VER_HASH   0
#define RPC_SERVICE_VER_BA_RPRT_BOOT_SUCCESS  0
#define RPC_SERVICE_VER_BA_SVC_GET_BOOT_FAIL_CNT  0

enum ba_svc_func_idx
{
    BA_SVC_CPU_ID,
    BA_SVC_CPU_NAME,
    BA_SVC_RUN_STATE_ID,
    BA_SVC_RUN_STATE_NAME,
    BA_SVC_GET_RUN_STATE,
    BA_SVC_NOTIFY_RUN_STATE,
    BA_SVC_REQUEST_RUN_STATE,
    BA_SVC_REQUEST_RUN_STATE_RESPONSE,
    BA_SVC_SET_RUN_STATE,
    BA_SVC_BOOT_FROM_ADDR,
    BA_SVC_XPORT_SET_PWR,
    BA_SVC_RNR_SRAM_DONE,
    BA_SVC_WOL_INTR_EN,
    BA_SVC_GET_SMCBL_VER,
    BA_SVC_GET_SMCBL_VER_HASH,
    BA_SVC_GET_SMCOS_VER,
    BA_SVC_GET_SMCOS_VER_HASH,
    BA_SVC_RPRT_BOOT_SUCCESS,
    BA_SVC_GET_BOOT_FAIL_CNT,
    BA_SVC_FUNC_MAX
};

enum ba_req_rs_rsp {
    BA_SVC_RESPONSE_READY,
    BA_SVC_RESPONSE_BUSY,
    BA_SVC_RESPONSE_MAX
};

struct ba_msg {
    uint32_t    hdr;
    union {
        uint32_t    rsvd0;
        struct {
            uint8_t cpu_id;
            uint8_t rs_id;
            union {
                uint8_t be_rude:1;
                uint8_t rsvd1:7;

                uint8_t response:4;
                uint8_t rsvd2:4;
            };
            uint8_t rc:8;
        };
    };
    union {
        uint32_t    rsvd3[2];
        char        name[8];
    };
};

/* 
    This is configurable parameter in git core.abbrev parameter currently it configure to 9 use more 3 byte for spare
    In case of changing lenght of this parameters need  to change it also in the SMC_OS and SMC_OS
*/
#define HASH_SHORT_SIZE         (12)

typedef struct 
{
    uint16_t       smcos_major_ver;
    uint16_t       smcos_minor_ver;
    uint16_t       smcos_rev;
    uint16_t       pon_major_ver;
    uint16_t       pon_minor_ver;
    uint16_t       pon_patch_ver; 
    char           smcos_ver_hash[HASH_SHORT_SIZE]; 
} smcos_ver_t;

typedef struct 
{
    uint16_t       smcbl_major_ver;
    uint16_t       smcbl_minor_ver;
    uint16_t       smcbl_rev;
    uint16_t       ponbl_major_ver;
    uint16_t       ponbl_minor_ver;
    char           smcbl_ver_hash[HASH_SHORT_SIZE];
} smcbl_ver_t;

#define BA_SVC_RESET_BOOT_WDOG	1
#define BA_SVC_RESET_BOOT_COUNT	2

/* ba svc functions */
int bcm_rpc_ba_xport_set_state(uint8_t port_id, uint8_t enable);
int bcm_rpc_ba_notify_runner_load_sram_done(void);
int bcm_rpc_ba_wol_intr_enable(void);
int bcm_rpc_ba_get_smcbl_ver(smcbl_ver_t  *smcbl_ver);
int bcm_rpc_ba_get_smcos_ver(smcos_ver_t  *smcos_ver);
int bcm_rpc_ba_report_boot_success(uint32_t flags);
int bcm_rpc_ba_get_boot_fail_cnt(void);
#endif

