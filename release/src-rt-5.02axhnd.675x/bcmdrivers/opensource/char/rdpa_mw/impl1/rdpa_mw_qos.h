/*
* <:copyright-BRCM:2015:DUAL/GPL:standard
* 
*    Copyright (c) 2015 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :>
*/

#include <rdpa_api.h>
#include "bcmtypes.h"
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include "rdpa_drv.h"


typedef enum{
    RDPA_MW_QOS_TYPE_FC         = 0,
    RDPA_MW_QOS_TYPE_IC         = 1,
    RDPA_MW_QOS_TYPE_MCAST      = 2,
    RDPA_MW_QOS_TYPE_MAX
} rdpa_mw_qos_type;


int rdpa_mw_pkt_based_qos_get(rdpa_traffic_dir dir, 
                            rdpa_mw_qos_type type, 
                            BOOL *enable);

int rdpa_mw_pkt_based_qos_set(rdpa_traffic_dir dir, 
                            rdpa_mw_qos_type type, 
                            BOOL *enable);

struct rdpa_mw_drop_precedence_args {
    rdpa_traffic_dir rdpa_dir;
    rdpadrv_dp_code dp_code;
};

int rdpa_mw_drop_precedence_set(rdpa_traffic_dir dir, rdpadrv_dp_code code);
