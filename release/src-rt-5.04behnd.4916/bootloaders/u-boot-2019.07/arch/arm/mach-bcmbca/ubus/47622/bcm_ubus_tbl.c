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
    {UBUS_PORT_ID_BIU,      "BIU",      0,    NULL},
    {UBUS_PORT_ID_PER,      "PER",      0,    NULL},
    {UBUS_PORT_ID_USB,      "USB",      0,    NULL},
    {UBUS_PORT_ID_PCIE0,    "PCIE0",    0,    NULL},
    {UBUS_PORT_ID_PMC,      "PMC",      0,    NULL},
    {UBUS_PORT_ID_SYSPORT,  "SYSPORT0", 0,    NULL},
    {UBUS_PORT_ID_SYSPORT1, "SYSPORT1", 0,    NULL},
    {UBUS_PORT_ID_WIFI,     "WIFI0",    0,    NULL},
    {UBUS_PORT_ID_WIFI1,    "WIFI1",    0,    NULL},
    {UBUS_PORT_ID_SPU,      "SPU",      0,    NULL},
    {-1, 0}
};
