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
    {UBUS_PORT_ID_WIFI,    "WIFI0",   0,    NULL,    1},
    {UBUS_PORT_ID_PCIE0,   "PCIE0",   0,    NULL,    1},
    {UBUS_PORT_ID_QM,      "QM",      0,    NULL,    0},
    {UBUS_PORT_ID_DMA0,    "DMA0",    0,    NULL,    0},
    {UBUS_PORT_ID_RQ0,     "RQ0",     0,    NULL,    0},
    {-1, 0}
};
