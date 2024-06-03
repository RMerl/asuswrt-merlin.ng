/*
 * <:copyright-BRCM:2020:DUAL/GPL:standard
 * 
 *    Copyright (c) 2020 Broadcom 
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

