/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

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

#ifndef __DHD_WMF_H__
#define __DHD_WMF_H__

#define WMF_DIRTYP_LEN (64)
#define STA_HW_ACC_UNKNOWN  (0) 
#define STA_HW_ACC_ENABLED  (1)
#define STA_HW_ACC_DISABLED (2)
#if defined(DHD_WMF)
int32 dhd_wmf_stall_sta_check_fn(void *wrapper, void *p, uint32 mgrp_ip);
int32 dhd_wmf_forward_fn(void *wrapper, void *p, uint32 mgrp_ip, void *txif, int rt_port);
void *dhd_wmf_get_igsc(void *pdev) ;
void *dhd_wmf_sdbfind(void *pdev,void *mac); 
void *dhd_wmf_get_device(void *dhd_p,int idx);
void dhd_wmf_update_if_stats(void *dhd_p,int ifidx,int wmf_action,unsigned int pktlen,bool frombss);
void dhd_wmf_update_if_uni_stats(void *dhd_p,int ifidx,int pktlen,bool drop);

#endif

#endif /* __DHD_WMF_H__*/
