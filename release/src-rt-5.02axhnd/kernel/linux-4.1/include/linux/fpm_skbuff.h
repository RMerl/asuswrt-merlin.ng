/*
* <:copyright-BRCM:2017:DUAL/GPL:standard
* 
*    Copyright (c) 2017 Broadcom 
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
*
*  Created on: May 2017
*  Author: Kosta Sopov, kosta.sopov@broadcom.com
*/

#include <rdp_drv_fpm.h>
#include <linux/skbuff.h>

/*
        FPM data buffer:

                skb->head     skb->data        skb->tail    skb->end
       +--------------+------------+-----------------+-----------+---------+
       | pre-headroom |  headroom  |       data      | tailroom  | endroom |
       +-------------------------------------------------------------------+
                      \-------------- vanila skb ----------------/                        
*/


#define FPM_BUFFER_SIZE (2048)
#define SKB_SHARED_INFO_SIZE_REDUCED SKB_DATA_ALIGN(offsetof(struct skb_shared_info, frags)) /* no fragments allowed */
#define FPM_DATA_PREHEADROOM SKB_DATA_ALIGN(sizeof(struct sk_buff))
#define MAX_DATALEN (FPM_BUFFER_SIZE - FPM_DATA_PREHEADROOM - SKB_SHARED_INFO_SIZE_REDUCED)

int skb_is_fpm_data(struct sk_buff *skb)
{
    return skb->recycle_flags & SKB_RECYCLE_FPM_DATA;
}

/* Refer to the scheme above */
int skb_fpm_preheadroom(struct sk_buff *skb)
{
    return FPM_DATA_PREHEADROOM;
}

/* Refer to the scheme above */
int skb_fpm_endroom(struct sk_buff *skb)
{
    return SKB_SHARED_INFO_SIZE_REDUCED;
}

static void fpm_recycle(void *nbuff_p, unsigned long context, uint32_t flags)
{
    struct sk_buff *skb = nbuff_p;
    uint32_t fpm_num = skb->fpm_num;

    drv_fpm_free_buffer(FPM_BUFFER_SIZE, fpm_num);
}

static void empty_skb_init(struct sk_buff *skb, void *buf, uint32_t size)
{
	struct skb_shared_info *shinfo;

	memset(skb, 0, offsetof(struct sk_buff, tail));
	skb->truesize = SKB_TRUESIZE(size);
	atomic_set(&skb->users, 1);
	skb->head = buf;
	skb->data = buf;
	skb_reset_tail_pointer(skb);
	skb->end = skb->tail + size;
	skb->mac_header = (typeof(skb->mac_header))~0U;
	skb->transport_header = (typeof(skb->transport_header))~0U;

	/* make sure we initialize shinfo sequentially */
	shinfo = skb_shinfo(skb);
	memset(shinfo, 0, offsetof(struct skb_shared_info, dataref));
	atomic_set(&shinfo->dataref, 1);
}

void *drv_fpm_buffer_get_address(uint32_t fpm_bn);

struct sk_buff *skb_fpm_alloc(void)
{
    struct sk_buff *skb;
    uint32_t buff_num, ret;
    void *buff_ptr;

    if ((ret = drv_fpm_alloc_buffer(FPM_BUFFER_SIZE, &buff_num)) != BDMF_ERR_OK)
        return NULL;

    buff_ptr = drv_fpm_buffer_get_address(buff_num);

    if (!(skb = skb_header_alloc()))
        return NULL;

    empty_skb_init(skb, buff_ptr + FPM_DATA_PREHEADROOM, MAX_DATALEN);

	skb->recycle_hook = fpm_recycle;
	skb->recycle_flags = (SKB_DATA_RECYCLE | SKB_RECYCLE_FPM_DATA); 
    skb->fpm_num = buff_num;

    return skb;
}

