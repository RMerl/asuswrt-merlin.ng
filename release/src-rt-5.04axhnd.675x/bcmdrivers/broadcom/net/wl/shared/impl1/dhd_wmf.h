/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
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
