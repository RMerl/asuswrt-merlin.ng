/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
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

#ifndef __BCM_UBUS_CHIP_H__
#define __BCM_UBUS_CHIP_H__


#define ROUTE_ADDR_SIZE        0x400
#define TOKEN_SIZE             0x400

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
    uint32_t port_cfg[12];                  /* 0x0 */
#define DCM_UBUS_CONGESTION_THRESHOLD 3  
    uint32_t reserved1[52];                 /* 0x30 */
    uint32_t loopback[20];                  /* 0x100 */
    uint32_t reserved2[556];                /* 0x150 */  
    DecodeCfgRegs decode_cfg;               /* 0xA00 */
    uint32_t reserved3[128];                /* 0xB00 */
    AXICfgRegs axi_cfg;                     /* 0xD00 */
    uint32_t reserved4[190];                /* 0xD08 */  
    uint32_t routing_addr[ROUTE_ADDR_SIZE]; /* 0x1000 */
    uint32_t token[TOKEN_SIZE];             /* 0x2000 */
} MstPortNode;

#endif
