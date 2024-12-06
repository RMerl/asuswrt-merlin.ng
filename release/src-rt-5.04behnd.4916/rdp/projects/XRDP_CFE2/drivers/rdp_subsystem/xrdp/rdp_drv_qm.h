/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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

#ifndef DRV_QM_H_INCLUDED
#define DRV_QM_H_INCLUDED


/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include "xrdp_drv_qm_ag.h"

/* UBUS SLAVE configuration */
#define QM_UBUS_SLV_APB_BASE    0x82100000
#define QM_UBUS_SLV_APB_MASK    0x000c0000
#define QM_UBUS_SLV_VPB_BASE    0x82140000
#define QM_UBUS_SLV_VPB_MASK    0x000c0000
#define QM_UBUS_SLV_DQM_BASE    0x82180000
#define QM_UBUS_SLV_DQM_MASK    0x000c0000


#endif
