/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
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

#ifndef _BDMF_SYSB_CHAIN_H_
#define _BDMF_SYSB_CHAIN_H_

#if (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
#include <osl.h>
#endif

static inline int bdmf_sysb_chained_unlink(const bdmf_sysb sysb)
{
#if (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
    if (IS_FKBUFF_PTR(sysb))
        return 0;

    PKTSETCLINK(sysb, NULL);
#endif
    return 0;
}

static inline int bdmf_sysb_is_chained(const bdmf_sysb sysb)
{
#if (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
    if (IS_FKBUFF_PTR(sysb))
        return 0;

    return PKTISCHAINED(sysb);
#else
    return 0;
#endif
}

static inline bdmf_sysb bdmf_sysb_chain_next(const bdmf_sysb sysb)
{
#if (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
    return PKTCLINK(sysb);
#else
    return NULL;
#endif
}

static inline bdmf_sysb bdmf_sysb_chain_link_set(const bdmf_sysb sysb, const bdmf_sysb next)
{
#if (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
    return PKTSETCLINK(sysb, next);
#else
    return NULL;
#endif
}

#endif
