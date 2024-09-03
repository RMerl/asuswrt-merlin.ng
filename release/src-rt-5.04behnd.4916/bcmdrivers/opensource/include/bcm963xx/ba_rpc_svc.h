/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

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
#include <pmc/pmc_shutdown.h>

#define RPC_SERVICE_VER_BA_RUN_STATE_ID           0
#define RPC_SERVICE_VER_BA_XPORT_SET_PWR          0
#define RPC_SERVICE_VER_BA_XRDP_INIT              0
#define RPC_SERVICE_VER_BA_WAN_ZONE_PWR_CTRL      0
#define RPC_SERVICE_VER_BA_RNR_SRAM_DONE          0
#define RPC_SERVICE_VER_BA_WOL_INTR_EN            0
#define RPC_SERVICE_VER_BA_GET_SMCBL_VER          0
#define RPC_SERVICE_VER_BA_GET_SMCBL_VER_HASH     0
#define RPC_SERVICE_VER_BA_GET_SMCOS_VER          0
#define RPC_SERVICE_VER_BA_GET_SMCOS_VER_HASH     0
#define RPC_SERVICE_VER_BA_RPRT_BOOT_SUCCESS      0
#define RPC_SERVICE_VER_BA_SVC_GET_BOOT_FAIL_CNT  0
#define RPC_SERVICE_VER_BA_SETUP_TRIGGER          0
#define RPC_SERVICE_VER_BA_SETUP_PWR_PIN          0
#define RPC_SERVICE_VER_BA_DEEP_SLEEP             0

enum ba_svc_func_idx
{
    BA_UBOOT_LINUX_BEGIN = 50,
    BA_SVC_XPORT_SET_PWR = BA_UBOOT_LINUX_BEGIN,
    BA_SVC_GET_SMCBL_VER,
    BA_SVC_GET_SMCBL_VER_HASH,
    BA_SVC_GET_SMCOS_VER,
    BA_SVC_GET_SMCOS_VER_HASH,
    BA_SVC_RPRT_BOOT_SUCCESS,

    /* ATTENTION:
     *
     * All RPC commands issued from Uboot and Linux should be added above this line
     * and synced with 
     * .../bootloaders/u-boot-2019.07/arch/arm/mach-bcmbca/include/ba_svc.h
     *
     * */
    BA_SVC_LINUX_ONLY_BEGIN = 100,
    BA_SVC_RNR_SRAM_DONE = BA_SVC_LINUX_ONLY_BEGIN,
    BA_SVC_GET_BOOT_FAIL_CNT,
    BA_SVC_SETUP_TRIGGER,
    BA_SVC_DEEP_SLEEP,
    BA_SVC_SETUP_PWR_PIN,

    /* ATTENTION:
     *
     * All Linux only RPC commands should be added above this note.
     *
     * */
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
int bcm_rpc_ba_get_smcbl_ver(smcbl_ver_t  *smcbl_ver);
int bcm_rpc_ba_get_smcos_ver(smcos_ver_t  *smcos_ver);
int bcm_rpc_ba_report_boot_success(uint32_t flags);
int bcm_rpc_ba_get_boot_fail_cnt(void);
int bcm_rpc_ba_setup_wake_trigger(wake_type_t wake_type, int param);
int bcm_rpc_ba_setup_pwr_pin(uint32_t pin_number, uint32_t polarity);
int bcm_rpc_ba_deep_sleep(void);
#endif

