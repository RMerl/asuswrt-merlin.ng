
/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
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

/*
 *******************************************************************************
 * File Name  : rdpa_cmd_cpu.c
 *
 * Description: This file contains rdpa cpu object configuration API.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/bcm_log.h>
#include "rdpa_types.h"
#include "rdpa_api.h"
#include "rdpa_ag_cpu.h"
#include "rdpa_drv.h"
#include "rdpa_cmd_cpu.h"
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#include <linux/iqos.h>
#include <ingqos.h>
#endif

#define __BDMF_LOG__

#define CMD_BR_LOG_ID_RDPA_CMD_DRV BCM_LOG_ID_RDPA_CMD_DRV

#if defined(__BDMF_LOG__)
#define CMD_CPU_LOG_ERROR(fmt, args...)			\
    do {						\
        if (bdmf_global_trace_level >= bdmf_trace_level_error)	\
            bdmf_trace("ERR: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_CPU_LOG_INFO(fmt, args...)			\
    do {						\
        if (bdmf_global_trace_level >= bdmf_trace_level_info)	\
            bdmf_trace("INF: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_CPU_LOG_DEBUG(fmt, args...)			\
    do {						\
        if (bdmf_global_trace_level >= bdmf_trace_level_debug)	\
            bdmf_trace("DBG: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#else
#define CMD_CPU_LOG_ERROR(fmt, arg...) BCM_LOG_ERROR(fmt, arg...)
#define CMD_CPU_LOG_INFO(fmt, arg...) BCM_LOG_INFO(fmt, arg...)
#define CMD_CPU_LOG_DEBUG(fmt, arg...) BCM_LOG_DEBUG(fmt, arg...)
#endif

#define INGQOS_RUNNER_MAX_QOS_RULES  255

/*******************************************************************************/
/* global routines                                                             */
/*******************************************************************************/

int rdpa_iq_cpu_get_status(void *iq_param)
{
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM_PON)
    bdmf_object_handle cpu = NULL;
#if !defined(CONFIG_BCM_PON_XRDP)
    bdmf_object_handle wlan0 = NULL;
#endif
    rdpa_cpu_rx_stat_t rxstat;
    bdmf_index idx;

    if (rdpa_cpu_get(rdpa_cpu_host, &cpu))
    {
        CMD_CPU_LOG_ERROR("%s rdpa_cpu_get returned error\n", __FUNCTION__);
        return -1;
    }

    printk("\n----------------IQ Status------------------\n");
    printk(" qidx\trxpkts\tqueued\tdropped\tinterrupts\n");
    printk("-------------------------------------------\n");

    for (idx = 0; idx < RDPA_CPU_MAX_QUEUES; idx++)
    {
        if (rdpa_cpu_rxq_stat_get(cpu, idx, &rxstat) == 0)
        {
            printk(" %d\t%u\t%u\t%u\t%u\n", (int)idx, rxstat.received, rxstat.queued, rxstat.dropped, rxstat.interrupts);
        }
    }
    if (cpu)
        bdmf_put(cpu);

#if !defined(CONFIG_BCM_PON_XRDP)
    if (rdpa_cpu_get(rdpa_cpu_wlan0, &wlan0))
    {
        CMD_CPU_LOG_ERROR("%s rdpa_cpu_get returned error\n", __FUNCTION__);
        return -1;
    }
    for (idx = RDPA_CPU_MAX_QUEUES; idx < (RDPA_CPU_MAX_QUEUES + RDPA_WLAN_MAX_QUEUES); idx++)
    {
        if (rdpa_cpu_rxq_stat_get(wlan0, idx, &rxstat) == 0)
        {
            printk(" %d\t%u\t%u\t%u\t%u\n",  (int)idx, rxstat.received, rxstat.queued, rxstat.dropped, rxstat.interrupts);
        }
    }

    if (wlan0)
        bdmf_put(wlan0);
#endif
#endif
#endif
    return 0;
}

