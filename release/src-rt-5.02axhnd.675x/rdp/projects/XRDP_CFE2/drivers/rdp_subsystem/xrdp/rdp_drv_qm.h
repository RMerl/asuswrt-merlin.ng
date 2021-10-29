/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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
