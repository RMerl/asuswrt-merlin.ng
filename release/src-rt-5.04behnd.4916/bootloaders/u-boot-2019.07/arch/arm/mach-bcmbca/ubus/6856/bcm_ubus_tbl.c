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

#include "bcm_ubus4.h"
#include "../bcm_ubus_internal.h"

ub_mst_addr_map_t ub_mst_addr_map_tbl[] = {
    {UBUS_PORT_ID_BIU,     "BIU",     0,    NULL},
    {UBUS_PORT_ID_PER,     "PER",     0,    NULL},
    {UBUS_PORT_ID_USB,     "USB",     0,    NULL},
    {UBUS_PORT_ID_PCIE0,   "PCIE0",   0,    NULL},
    {UBUS_PORT_ID_PCIE2,   "PCIE2",   0,    NULL},
    {UBUS_PORT_ID_QM,      "QM",      0,    NULL},
    {UBUS_PORT_ID_DQM,     "DQM",     0,    NULL},
    {UBUS_PORT_ID_DMA0,    "DMA0",    0,    NULL},
    {UBUS_PORT_ID_DMA1,    "DMA1",    0,    NULL},
    {UBUS_PORT_ID_NATC,    "NATC",    0,    NULL},
    {UBUS_PORT_ID_RQ0,     "RQ0",     0,    NULL},
    {UBUS_PORT_ID_RQ1,     "RQ1",     0,    NULL},
    {-1, 0}
};
