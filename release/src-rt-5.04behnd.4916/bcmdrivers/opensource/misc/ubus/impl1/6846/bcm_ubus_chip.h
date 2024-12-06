/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
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

#ifndef __BCM_UBUS_CHIP_H__
#define __BCM_UBUS_CHIP_H__

#define ROUTE_ADDR_SIZE          0x80
#define TOKEN_SIZE               0x80

typedef struct
{
    uint32_t base_addr;
    uint32_t remap_addr;
    uint32_t attributes;
}DecodeCfgMstWndRegs;

typedef struct
{
    uint32_t ctrl;
    uint32_t cache_cfg;
    uint32_t reserved[2];
    DecodeCfgMstWndRegs window[20];
} DecodeCfgRegs;

typedef struct
{
    uint32_t port_cfg[8];
#define DCM_UBUS_CONGESTION_THRESHOLD 3
    uint32_t reserved1[56];
    uint32_t loopback[20];
    uint32_t reserved2[44];
    uint32_t routing_addr[ROUTE_ADDR_SIZE];
    uint32_t token[TOKEN_SIZE];
    DecodeCfgRegs decode_cfg;
    uint8_t rsvd1[512];
    AXICfgRegs axi_cfg;
} MstPortNode;

#define XRDP_RCQ_GENERAL_DDR_CONGEST_THRESHOLD_SHIFT   4
#define XRDP_RCQ_GENERAL_PSRAM_CONGEST_THRESHOLD_SHIFT 9

#endif
