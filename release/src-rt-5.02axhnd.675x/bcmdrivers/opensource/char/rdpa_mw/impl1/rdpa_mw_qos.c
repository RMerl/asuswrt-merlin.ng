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

#include "bcm_OS_Deps.h"
#include "bcmenet_common.h"
#include "rdpa_mw_qos.h"

#define PBIT_VAL_MAX 7

#define rdpaMwDebug(fmt, arg...) BCM_LOG_DEBUG(BCM_LOG_ID_RDPA, fmt, ##arg)
#define rdpaMwInfo(fmt, arg...) BCM_LOG_INFO(BCM_LOG_ID_RDPA, fmt, ##arg)
#define rdpaMwError(fmt, arg...) BCM_LOG_ERROR(BCM_LOG_ID_RDPA, fmt, ##arg)


static BOOL pkt_based_qos_en[2][RDPA_MW_QOS_TYPE_MAX];

int rdpa_mw_pkt_based_qos_get(rdpa_traffic_dir dir,
                            rdpa_mw_qos_type type,
                            BOOL *enable)
{
    if (dir > rdpa_dir_us || type >= RDPA_MW_QOS_TYPE_MAX)
    {
        return -1;
    }
    else
    {
        *enable = pkt_based_qos_en[dir][type];
        return 0;
    }
}

EXPORT_SYMBOL(rdpa_mw_pkt_based_qos_get);

int rdpa_mw_pkt_based_qos_set(rdpa_traffic_dir dir,
                            rdpa_mw_qos_type type,
                            BOOL *enable)
{
    if (dir > rdpa_dir_us || type >= RDPA_MW_QOS_TYPE_MAX)
    {
        return -1;
    }
    else
    {
        pkt_based_qos_en[dir][type] = *enable;
        return 0;
    }
}

EXPORT_SYMBOL(rdpa_mw_pkt_based_qos_set);

int rdpa_mw_drop_precedence_set(rdpa_traffic_dir dir, rdpadrv_dp_code dpCode)
{
    bdmf_object_handle system_obj = NULL;
    int rc = 0;
    int i = 0;
    int ret = 0;
    rdpa_dp_key_t dpKey;

    bdmf_lock();

    rc = rdpa_system_get(&system_obj);
    if (rc)
    {
        rdpaMwError("rdpa_system_get() failed");
        bdmf_unlock();
        return -1;
    }

    dpKey.dir = dir;

    for (i = 0; i <= PBIT_VAL_MAX; i++)
    {
        dpKey.pbit = i;
        dpKey.dei = 0;
        rdpa_system_drop_precedence_set(system_obj, &dpKey, 0);
        dpKey.dei = 1;
        rdpa_system_drop_precedence_set(system_obj, &dpKey, 0);
    }

    switch (dpCode)
    {
        case RDPADRV_DP_CODE_DEI:
        {
            /* Drop eligibility = true for packets with DEI value 1. */
            for (i = 0; i <= PBIT_VAL_MAX; i++)
            {
                dpKey.pbit = i;
                dpKey.dei = 1;
                rdpa_system_drop_precedence_set(system_obj, &dpKey, 1);
            }
            break;
        }

        case RDPADRV_DP_CODE_PCP6P2D:
        {
            /* Drop eligibility = true for packets with PBIT 2 or 4. */
            dpKey.pbit = 2;
            dpKey.dei = 0;
            rdpa_system_drop_precedence_set(system_obj, &dpKey, 1);
            dpKey.pbit = 2;
            dpKey.dei = 1;
            rdpa_system_drop_precedence_set(system_obj, &dpKey, 1);
            dpKey.pbit = 4;
            dpKey.dei = 0;
            rdpa_system_drop_precedence_set(system_obj, &dpKey, 1);
            dpKey.pbit = 4;
            dpKey.dei = 1;
            rdpa_system_drop_precedence_set(system_obj, &dpKey, 1);
            break;
        }

        case RDPADRV_DP_CODE_NONE:
        case RDPADRV_DP_CODE_INTERNAL:
        {
            break;
        }

        default:
        {
            rdpaMwError("Drop precedence code %d is not supported",
              dpCode);
            ret = -1;
            break;
        }
    }

    bdmf_put(system_obj);
    bdmf_unlock();

    return ret;
}

EXPORT_SYMBOL(rdpa_mw_drop_precedence_set);
