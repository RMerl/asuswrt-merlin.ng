#ifndef __INGQOS_H_INCLUDED__
#define __INGQOS_H_INCLUDED__

/*
 *
<:copyright-BRCM:2009:DUAL/GPL:standard

   Copyright (c) 2009 Broadcom 
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

/*
 *******************************************************************************
 * File Name : ingqos.h
 *
 *******************************************************************************
 */
#define IQ_VERSION             "v0.1"

#define IQ_VER_STR             IQ_VERSION
#define IQ_MODNAME             "Broadcom Ingress QoS Module "

/* Ingess QoS Character Device */
#define IQ_DRV_MAJOR             3003


#define IQ_HASHTBL_SIZE             64
#define IQ_OVFLTBL_SIZE             64

#define CC_IQ_STATS


typedef struct {
    uint32_t loThresh;
    uint32_t hiThresh;
} thresh_t;


/*
 * CAUTION!!! 
 * It is highly recommended NOT to change the tuning parameters
 * in this file from their default values. Any change may badly affect
 * the performance of the system.
 */

/* It is recommneded to keep the low thresh > 50%        */
/* Ethernet Ingress QoS low and high thresholds as % of Ring size */
#define IQ_ENET_LO_THRESH_PCT       66
#define IQ_ENET_HI_THRESH_PCT       75

/* Ethernet Ingress QoS low and high thresholds as % of Ring size */
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
#define IQ_XTM_LO_THRESH_PCT        66
#define IQ_XTM_HI_THRESH_PCT        75
#endif

/* CMF Fwd Ingress QoS low and high thresholds as % of Ring size */


#endif  /* defined(__INGQOS_H_INCLUDED__) */

