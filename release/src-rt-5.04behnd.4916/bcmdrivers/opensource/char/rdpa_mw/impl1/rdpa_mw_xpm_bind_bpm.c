/*
*  <:copyright-BRCM:2022:DUAL/GPL:standard
*
*  Copyright (c) 2022 Broadcom
*  All Rights Reserved
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
*  :>
*/

/*
*******************************************************************************
*
* File Name  : rdpa_mw_xpm_bind_bpm.c
*
* Description: RDPA XPM bind to GBPM (Global Buffer Pool Manager)
*
*******************************************************************************
*/
#if !defined(CONFIG_BCM_BPM_SW_ONLY)
#include <rdpa_api.h>
#include <linux/gbpm.h>


#include "rdp_drv_xpm.h"
#include <rdpa_buffer.h>
#include "bpm.h"

struct rdpa_xpm_bind_bpm_drv_t {
    struct bpm_hw_pool_def_ring_id  map;
    struct bpm_hw_pool_ops          ops;
};

static struct rdpa_xpm_bind_bpm_drv_t rdpa_xpm_bind_bpm_drv;

/* 
 * RDPA Buffer HW BPM Operations Callbacks
 */

static void rdpa_buffer_bpm_free_buf(unsigned long ring_index, void *buf_p)
{
    rdpa_buffer_free(buf_p);
}

static void rdpa_buffer_bpm_free_mult_buf(unsigned long ring_index, uint32_t num, void **buf_p)
{
    rdpa_buffer_free_mult(num, buf_p);
}



static void rdpa_buffer_bpm_free_skb_data_list(unsigned long ring_index, void *head, void *tail, uint32_t num)
{
    rdpa_buffer_free_mult(num, &head);
}




/* 
 * RDPA Buffers
 */


/* 
 * GBPM Callbacks
 */
extern bool (*f_rdpa_buffer_is_hw_buffer)(void *buffer_ptr);


static int rdpa_xpm_bind_bpm_hw_pool_init(void)
{
    int ret = 0;

    memset((void *)&rdpa_xpm_bind_bpm_drv, 0x00, sizeof(rdpa_xpm_bind_bpm_drv));

    // HW BPM Operations Callbacks
    rdpa_xpm_bind_bpm_drv.ops.is_hw_buffer        =  f_rdpa_buffer_is_hw_buffer;
    rdpa_xpm_bind_bpm_drv.ops.free_buf		= rdpa_buffer_bpm_free_buf;
    rdpa_xpm_bind_bpm_drv.ops.free_mult_buf	= rdpa_buffer_bpm_free_mult_buf;
    rdpa_xpm_bind_bpm_drv.ops.free_skb_data_list	= rdpa_buffer_bpm_free_skb_data_list;

    // Register RDPA Buffer HW BPM with GBPM
    ret = gbpm_register_hw_pool_api((void *)&(rdpa_xpm_bind_bpm_drv.ops), (void *)&(rdpa_xpm_bind_bpm_drv.map));
    if (ret) {
        goto exit;
    }

    return 0;

exit:
    return ret;
}

static void rdpa_buffer_bpm_hw_pool_exit(void)
{

    memset((void *)&rdpa_xpm_bind_bpm_drv, 0x00, sizeof(rdpa_xpm_bind_bpm_drv));
}

struct gbpm_hw_buffer_mngr rdpa_buffer_hw_buffer_mngr = {
    .name = "RDPA_XPM",
    .pool_init = rdpa_xpm_bind_bpm_hw_pool_init,
    .pool_exit = rdpa_buffer_bpm_hw_pool_exit,
};

#endif
