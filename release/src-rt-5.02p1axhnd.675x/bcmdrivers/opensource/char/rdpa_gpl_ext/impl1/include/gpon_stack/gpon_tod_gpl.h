/*
* <:copyright-BRCM:2017:GPL/GPL:standard
* 
*    Copyright (c) 2017 Broadcom 
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

#ifndef GPON_TOD_GPL_H_INCLUDED
#define GPON_TOD_GPL_H_INCLUDED

#include <linux/kernel.h>

#ifdef CONFIG_BCM_GPON_TODD

/* Timestamp: IEEE 1588-2008, 5.3.3 */
typedef struct
{
    uint16_t sec_ms;    /* Seconds, MS bits */
    uint32_t sec_ls;    /* Seconds, LS bits */

    uint32_t nsec;      /* Nanoseconds      */
}
gpon_todd_tstamp_t;

typedef void (*gpon_todd_1pps_ctrl_cb_t) (void);

void gpon_todd_set_tod_info(uint32_t sframe_num_ls, uint32_t sframe_num_ms, 
    const gpon_todd_tstamp_t* tstamp_n);
void gpon_todd_get_tod_info(uint32_t* sframe_num_ls, uint32_t* sframe_num_ms,
    gpon_todd_tstamp_t* tstamp_n);
void gpon_todd_reg_1pps_start_cb(gpon_todd_1pps_ctrl_cb_t onepps_start_cb);
void gpon_todd_get_tod(gpon_todd_tstamp_t* tstamp, uint64_t *ts);
void gpon_tod_get_sfc(uint32_t *sfc_ls, uint32_t *sfc_ms);

#endif
#endif 

