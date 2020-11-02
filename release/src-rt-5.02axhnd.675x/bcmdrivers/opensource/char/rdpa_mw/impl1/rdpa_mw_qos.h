/*
* <:copyright-BRCM:2015:DUAL/GPL:standard
* 
*    Copyright (c) 2015 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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
