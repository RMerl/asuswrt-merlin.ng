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

#include "bcm_ubus4.h"
#include "../bcm_ubus_internal.h"

ub_mst_addr_map_t ub_mst_addr_map_tbl[] = {
    {UBUS_PORT_ID_BIU,     "BIU",     0,    NULL,    0},
    {UBUS_PORT_ID_PER,     "PER",     0,    NULL,    0},
    {UBUS_PORT_ID_USB,     "USB",     0,    NULL,    1},
    {UBUS_PORT_ID_PERDMA,  "PERDMA",  0,    NULL,    0},
    {UBUS_PORT_ID_SPU,     "SPU",     0,    NULL,    1},
    {UBUS_PORT_ID_PCIE0,   "PCIE0",   0,    NULL,    1},
    {UBUS_PORT_ID_PCIE2,   "PCIE2",   0,    NULL,    1},
    {UBUS_PORT_ID_PMC,     "PMC",     0,    NULL,    0},
    {UBUS_PORT_ID_QM,      "QM",      0,    NULL,    0},
    {UBUS_PORT_ID_DQM,     "DQM",     0,    NULL,    0},
    {UBUS_PORT_ID_DMA0,    "DMA0",    0,    NULL,    0},
    {UBUS_PORT_ID_DMA1,    "DMA1",    0,    NULL,    0},
    {UBUS_PORT_ID_NATC,    "NATC",    0,    NULL,    0},
    {UBUS_PORT_ID_TOP_BUFF, "TOP_BUFF",    0,    NULL,    0},
    {UBUS_PORT_ID_XRDP_BUFF,"XRDP_BUFF",    0,    NULL,    0},
    {UBUS_PORT_ID_RQ0,     "RQ0",     0,    NULL,    0},
    {UBUS_PORT_ID_RQ1,     "RQ1",     0,    NULL,    0},
    {UBUS_PORT_ID_RQ2,     "RQ2",     0,    NULL,    0},
    {UBUS_PORT_ID_RQ3,     "RQ3",     0,    NULL,    0},
    {-1, 0}
};

ubus_credit_cfg_t ubus_credit_tbl[UBUS_NUM_OF_MST_PORTS][UBUS_MAX_PORT_NUM+2] = {
    { {UBUS_PORT_ID_BIU, -1}, {-1,-1} },
    { {UBUS_PORT_ID_PER, -1}, {UBUS_PORT_ID_MEMC, 1}, {-1,-1} },
    { {UBUS_PORT_ID_USB, -1}, {UBUS_PORT_ID_MEMC, 4}, {-1,-1} },
    { {UBUS_PORT_ID_PERDMA, -1}, {UBUS_PORT_ID_BIU, 0} },
    { {UBUS_PORT_ID_SPU, -1}, {-1,-1} },
    { {UBUS_PORT_ID_PCIE0, -1}, {UBUS_PORT_ID_RQ0, 1}, {UBUS_PORT_ID_RQ1, 1}, {UBUS_PORT_ID_RQ2, 1}, {UBUS_PORT_ID_RQ3, 1}, {-1,-1} },
    { {UBUS_PORT_ID_PCIE2, -1}, {UBUS_PORT_ID_RQ0, 1}, {UBUS_PORT_ID_RQ1, 1}, {UBUS_PORT_ID_RQ2, 1}, {UBUS_PORT_ID_RQ3, 1}, {-1,-1} },
    { {UBUS_PORT_ID_PMC, -1}, {UBUS_PORT_ID_MEMC, 1}, {UBUS_PORT_ID_RQ0, 1}, {UBUS_PORT_ID_RQ1, 1}, {UBUS_PORT_ID_RQ2, 1}, {UBUS_PORT_ID_RQ3, 1}, {-1,-1} },
    { {UBUS_PORT_ID_QM, -1}, {UBUS_PORT_ID_MEMC, 10}, {-1,-1} },
    { {UBUS_PORT_ID_DQM, -1}, {UBUS_PORT_ID_MEMC, 7}, {-1,-1} },
    { {UBUS_PORT_ID_DMA0, -1}, {UBUS_PORT_ID_MEMC, 9}, {-1,-1} },
    { {UBUS_PORT_ID_DMA1, -1}, {UBUS_PORT_ID_MEMC, 9}, {-1,-1} },
    { {UBUS_PORT_ID_NATC, -1}, {UBUS_PORT_ID_BIU, 10}, {-1,-1} },
    { {UBUS_PORT_ID_TOP_BUFF, -1}, {-1,-1} },
    { {UBUS_PORT_ID_XRDP_BUFF, -1}, {-1,-1} },
    { {UBUS_PORT_ID_RQ0, -1}, {UBUS_PORT_ID_MEMC, 8}, {UBUS_PORT_ID_XRDP_VPB, 1}, {UBUS_PORT_ID_RQ0, 1}, {-1,-1} },
    { {UBUS_PORT_ID_RQ1, -1}, {UBUS_PORT_ID_MEMC, 4}, {UBUS_PORT_ID_XRDP_VPB, 1}, {UBUS_PORT_ID_RQ1, 1}, {-1,-1} },
    { {UBUS_PORT_ID_RQ2, -1}, {UBUS_PORT_ID_MEMC, 4}, {UBUS_PORT_ID_XRDP_VPB, 1}, {UBUS_PORT_ID_RQ2, 1}, {-1,-1} },
    { {UBUS_PORT_ID_RQ3, -1}, {UBUS_PORT_ID_MEMC, 4}, {UBUS_PORT_ID_XRDP_VPB, 1}, {UBUS_PORT_ID_RQ3, 1}, {-1,-1} },
};
