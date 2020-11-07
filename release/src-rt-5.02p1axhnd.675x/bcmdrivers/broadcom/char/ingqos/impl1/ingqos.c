
/*
<:copyright-BRCM:2018:proprietary:standard

   Copyright (c) 2018 Broadcom
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/


/*
 *******************************************************************************
 * File Name  : ingqos.c
 *
 * Description: This file implements the Ingress QoS.
 *
 *******************************************************************************
 */

#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/ip.h>
#include "pktHdr.h"
#include <linux/bcm_log_mod.h>
#include <linux/bcm_log.h>
#include <linux/iqos.h>
#include "iqctl_common.h"
#include "ingqos.h"

/* Hooks for getting/dumping the Ingress QoS status */
iqos_status_hook_t iqos_enet_status_hook_g = NULL;
iqos_status_hook_t iqos_xtm_status_hook_g = NULL;

/* Hooks for getting the current RX DQM queue depth */
iqos_fap_ethRxDqmQueue_hook_t iqos_fap_ethRxDqmQueue_hook_g = NULL;
iqos_fap_xtmRxDqmQueue_hook_t iqos_fap_xtmRxDqmQueue_hook_g = NULL;

/*---- keeping the old define.. might need to change it later ----*/
/*----- Globals -----*/
extern uint32_t iqos_enable_g;
extern uint32_t iqos_cpu_cong_g;
extern spinlock_t iqos_lock_g;

#undef IQ_DECL
#define IQ_DECL(x) #x,

const char *iqctl_ioctl_name[] =
{
	IQ_DECL(IQCTL_IOCTL_SYS)
	IQ_DECL(IQCTL_IOCTL_MAX)
};

const char *iqctl_subsys_name[] =
{
	IQ_DECL(IQCTL_SUBSYS_STATUS)
	IQ_DECL(IQCTL_SUBSYS_KEY)
	IQ_DECL(IQCTL_SUBSYS_KEYMASK)
	IQ_DECL(IQCTL_SUBSYS_HW_ACCEL_CONG_CTRL)
	IQ_DECL(IQCTL_SUBSYS_MAX)
};

const char *iqctl_op_name[] =
{
	IQ_DECL(IQCTL_OP_GET)
	IQ_DECL(IQCTL_OP_SET)
	IQ_DECL(IQCTL_OP_ADD)
	IQ_DECL(IQCTL_OP_REM)
	IQ_DECL(IQCTL_OP_DUMP)
	IQ_DECL(IQCTL_OP_FLUSH)
	IQ_DECL(IQCTL_OP_GETNEXT)
	IQ_DECL(IQCTL_OP_MAX)
};
/*----- end of old define ------------*/

typedef struct {
	uint32_t key_mask;		/* Key mask field */
	iq_key_option_t key_opt;	/* additional option for the key mask */
	uint8_t prio;			/* priority of the key mask */
	atomic_t refcnt;		/* reference/usage count */
} iq_keymask_entry_t;

typedef struct {
	uint32_t key_mask;		/* key mask field */
	iq_key_option_t key_opt;	/* additional option for the key mask */
	uint8_t prio;			/* priority inhereited from key mask */
	uint8_t keymask_idx;		/* index of the keymask which this key is using */
	iq_action_t action;		/* action info if this key is hit */
	iq_key_data_t key_data;		/* data/value for this key */
	int key_size;			/* key size used for hash computation */
	uint8_t key_value[128];		/* key value used for hash computation */
	uint32_t hit_cnt;		/* accumulative hit count */
	atomic_t refcnt;		/* reference count */
} iq_hash_entry_t;

iq_keymask_entry_t keymask_tbl[IQ_KEYMASKTBL_SIZE];
iq_hash_entry_t qos_htbl[IQ_HASHTBL_SIZE];
atomic_t keymask_entry_cnt;
atomic_t hash_entry_cnt;
atomic_t static_hash_entry_cnt;
iq_hw_info_t g_hw_info;

int bcm_iq_add_keymask(uint32_t new_key_mask, iq_key_option_t *key_opt,
		       uint8_t prio)
{
	int i, index_to_add = 0;
	iq_keymask_entry_t *curr;

	if (unlikely(iqos_enable_g != 0) || (atomic_read(&hash_entry_cnt) != 0)) {
		printk(KERN_ERR "Please disable and flush Ingress Qos before "
			"adding a new entry\n");
		return -EPERM;
	}

	if (new_key_mask == 0)
		return -EINVAL;

	/* no more available slot for the new entry */
	if (keymask_tbl[IQ_KEYMASKTBL_SIZE - 1].key_opt.valid == 1)
		return -EPERM;

	key_opt->valid = 1;

	IQOS_LOCK_BH();
	for (i = 0; i < IQ_KEYMASKTBL_SIZE; i++) {
		curr = &keymask_tbl[i];
		if ((new_key_mask == curr->key_mask) &&
		    (key_opt->word == curr->key_opt.word) &&
		    (key_opt->offset0_mask == curr->key_opt.offset0_mask) &&
		    (key_opt->offset1_mask == curr->key_opt.offset1_mask)) {
			IQOS_UNLOCK_BH();
			printk(KERN_ERR "entry with the same fields exists\n");
			return -EEXIST;
		}

		if (curr->prio == prio) {
			IQOS_UNLOCK_BH();
			printk(KERN_ERR "entry with the same priority exists\n");
			return -EEXIST;
		}

		if ((curr->key_opt.valid == 1) && (curr->prio < prio)) {
			index_to_add = i;
			break;
		}

		if (curr->key_opt.valid == 0) {
			if (index_to_add == 0)
				index_to_add = i;
			break;
		}
	}

	/* copy all the table from index#i to #(IQ_KEYMASKTBL_SIZE - 2)
	 * to index#i+1 to #(IQ_KEYMASKTBL_SIZE - 1) */
	memmove(&keymask_tbl[index_to_add + 1], &keymask_tbl[index_to_add],
		sizeof(iq_keymask_entry_t) *
		       (IQ_KEYMASKTBL_SIZE - index_to_add - 1));

	curr = &keymask_tbl[index_to_add];
	keymask_tbl[index_to_add].key_mask = new_key_mask;
	keymask_tbl[index_to_add].key_opt.word = key_opt->word;
	keymask_tbl[index_to_add].key_opt.offset0_mask = key_opt->offset0_mask;
	keymask_tbl[index_to_add].key_opt.offset1_mask = key_opt->offset1_mask;
	keymask_tbl[index_to_add].prio = prio;
	atomic_set(&keymask_tbl[index_to_add].refcnt, 0);
	atomic_inc(&keymask_entry_cnt);
	IQOS_UNLOCK_BH();

	return 0;
}
EXPORT_SYMBOL(bcm_iq_add_keymask);

/* the following function is called with lock obtained and all the checks
 * have been done, there is no return error, everything should work */
static void __bcm_iq_delete_keymask_by_index(uint32_t index)
{
	/* this will copy entire table from index+1 to IQ_KEYMASKTBL_SIZE - 1
	 * to location index */
	memmove(&keymask_tbl[index], &keymask_tbl[index + 1],
		sizeof(iq_keymask_entry_t) * (IQ_KEYMASKTBL_SIZE - index - 1));

	memset(&keymask_tbl[IQ_KEYMASKTBL_SIZE - 1], 0x0,
	       sizeof(iq_keymask_entry_t));
	atomic_dec(&keymask_entry_cnt);
}

int bcm_iq_delete_keymask_by_index(uint32_t index)
{
	if (index >= IQ_KEYMASKTBL_SIZE)
		return -ERANGE;

	if (keymask_tbl[index].key_opt.valid == 0)
		return -EINVAL;

	if (atomic_read(&keymask_tbl[index].refcnt) != 0)
		return -EINVAL;

	IQOS_LOCK_BH();
	__bcm_iq_delete_keymask_by_index(index);
	IQOS_UNLOCK_BH();

	return 0;
}
EXPORT_SYMBOL(bcm_iq_delete_keymask_by_index);

static int iq_find_keymask(uint32_t key_mask, iq_key_option_t *key_opt)
{
	int i;
	iq_keymask_entry_t *curr;

	key_opt->valid = 1;
	for (i = 0; i < atomic_read(&keymask_entry_cnt); i++) {
		curr = &keymask_tbl[i];
		if ((curr->key_opt.valid == 1) && (curr->key_mask == key_mask) &&
		    (curr->key_opt.word == key_opt->word) &&
		    (curr->key_opt.offset0_mask == key_opt->offset0_mask) &&
		    (curr->key_opt.offset1_mask == key_opt->offset1_mask))
			return i;
	}
	return -ENOENT;
}

int bcm_iq_delete_keymask(uint32_t key_mask, iq_key_option_t *key_opt)
{
	int keymask_idx;

	if (key_opt == NULL)
		return -EINVAL;

	keymask_idx = iq_find_keymask(key_mask, key_opt);
	if (keymask_idx < 0)
		return keymask_idx;

	IQOS_LOCK_BH();
	__bcm_iq_delete_keymask_by_index(keymask_idx);
	IQOS_UNLOCK_BH();

	return 0;
}
EXPORT_SYMBOL(bcm_iq_delete_keymask);

static uint16_t iq_hash_compute_crc(uint8_t *buffer, int size)
{
	/* compute crc32/16 and return. currently using the existing crc16 */
	return (_crc16ccitt(buffer, size) & (IQ_HASHTBL_SIZE - 1));
}

/* standard implementation for iq_key_data_t based */
/* return the size of this field */
static int iq_key_gen_field(iq_key_field_t key_type, iq_key_option_t *key_opt,
			    iq_key_data_t *key_data, uint8_t *key_buffer,
			    int curr_offset)
{
	uint8_t *target_ptr = key_buffer + curr_offset;
	uint32_t *temp_ptr, start_offset = 0;

	switch (key_type) {
	case IQ_KEY_FIELD_INGRESS_DEVICE:
		// FIXME! TBD
		return 1;
	case IQ_KEY_FIELD_SRC_MAC:
		memcpy(target_ptr, key_data->src_mac, BLOG_ETH_ADDR_LEN);
		return BLOG_ETH_ADDR_LEN;
	case IQ_KEY_FIELD_DST_MAC:
		memcpy(target_ptr, key_data->dst_mac, BLOG_ETH_ADDR_LEN);
		return BLOG_ETH_ADDR_LEN;
	case IQ_KEY_FIELD_ETHER_TYPE:
		memcpy(target_ptr, &key_data->eth_type, 2);
		return 2;
	case IQ_KEY_FIELD_OUTER_VID:
		memcpy(target_ptr, &key_data->outer_vid, 2);
		return 2;
	case IQ_KEY_FIELD_OUTER_PBIT:
		memcpy(target_ptr, &key_data->outer_pbit, 1);
		return 1;
	case IQ_KEY_FIELD_INNER_VID:
		memcpy(target_ptr, &key_data->inner_vid, 2);
		return 2;
	case IQ_KEY_FIELD_INNER_PBIT:
		memcpy(target_ptr, &key_data->inner_pbit, 1);
		return 1;
	case IQ_KEY_FIELD_L2_PROTO:
		memcpy(target_ptr, &key_data->l2_proto, 2);
		return 2;
	case IQ_KEY_FIELD_L3_PROTO:
		memcpy(target_ptr, &key_data->l3_proto, 2);
		return 2;
	case IQ_KEY_FIELD_IP_PROTO:
		*target_ptr = key_data->ip_proto;
		return 1;
	case IQ_KEY_FIELD_SRC_IP:
		if (key_data->is_ipv6) {
			/* IPv6 */
			memcpy(target_ptr, key_data->src_ip, 16);
			return 16;
		} else {
			memcpy(target_ptr, key_data->src_ip, 4);
			return 4;
		}
	case IQ_KEY_FIELD_DST_IP:
		if (key_data->is_ipv6) {
			/* IPv6 */
			memcpy(target_ptr, key_data->dst_ip, 16);
			return 16;
		} else {
			memcpy(target_ptr, key_data->dst_ip, 4);
			return 4;
		}
	case IQ_KEY_FIELD_DSCP:
		memcpy(target_ptr, &key_data->dscp, 1);
		return 1;
	case IQ_KEY_FIELD_IPV6_FLOW_LABEL:
		memcpy(target_ptr, &key_data->flow_label, 4);
		return 4;
	case IQ_KEY_FIELD_SRC_PORT:
		memcpy(target_ptr, &key_data->l4_src_port, 2);
		return 2;
	case IQ_KEY_FIELD_DST_PORT:
		memcpy(target_ptr, &key_data->l4_dst_port, 2);
		return 2;
	case IQ_KEY_FIELD_OFFSET_0:
		if (key_opt->offset0_type == IQ_KEY_OFFSET_L2)
			start_offset = key_data->l2_offset;
		else if (key_opt->offset0_type == IQ_KEY_OFFSET_L3)
			start_offset = key_data->l3_offset;
		else if (key_opt->offset0_type == IQ_KEY_OFFSET_L4)
			start_offset = key_data->l4_offset;
		else
			return -EINVAL;

		start_offset += key_opt->offset0_start;

		memcpy(target_ptr, &key_data->packet_cache[start_offset], key_opt->offset0_size);
		if (key_opt->offset0_mask != 0xffffffff) {
			temp_ptr = (uint32_t *)target_ptr;
			*temp_ptr &= key_opt->offset0_mask;
		}
		return key_opt->offset0_size;
	case IQ_KEY_FIELD_OFFSET_1:
		if (key_opt->offset1_type == IQ_KEY_OFFSET_L2)
			start_offset = key_data->l2_offset;
		else if (key_opt->offset1_type == IQ_KEY_OFFSET_L3)
			start_offset = key_data->l3_offset;
		else if (key_opt->offset1_type == IQ_KEY_OFFSET_L4)
			start_offset = key_data->l4_offset;
		else
			return -EINVAL;
		start_offset += key_opt->offset1_start;

		memcpy(target_ptr, &key_data->packet_cache[start_offset], key_opt->offset1_size);
		if (key_opt->offset1_mask != 0xffffffff) {
			temp_ptr = (uint32_t *)target_ptr;
			*temp_ptr &= key_opt->offset1_mask;
		}
		return key_opt->offset1_size;
	default:
		/* shouldn't reach here */
		WARN_ON(1);
		return -EINVAL;
	}
}

static int iq_hash_gen_key_value(uint32_t key_mask, iq_key_option_t *key_opt,
				 iq_key_data_t *key_data, uint8_t *buffer,
				 int buffer_size)
{
	int key_offset, rc, i;
	uint32_t curr_mask;
	uint32_t key_mask_buffer = key_mask;

	/* TODO!! implement something that check the buffer size is big enough
	 * to hold the value */
	memcpy(buffer, &key_mask_buffer, sizeof(uint32_t));
	key_offset = 4;
	for (i = 0; i < IQ_KEY_FIELD_MAX; i++) {
		curr_mask = 0x1 << i;
		if (key_mask & curr_mask) {
			rc = iq_key_gen_field(i, key_opt, key_data, buffer,
					      key_offset);
			if (unlikely(rc <= 0))
				return rc;
			key_offset += rc;
		}
	}

	return key_offset;
}

static uint16_t iq_hash(uint32_t key_mask, iq_key_option_t *key_opt,
			iq_key_data_t *key_data)
{
	uint8_t key_buffer[128];
	int key_value_size;

	key_value_size = iq_hash_gen_key_value(key_mask, key_opt, key_data,
					       key_buffer, 128);
	if (key_value_size <= 0)
		return 0xffff;

	return iq_hash_compute_crc(key_buffer, key_value_size);
}

/* return 1; if iq_hash_a contains matching info with key_mask_b, key_opt_b,
 * and key_data_b... otherwise, return 0 */
static int iq_compare_key(iq_hash_entry_t *iq_hash_a, uint32_t key_mask_b,
			  iq_key_option_t *key_opt_b, iq_key_data_t *key_data_b)
{
	uint8_t key_buffer_a[128], key_buffer_b[128];
	int key_size_a, key_size_b;

	if ((iq_hash_a->key_mask != key_mask_b) ||
	    (iq_hash_a->key_opt.word != key_opt_b->word))
		return 0;

	key_size_a = iq_hash_gen_key_value(key_mask_b, key_opt_b,
					   &iq_hash_a->key_data,
					   key_buffer_a, 128);
	key_size_b = iq_hash_gen_key_value(key_mask_b, key_opt_b,
					   key_data_b, key_buffer_b, 128);
	if (unlikely((key_size_a <= 0) ||
		     (key_size_b <= 0) ||
		     (key_size_a != key_size_b)))
		return 0;

	if (memcmp(key_buffer_a, key_buffer_b, key_size_a) != 0)
		return 0;

	/* all matched */
	return 1;
}

/* return 1; if action is same
 * otherwise, return 0 */
static int iq_compare_action(iq_action_t *action1, iq_action_t *action2)
{

	if ((action1->type == action2->type) &&
	    (action1->value == action2->value))
		return 1;

	return 0;
		
}

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
/* implementation for Blog based */
/* return the size of this field */
static int iq_key_gen_field_blog(iq_key_field_t key_type,
				 iq_key_option_t *key_opt,
				 struct blog_t *blog, uint8_t *key_buffer,
				 int curr_offset)
{
	uint8_t *target_ptr = key_buffer + curr_offset;

	switch (key_type) {
	case IQ_KEY_FIELD_INGRESS_DEVICE:
		// FIXME! TBD
		return 1;
	case IQ_KEY_FIELD_SRC_MAC:
		memcpy(target_ptr, blog->src_mac.u8, BLOG_ETH_ADDR_LEN);
		return BLOG_ETH_ADDR_LEN;
	case IQ_KEY_FIELD_DST_MAC:
		memcpy(target_ptr, blog->dst_mac.u8, BLOG_ETH_ADDR_LEN);
		return BLOG_ETH_ADDR_LEN;
	case IQ_KEY_FIELD_ETHER_TYPE:
		memcpy(target_ptr, &blog->eth_type, 2);
		return 2;
	case IQ_KEY_FIELD_OUTER_VID:
	case IQ_KEY_FIELD_OUTER_PBIT:
	case IQ_KEY_FIELD_INNER_VID:
	case IQ_KEY_FIELD_INNER_PBIT:
		// FIXME! TBD
		return 2;
	case IQ_KEY_FIELD_L2_PROTO:
		// FIXME! TBD
		return 2;
	case IQ_KEY_FIELD_L3_PROTO:
		// FIXME! TBD
		return 2;
	case IQ_KEY_FIELD_IP_PROTO:
		*target_ptr = blog->key.protocol;
		return 1;
	case IQ_KEY_FIELD_SRC_IP:
		if ((blog->rx.info.bmap.PLD_IPv6) && !(RX_IP4in6(blog))) {
			/* IPv6 */
			memcpy(target_ptr, blog->tupleV6.saddr.p8, 16);
			return 16;
		} else {
			memcpy(target_ptr, &blog->rx.tuple.saddr, 4);
			return 4;
		}
	case IQ_KEY_FIELD_DST_IP:
		if ((blog->rx.info.bmap.PLD_IPv6) && !(RX_IP4in6(blog))) {
			/* IPv6 */
			memcpy(target_ptr, blog->tupleV6.daddr.p8, 16);
			return 16;
		} else {
			memcpy(target_ptr, &blog->rx.tuple.daddr, 4);
			return 4;
		}
	case IQ_KEY_FIELD_DSCP:
		// FIXME! TBD
		return 2;
	case IQ_KEY_FIELD_IPV6_FLOW_LABEL:
		// FIXME! TBD
		return 2;
	case IQ_KEY_FIELD_SRC_PORT:
		/* IPv6 */
		if ((blog->rx.info.bmap.PLD_IPv6) && !(RX_IP4in6(blog)))
			memcpy(target_ptr, &blog->tupleV6.port.source, 2);
		else
			memcpy(target_ptr, &blog->rx.tuple.port.source, 2);
		return 2;
	case IQ_KEY_FIELD_DST_PORT:
		/* IPv6 */
		if ((blog->rx.info.bmap.PLD_IPv6) && !(RX_IP4in6(blog)))
			memcpy(target_ptr, &blog->tupleV6.port.dest, 2);
		else
			memcpy(target_ptr, &blog->rx.tuple.port.dest, 2);
		return 2;
	case IQ_KEY_FIELD_OFFSET_0:
		// FIXME! TBD
		return 2;
	case IQ_KEY_FIELD_OFFSET_1:
		// FIXME! TBD
		return 2;
	default:
		/* shouldn't reach here */
		WARN_ON(1);
		return -EINVAL;
	}
}

static int iq_hash_gen_key_value_blog(uint32_t key_mask,
				      iq_key_option_t *key_opt,
				      struct blog_t *blog,
				      uint8_t *buffer, int buffer_size)
{
	int key_offset, rc, i;
	uint32_t curr_mask;
	uint32_t key_mask_buffer = key_mask;

	/* TODO!! implement something that check the buffer size is big enough
	 * to hold the value */
	memcpy(buffer, &key_mask_buffer, sizeof(uint32_t));
	key_offset = 4;
	for (i = 0; i < IQ_KEY_FIELD_MAX; i++) {
		curr_mask = 0x1 << i;
		if (key_mask & curr_mask) {
			rc = iq_key_gen_field_blog(i, key_opt, blog, buffer,
					 	   key_offset);
			if (unlikely(rc <= 0))
				return rc;
			key_offset += rc;

			/* key generation will deal all the VLAN together */
			if ((i >= IQ_KEY_FIELD_OUTER_VID) &&
			    (i <= IQ_KEY_FIELD_INNER_PBIT))
				i = IQ_KEY_FIELD_INNER_PBIT;
		}
	}

	return key_offset;
}

#if 0	/* FIXME!! may not be needed */
static uint16_t iq_hash_blog(uint32_t key_mask, iq_key_option_t *key_opt,
			     struct blog_t *blog)
{
	uint8_t key_buffer[128];
	int key_value_size;

	key_value_size = iq_hash_gen_key_value_blog(key_mask, key_opt, blog,
						    key_buffer, 128);
	if (key_value_size <= 0)
		return 0xffff;

	return iq_hash_compute_crc(key_buffer, key_value_size);
}
#endif
#endif

static int iq_add_hw_entry(iq_hash_entry_t *p_iq_hash)
{
	uint32_t key_mask = p_iq_hash->key_mask;
	iq_param_t iq_param;

	if (g_hw_info.registered == 0)
		return -EPERM;

	if ((key_mask & g_hw_info.mask_capability) != key_mask)
		return -EINVAL;

	/* if HW is disabled, don't add the entry to hardware */
	if (unlikely(g_hw_info.enabled == 0))
		return 0;

	iq_param.key_mask = key_mask;
	iq_param.prio = p_iq_hash->prio;
	memcpy(&iq_param.key_opt, &p_iq_hash->key_opt, sizeof(iq_key_option_t));
	memcpy(&iq_param.key_data, &p_iq_hash->key_data, sizeof(iq_key_data_t));
	memcpy(&iq_param.action, &p_iq_hash->action, sizeof(iq_action_t));

	return g_hw_info.add_entry((void *)&iq_param);
}

int bcm_iq_add_entry(uint32_t key_mask, iq_key_option_t *key_opt,
		     iq_key_data_t *key_data, iq_action_t *action)
{
	int keymask_idx, i;
	uint16_t hash_value;
	iq_hash_entry_t *p_iq_hash;
	uint8_t key_buffer[128];
	int key_value_size;

	key_opt->valid = 1;

	keymask_idx = iq_find_keymask(key_mask, key_opt);
	if (keymask_idx < 0)
		return -EINVAL;

	key_value_size = iq_hash_gen_key_value(key_mask, key_opt, key_data,
					       key_buffer, 128);
	if (key_value_size <= 0)
		return -EINVAL;

	hash_value = iq_hash_compute_crc(key_buffer, key_value_size);

	IQOS_LOCK_BH();
	for (i = 0; i < IQ_HASH_BIN_SIZE; i++) {
		p_iq_hash = &qos_htbl[hash_value + i];
		if (p_iq_hash->action.valid == 0) {
			memcpy(&p_iq_hash->key_data, key_data, sizeof(iq_key_data_t));
			p_iq_hash->action.word = action->word;
			p_iq_hash->action.valid = 1;
			p_iq_hash->key_mask = key_mask;
			p_iq_hash->key_opt.word = key_opt->word;
			p_iq_hash->key_opt.offset0_mask = key_opt->offset0_mask;
			p_iq_hash->key_opt.offset1_mask = key_opt->offset1_mask;
			p_iq_hash->prio = keymask_tbl[keymask_idx].prio;
			p_iq_hash->keymask_idx = keymask_idx;
			p_iq_hash->hit_cnt = 0;
			p_iq_hash->key_size = key_value_size;
			memcpy(p_iq_hash->key_value, key_buffer, key_value_size);
			atomic_set(&p_iq_hash->refcnt, 1);
			atomic_inc(&keymask_tbl[keymask_idx].refcnt);

			if (p_iq_hash->action.is_static == 1)
				atomic_inc(&static_hash_entry_cnt);
			else
				atomic_inc(&hash_entry_cnt);

			/* adding entry to hardware */
			iq_add_hw_entry(p_iq_hash);
			IQOS_UNLOCK_BH();

			return 0;
		} else {
			/* check for collison, if the new and existing match,
			 * then return entry exists, or else move on */
			if (iq_compare_key(p_iq_hash, key_mask, key_opt, key_data)) {
				if (iq_compare_action(&(p_iq_hash->action), action)) {
					if ((p_iq_hash->action.is_static != 1) &&
					    (action->is_static == 1)) {
						/* upgrade entry to static */
						p_iq_hash->action.is_static = 1;
						atomic_inc(&static_hash_entry_cnt);
						atomic_dec(&hash_entry_cnt);
					}
					atomic_inc(&p_iq_hash->refcnt);
					IQOS_UNLOCK_BH();
					return 0;
				} else {
					IQOS_UNLOCK_BH();
					return -EINVAL;
				}
			}
		}
	}

	IQOS_UNLOCK_BH();
	/* if reaching here, it means we are not able to add the key */
	return -EPERM;
}
EXPORT_SYMBOL(bcm_iq_add_entry);

static int iq_delete_hw_entry(iq_hash_entry_t *p_iq_hash)
{
	uint32_t key_mask = p_iq_hash->key_mask;
	iq_param_t iq_param;

	if (g_hw_info.registered == 0)
		return -EPERM;

	if ((key_mask & g_hw_info.mask_capability) != key_mask)
		return -EINVAL;

	/* if HW is disabled, don't add the entry to hardware */
	if (unlikely(g_hw_info.enabled == 0))
		return 0;

	iq_param.key_mask = key_mask;
	iq_param.prio = p_iq_hash->prio;
	memcpy(&iq_param.key_opt, &p_iq_hash->key_opt, sizeof(iq_key_option_t));
	memcpy(&iq_param.key_data, &p_iq_hash->key_data, sizeof(iq_key_data_t));
	memcpy(&iq_param.action, &p_iq_hash->action, sizeof(iq_action_t));

	return g_hw_info.delete_entry((void *)&iq_param);
}

int bcm_iq_delete_entry(uint32_t key_mask, iq_key_option_t *key_opt,
			iq_key_data_t *key_data)
{
	int i;
	uint16_t hash_value;
	iq_hash_entry_t *p_iq_hash;

	/* always set the valid bit to 1, because this would ensure it
	 * matches with the existing valid entry */
	key_opt->valid = 1;

	hash_value = iq_hash(key_mask, key_opt, key_data);
	if (hash_value == 0xffff)
		return -EINVAL;

	IQOS_LOCK_BH();
	for (i = 0; i < IQ_HASH_BIN_SIZE; i++) {
		p_iq_hash = &qos_htbl[hash_value + i];
		if ((p_iq_hash->action.valid == 1) &&
		    (iq_compare_key(p_iq_hash, key_mask, key_opt, key_data))) {

			/* found match, decrement the refcnt, delete the
			 * entry only when refcnt is <= 0 */
			atomic_dec(&p_iq_hash->refcnt);

			if (atomic_read(&p_iq_hash->refcnt) > 0) {
				IQOS_UNLOCK_BH();
				return 0;
			}

			/* delete from the HW accelerator */
			iq_delete_hw_entry(p_iq_hash);

			/* decrement the refcnt in keymask */
			atomic_dec(&keymask_tbl[p_iq_hash->keymask_idx].refcnt);

			if (p_iq_hash->action.is_static == 1)
				atomic_dec(&static_hash_entry_cnt);
			else
				atomic_dec(&hash_entry_cnt);

			memset(p_iq_hash, 0x0, sizeof(iq_hash_entry_t));
			IQOS_UNLOCK_BH();
			return 0;
		}
	}

	/* entry not found */
	IQOS_UNLOCK_BH();
	return -ENOENT;
}
EXPORT_SYMBOL(bcm_iq_delete_entry);

static int __bcm_iq_check(iq_key_data_t *key_data, uint32_t key_mask,
			  iq_action_t *action, int f_enable_bypass)
{
	int i, j;
	iq_keymask_entry_t *curr_key;
	uint8_t key_buffer_dst[128];
	int key_size_dst;
	uint16_t hash_value;
	iq_hash_entry_t *curr_hash;

	/* TODO!! check if we also disable software ingress QoS in this case.
	 * previous implementation, SW check wasn't disabled with iqos disable */
	if (unlikely((iqos_enable_g == 0) && (f_enable_bypass == 0)))
		return -1;

	IQOS_LOCK_BH();
	for (i = 0; i < atomic_read(&keymask_entry_cnt); i++) {
		curr_key = &keymask_tbl[i];
		if (unlikely(curr_key->key_opt.valid == 0))
			break;

		if (unlikely(atomic_read(&curr_key->refcnt) == 0))
			continue;

		if ((key_mask != 0) &&
		    ((curr_key->key_mask & key_mask) != curr_key->key_mask))
			continue;

		key_size_dst = iq_hash_gen_key_value(curr_key->key_mask,
					&curr_key->key_opt, key_data,
					key_buffer_dst, 128);
		if (unlikely(key_size_dst <= 0))
			continue;

		hash_value = iq_hash_compute_crc(key_buffer_dst, key_size_dst);

		for (j = 0; j < IQ_HASH_BIN_SIZE; j++) {
			curr_hash = &qos_htbl[hash_value + j];

			/* checking if the hit is really a hit by making sure
			 * 1) hash is really valid, and keymask used matches
			 *    with the one stored in hash
			 * 2) key value generated based on the hash matches
			 *    with the incoming key_data */
			if (unlikely((curr_hash->action.valid == 0) ||
				     (curr_hash->key_mask != curr_key->key_mask) ||
				     (curr_hash->key_opt.word != curr_key->key_opt.word)))
				continue;

			if (unlikely(curr_hash->key_size != key_size_dst))
				continue;

			if (unlikely(memcmp(curr_hash->key_value, key_buffer_dst,
					    key_size_dst) != 0))
				continue;

			goto iq_check_entry_found;
		}
	}

	/* if reach here, means no match is found */
	IQOS_UNLOCK_BH();
	return -1;

iq_check_entry_found:

	/* increment the SW hit count and return the action info */
	curr_hash->hit_cnt++;
	action->word = curr_hash->action.word;
	IQOS_UNLOCK_BH();

	return 0;
}

int bcm_iq_check(iq_key_data_t *key_data, iq_action_t *action)
{
	return __bcm_iq_check(key_data, 0, action, 0);
}
EXPORT_SYMBOL(bcm_iq_check);

int bcm_iq_check_with_mask(iq_key_data_t *key_data, uint32_t key_mask,
			   iq_action_t *action)
{
	return __bcm_iq_check(key_data, key_mask, action, 0);
}
EXPORT_SYMBOL(bcm_iq_check_with_mask);

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
int bcm_iq_check_blog(struct blog_t *blog, iq_action_t *action)
{
	int i, j;
	iq_keymask_entry_t *curr_key;
	uint8_t key_buffer_dst[128];
	int key_size_dst;
	uint16_t hash_value;
	iq_hash_entry_t *curr_hash;

	/* TODO!! check if we also disable software ingress QoS in this case.
	 * previous implementation, SW check wasn't disabled with iqos disable */
	if (unlikely(iqos_enable_g == 0))
		return -1;

	for (i = 0; i < atomic_read(&keymask_entry_cnt); i++) {
		curr_key = &keymask_tbl[i];
		if (unlikely(curr_key->key_opt.valid == 0))
			break;

		if (unlikely(atomic_read(&curr_key->refcnt) == 0))
			continue;

		key_size_dst = iq_hash_gen_key_value_blog(curr_key->key_mask,
					&curr_key->key_opt, blog,
					key_buffer_dst, 128);
		if (unlikely(key_size_dst <= 0))
			continue;

		hash_value = iq_hash_compute_crc(key_buffer_dst, key_size_dst);

		for (j = 0; j < IQ_HASH_BIN_SIZE; j++) {
			curr_hash = &qos_htbl[hash_value + j];

			/* checking if the hit is really a hit by making sure
			 * 1) hash is really valid, and keymask used matches
			 *    with the one stored in hash
			 * 2) key value generated based on the hash matches
			 *    with the incoming blog */
			if (unlikely((curr_hash->action.valid == 0) ||
				     (curr_hash->key_mask != curr_key->key_mask) ||
				     (curr_hash->key_opt.word != curr_key->key_opt.word)))
				continue;

			if (unlikely(curr_hash->key_size != key_size_dst))
				continue;

			if (unlikely(memcmp(curr_hash->key_value, key_buffer_dst,
					    key_size_dst) != 0))
				continue;

			goto iq_check_blog_found;
		}
	}

	/* if reach here, means no match is found */
	return -1;

iq_check_blog_found:

	curr_hash->hit_cnt++;
	// FIXME!! TODO!! implement me! the action!!
	// temporary just return the action
	action->word = curr_hash->action.word;
	return 0;
}
EXPORT_SYMBOL(bcm_iq_check_blog);
#endif

int bcm_iq_check_skb(struct sk_buff *skb, iq_action_t *action)
{
	/* FIXME!! is this needed? if so, travese skb to form iq_key_data_t,
	 * and call iq_check */
	return 0;
}
EXPORT_SYMBOL(bcm_iq_check_skb);

/*------------- new/updated iqos module in kernel ------------------*/

/* converting kernel's IQOS keymask field to bcm_ingqos field definition */
static uint32_t iqos_keymask_convert(uint32_t iqos_field_mask)
{
	uint32_t keymask = 0;
	int i;

	for (i = 0; i < IQOS_FIELD_MAX; i++) {
		if (iqos_field_mask & (0x1 << i)) {
			switch (i) {
			case IQOS_FIELD_INGRESS_DEVICE:
				keymask |= IQ_KEY_MASK_INGRESS_DEVICE;
				break;
			case IQOS_FIELD_SRC_MAC:
				keymask |= IQ_KEY_MASK_SRC_MAC;
				break;
			case IQOS_FIELD_DST_MAC:
				keymask |= IQ_KEY_MASK_DST_MAC;
				break;
			case IQOS_FIELD_ETHER_TYPE:
				keymask |= IQ_KEY_MASK_ETHER_TYPE;
				break;
			case IQOS_FIELD_OUTER_VID:
				keymask |= IQ_KEY_MASK_OUTER_VID;
				break;
			case IQOS_FIELD_OUTER_PBIT:
				keymask |= IQ_KEY_MASK_OUTER_PBIT;
				break;
			case IQOS_FIELD_INNER_VID:
				keymask |= IQ_KEY_MASK_INNER_VID;
				break;
			case IQOS_FIELD_INNER_PBIT:
				keymask |= IQ_KEY_MASK_INNER_PBIT;
				break;
			case IQOS_FIELD_L2_PROTO:
				keymask |= IQ_KEY_MASK_L2_PROTO;
				break;
			case IQOS_FIELD_L3_PROTO:
				keymask |= IQ_KEY_MASK_L3_PROTO;
				break;
			case IQOS_FIELD_IP_PROTO:
				keymask |= IQ_KEY_MASK_IP_PROTO;
				break;
			case IQOS_FIELD_SRC_IP:
				keymask |= IQ_KEY_MASK_SRC_IP;
				break;
			case IQOS_FIELD_DST_IP:
				keymask |= IQ_KEY_MASK_DST_IP;
				break;
			case IQOS_FIELD_DSCP:
				keymask |= IQ_KEY_MASK_DSCP;
				break;
			case IQOS_FIELD_IPV6_FLOW_LABEL:
				keymask |= IQ_KEY_MASK_IPV6_FLOW_LABEL;
				break;
			case IQOS_FIELD_SRC_PORT:
				keymask |= IQ_KEY_MASK_SRC_PORT;
				break;
			case IQOS_FIELD_DST_PORT:
				keymask |= IQ_KEY_MASK_DST_PORT;
				break;
			case IQOS_FIELD_OFFSET_0:
				keymask |= IQ_KEY_MASK_OFFSET_0;
				break;
			case IQOS_FIELD_OFFSET_1:
				keymask |= IQ_KEY_MASK_OFFSET_1;
				break;
			default:
				printk(KERN_WARNING "%s:Unknown field detected "
				       "%d\n", __func__, i);
				break;
			}
		}
	}
	return keymask;
}

int iq_add_keymask(iqos_param_t *param)
{
	uint32_t keymask;
	iq_key_option_t key_opt;

	key_opt.word = 0;
	key_opt.offset0_mask = 0;
	key_opt.offset1_mask = 0;
	keymask = iqos_keymask_convert(param->field_mask);

	if (param->field_mask & (0x1 << IQOS_FIELD_OFFSET_0)) {
		if (param->offset0.type == IQOS_OFFSET_TYPE_L2)
			key_opt.offset0_type = IQ_KEY_OFFSET_L2;
		else if (param->offset0.type == IQOS_OFFSET_TYPE_L3)
			key_opt.offset0_type = IQ_KEY_OFFSET_L3;
		else /* if (param->offset0.type == IQOS_OFFSET_TYPE_L4) */
			key_opt.offset0_type = IQ_KEY_OFFSET_L4;

		key_opt.offset0_start = param->offset0.start;
		key_opt.offset0_size = param->offset0.size;
		key_opt.offset0_mask = param->offset0.mask;
	}

	if (param->field_mask & (0x1 << IQOS_FIELD_OFFSET_1)) {
		if (param->offset1.type == IQOS_OFFSET_TYPE_L2)
			key_opt.offset1_type = IQ_KEY_OFFSET_L2;
		else if (param->offset1.type == IQOS_OFFSET_TYPE_L3)
			key_opt.offset1_type = IQ_KEY_OFFSET_L3;
		else /* if (param->offset1.type == IQOS_OFFSET_TYPE_L4) */
			key_opt.offset1_type = IQ_KEY_OFFSET_L4;

		key_opt.offset1_start = param->offset1.start;
		key_opt.offset1_size = param->offset1.size;
		key_opt.offset1_mask = param->offset1.mask;
	}

	return bcm_iq_add_keymask(keymask, &key_opt, param->prio);
}

int iq_delete_keymask(iqos_param_t *param)
{
	uint32_t keymask;
	iq_key_option_t key_opt;

	keymask = iqos_keymask_convert(param->field_mask);
	key_opt.word = 0;
	key_opt.offset0_mask = 0;
	key_opt.offset1_mask = 0;

	if (param->field_mask & (0x1 << IQOS_FIELD_OFFSET_0)) {
		if (param->offset0.type == IQOS_OFFSET_TYPE_L2)
			key_opt.offset0_type = IQ_KEY_OFFSET_L2;
		else if (param->offset0.type == IQOS_OFFSET_TYPE_L3)
			key_opt.offset0_type = IQ_KEY_OFFSET_L3;
		else /* if (param->offset0.type == IQOS_OFFSET_TYPE_L4) */
			key_opt.offset0_type = IQ_KEY_OFFSET_L4;

		key_opt.offset0_start = param->offset0.start;
		key_opt.offset0_size = param->offset0.size;
		key_opt.offset0_mask = param->offset0.mask;
	}

	if (param->field_mask & (0x1 << IQOS_FIELD_OFFSET_1)) {
		if (param->offset1.type == IQOS_OFFSET_TYPE_L2)
			key_opt.offset1_type = IQ_KEY_OFFSET_L2;
		else if (param->offset1.type == IQOS_OFFSET_TYPE_L3)
			key_opt.offset1_type = IQ_KEY_OFFSET_L3;
		else /* if (param->offset1.type == IQOS_OFFSET_TYPE_L4) */
			key_opt.offset1_type = IQ_KEY_OFFSET_L4;

		key_opt.offset1_start = param->offset1.start;
		key_opt.offset1_size = param->offset1.size;
		key_opt.offset1_mask = param->offset1.mask;
	}

	return bcm_iq_delete_keymask(keymask, &key_opt);
}

static uint32_t iqos_key_convert(iqos_param_t *param, iq_key_option_t *key_opt,
				 iq_key_data_t *key_data)
{
	uint32_t keymask = 0;
	int i;

	for (i = 0; i < IQOS_FIELD_MAX; i++) {
		if (param->field_mask & (0x1 << i)) {
			switch (i) {
			case IQOS_FIELD_INGRESS_DEVICE:
				keymask |= IQ_KEY_MASK_INGRESS_DEVICE;
				key_data->ingress_device = param->data.ingress_device;
				break;
			case IQOS_FIELD_SRC_MAC:
				keymask |= IQ_KEY_MASK_SRC_MAC;
				memcpy(key_data->src_mac, param->data.src_mac,
				       BLOG_ETH_ADDR_LEN);
				break;
			case IQOS_FIELD_DST_MAC:
				keymask |= IQ_KEY_MASK_DST_MAC;
				memcpy(key_data->dst_mac, param->data.dst_mac,
				       BLOG_ETH_ADDR_LEN);
				break;
			case IQOS_FIELD_ETHER_TYPE:
				keymask |= IQ_KEY_MASK_ETHER_TYPE;
				key_data->eth_type = param->data.eth_type;
				break;
			case IQOS_FIELD_OUTER_VID:
				keymask |= IQ_KEY_MASK_OUTER_VID;
				key_data->outer_vid = param->data.outer_vid;
				break;
			case IQOS_FIELD_OUTER_PBIT:
				keymask |= IQ_KEY_MASK_OUTER_PBIT;
				key_data->outer_pbit = param->data.outer_pbit;
				break;
			case IQOS_FIELD_INNER_VID:
				keymask |= IQ_KEY_MASK_INNER_VID;
				key_data->inner_vid = param->data.inner_vid;
				break;
			case IQOS_FIELD_INNER_PBIT:
				keymask |= IQ_KEY_MASK_INNER_PBIT;
				key_data->inner_pbit = param->data.inner_pbit;
				break;
			case IQOS_FIELD_L2_PROTO:
				keymask |= IQ_KEY_MASK_L2_PROTO;
				key_data->l2_proto = param->data.l2_proto;
				break;
			case IQOS_FIELD_L3_PROTO:
				keymask |= IQ_KEY_MASK_L3_PROTO;
				key_data->l3_proto = param->data.l3_proto;
				break;
			case IQOS_FIELD_IP_PROTO:
				keymask |= IQ_KEY_MASK_IP_PROTO;
				key_data->ip_proto = param->data.ip_proto;
				break;
			case IQOS_FIELD_SRC_IP:
				keymask |= IQ_KEY_MASK_SRC_IP;
				key_data->is_ipv6 = param->data.is_ipv6;
				memcpy(key_data->src_ip, param->data.src_ip, 16);
				break;
			case IQOS_FIELD_DST_IP:
				keymask |= IQ_KEY_MASK_DST_IP;
				key_data->is_ipv6 = param->data.is_ipv6;
				memcpy(key_data->dst_ip, param->data.dst_ip, 16);
				break;
			case IQOS_FIELD_DSCP:
				keymask |= IQ_KEY_MASK_DSCP;
				key_data->dscp = param->data.dscp;
				break;
			case IQOS_FIELD_IPV6_FLOW_LABEL:
				keymask |= IQ_KEY_MASK_IPV6_FLOW_LABEL;
				key_data->is_ipv6 = param->data.is_ipv6;
				key_data->flow_label = param->data.flow_label;
				break;
			case IQOS_FIELD_SRC_PORT:
				keymask |= IQ_KEY_MASK_SRC_PORT;
				key_data->l4_src_port = param->data.l4_src_port;
				break;
			case IQOS_FIELD_DST_PORT:
				keymask |= IQ_KEY_MASK_DST_PORT;
				key_data->l4_dst_port = param->data.l4_dst_port;
				break;
			case IQOS_FIELD_OFFSET_0:
				if (param->offset0.type >= IQOS_OFFSET_TYPE_MAX)
					return 0;

				keymask |= IQ_KEY_MASK_OFFSET_0;
				key_data->l2_offset = param->data.l2_offset;
				key_data->l3_offset = param->data.l3_offset;
				key_data->l4_offset = param->data.l4_offset;
				memcpy(key_data->packet_cache,
				       param->data.packet_cache,
				       IQ_PACKET_CACHE_MAX_SIZE);
				switch (param->offset0.type) {
				case IQOS_OFFSET_TYPE_L2:
					key_opt->offset0_type = IQ_KEY_OFFSET_L2;
					break;
				case IQOS_OFFSET_TYPE_L3:
					key_opt->offset0_type = IQ_KEY_OFFSET_L3;
					break;
				case IQOS_OFFSET_TYPE_L4:
					key_opt->offset0_type = IQ_KEY_OFFSET_L4;
					break;
				}
				key_opt->offset0_start = param->offset0.start;
				key_opt->offset0_size = param->offset0.size;
				key_opt->offset0_mask = param->offset0.mask;
				break;
			case IQOS_FIELD_OFFSET_1:
				if (param->offset1.type >= IQOS_OFFSET_TYPE_MAX)
					return 0;

				keymask |= IQ_KEY_MASK_OFFSET_1;
				key_data->l2_offset = param->data.l2_offset;
				key_data->l3_offset = param->data.l3_offset;
				key_data->l4_offset = param->data.l4_offset;
				memcpy(key_data->packet_cache,
				       param->data.packet_cache,
				       IQ_PACKET_CACHE_MAX_SIZE);
				switch (param->offset1.type) {
				case IQOS_OFFSET_TYPE_L2:
					key_opt->offset1_type = IQ_KEY_OFFSET_L2;
					break;
				case IQOS_OFFSET_TYPE_L3:
					key_opt->offset1_type = IQ_KEY_OFFSET_L3;
					break;
				case IQOS_OFFSET_TYPE_L4:
					key_opt->offset1_type = IQ_KEY_OFFSET_L4;
					break;
				}
				key_opt->offset1_start = param->offset1.start;
				key_opt->offset1_size = param->offset1.size;
				key_opt->offset1_mask = param->offset1.mask;
				break;
			default:
				printk(KERN_WARNING "%s:Unknown field detected "
				       "%d\n", __func__, i);
				break;
			}
		}
	}

	if (keymask != 0)
		key_opt->valid = 1;

	return keymask;
}

int iq_add_key(iqos_param_t *param)
{
	uint32_t keymask;
	iq_key_option_t key_opt;
	iq_key_data_t key_data;
	iq_action_t action;

	key_opt.word = 0;
	key_opt.offset0_mask = 0;
	key_opt.offset1_mask = 0;
	memset(&key_data, 0x0, sizeof(iq_key_data_t));

	keymask = iqos_key_convert(param, &key_opt, &key_data);

	action.word = 0;
	switch (param->action) {
	case IQOS_ACTION_NOP:
		action.type = IQ_ACTION_TYPE_NOP;
		break;
	case IQOS_ACTION_PRIO:
		action.type = IQ_ACTION_TYPE_PRIO;
		action.value = param->action_value;
		break;
	case IQOS_ACTION_DROP:
		action.type = IQ_ACTION_TYPE_DROP;
		break;
	case IQOS_ACTION_DST_Q:
		action.type = IQ_ACTION_TYPE_DST_Q;
		action.value = param->action_value;
		break;
	case IQOS_ACTION_TRAP:
		action.type = IQ_ACTION_TYPE_TRAP;
		break;
	default:
		break;
	}

	if (param->type == IQOS_ENT_STAT)
		action.is_static = 1;

	return bcm_iq_add_entry(keymask, &key_opt, &key_data, &action);
}

int iq_delete_key(iqos_param_t *param)
{
	uint32_t keymask;
	iq_key_option_t key_opt;
	iq_key_data_t key_data;

	key_opt.word = 0;
	key_opt.offset0_mask = 0;
	key_opt.offset1_mask = 0;
	memset(&key_data, 0x0, sizeof(iq_key_data_t));
	keymask = iqos_key_convert(param, &key_opt, &key_data);

	return bcm_iq_delete_entry(keymask, &key_opt, &key_data);
}

int iq_get_key(iqos_param_t *param)
{
	uint32_t keymask;
	iq_key_option_t key_opt;
	iq_key_data_t key_data;
	iq_action_t action;
	int rc;

	key_opt.word = 0;
	key_opt.offset0_mask = 0;
	key_opt.offset1_mask = 0;
	memset(&key_data, 0x0, sizeof(iq_key_data_t));
	keymask = iqos_key_convert(param, &key_opt, &key_data);
	action.word = 0;

	rc = bcm_iq_check_with_mask(&key_data, keymask, &action);
	if (rc)
		return rc;
	switch (action.type) {
	case IQ_ACTION_TYPE_NOP:
		param->action = IQOS_ACTION_NOP;
		break;
	case IQ_ACTION_TYPE_PRIO:
		param->action = IQOS_ACTION_PRIO;
		param->action_value = action.value;
		break;
	case IQ_ACTION_TYPE_DROP:
		param->action = IQOS_ACTION_DROP;
		break;
	case IQ_ACTION_TYPE_DST_Q:
		param->action = IQOS_ACTION_DST_Q;
		param->action_value = action.value;
		break;
	case IQ_ACTION_TYPE_TRAP:
		param->action = IQOS_ACTION_TRAP;
		break;
	default:
		break;
	}

	return rc;
}

static void iq_dump_tbl_field(iq_hash_entry_t *p_iq_hash, iq_key_field_t field)
{
	int i, start_offset;
	iq_key_data_t *key_data = &p_iq_hash->key_data;
	iq_key_option_t *key_opt = &p_iq_hash->key_opt;
	unsigned char* type_str;

	switch (field) {
	case IQ_KEY_FIELD_INGRESS_DEVICE:
		// FIXME! TBD
		printk("\t\tingress device = \n");
		return;
	case IQ_KEY_FIELD_SRC_MAC:
		printk("\t\tSRC MAC = ");
		for (i = 0; i < BLOG_ETH_ADDR_LEN - 1; i++)
			printk("%02x:", key_data->src_mac[i]);
		printk("%02x\n", key_data->src_mac[i]);
		return;
	case IQ_KEY_FIELD_DST_MAC:
		printk("\t\tDST MAC = ");
		for (i = 0; i < BLOG_ETH_ADDR_LEN - 1; i++)
			printk("%02x:", key_data->dst_mac[i]);
		printk("%02x\n", key_data->dst_mac[i]);
		return;
	case IQ_KEY_FIELD_ETHER_TYPE:
		printk("\t\tEtherType = 0x%04x\n",
		       (unsigned int)key_data->eth_type);
		return;
	case IQ_KEY_FIELD_OUTER_VID:
		printk("\t\tOuter VID = %d\n", key_data->outer_vid);
		return;
	case IQ_KEY_FIELD_OUTER_PBIT:
		printk("\t\tOuter PBIT = %d\n", key_data->outer_pbit);
		return;
	case IQ_KEY_FIELD_INNER_VID:
		printk("\t\tInner VID = %d\n", key_data->inner_vid);
		return;
	case IQ_KEY_FIELD_INNER_PBIT:
		printk("\t\tInner PBIT = %d\n", key_data->inner_pbit);
		return;
	case IQ_KEY_FIELD_L2_PROTO:
		printk("\t\tL2 Proto = %d\n", key_data->l2_proto);
		return;
	case IQ_KEY_FIELD_L3_PROTO:
		printk("\t\tL3 Proto = 0x%04x\n", key_data->l3_proto);
		return;
	case IQ_KEY_FIELD_IP_PROTO:
		printk("\t\tIP PROTO = 0x%02x\n", key_data->ip_proto);
		return;
	case IQ_KEY_FIELD_SRC_IP:
		printk("\t\tSRC IP ADDR = ");
		if (key_data->is_ipv6) {
			for (i = 0; i < 3; i++)
				printk("%08x:", key_data->src_ip[i]);
			printk("%08x\n", key_data->src_ip[i]);
			return;
		} else {
			// TODO! improve the output format
			printk("%08x\n", ntohl(key_data->src_ip[0]));
			return;
		}
	case IQ_KEY_FIELD_DST_IP:
		printk("\t\tDST IP ADDR = ");
		if (key_data->is_ipv6) {
			/* IPv6 */
			for (i = 0; i < 3; i++)
				printk("%08x:", key_data->dst_ip[i]);
			printk("%08x\n", key_data->dst_ip[i]);
			return;
		} else {
			// TODO! improve the output format
			printk("%08x\n", ntohl(key_data->dst_ip[0]));
			return;
		}
	case IQ_KEY_FIELD_DSCP:
		printk("\t\tDSCP = %d\n", key_data->dscp);
		return;
	case IQ_KEY_FIELD_IPV6_FLOW_LABEL:
		printk("\t\tFlow Label = %d\n", key_data->flow_label);
		return;
	case IQ_KEY_FIELD_SRC_PORT:
		printk("\t\tSRC Port = %d\n", key_data->l4_src_port);
		return;
	case IQ_KEY_FIELD_DST_PORT:
		printk("\t\tDST Port = %d\n", key_data->l4_dst_port);
		return;
	case IQ_KEY_FIELD_OFFSET_0:
		if (key_opt->offset0_type == IQ_KEY_OFFSET_L2) {
			type_str = "L2";
			start_offset = key_data->l2_offset;
		} else if (key_opt->offset0_type == IQ_KEY_OFFSET_L3) {
			type_str = "L3";
			start_offset = key_data->l3_offset;
		} else if (key_opt->offset0_type == IQ_KEY_OFFSET_L4) {
			type_str = "L4";
			start_offset = key_data->l4_offset;
		} else
			return;

		printk("\t\tOffset#0, type = %s, start = %d, size = %d, "
		       "4-byte mask = 0x%08x\n\t\t\tBuffer = 0x", type_str,
		       key_opt->offset0_start, key_opt->offset0_size,
		       key_opt->offset0_mask);
		for (i = start_offset;
		     i < (start_offset + key_opt->offset0_size); i++) {
			printk("%02x", key_data->packet_cache[i]);
			if ((i % 4) == 0)
				printk(" ");
		}
		return;
	case IQ_KEY_FIELD_OFFSET_1:
		if (key_opt->offset1_type == IQ_KEY_OFFSET_L2) {
			type_str = "L2";
			start_offset = key_data->l2_offset;
		} else if (key_opt->offset1_type == IQ_KEY_OFFSET_L3) {
			type_str = "L3";
			start_offset = key_data->l3_offset;
		} else if (key_opt->offset1_type == IQ_KEY_OFFSET_L4) {
			type_str = "L4";
			start_offset = key_data->l4_offset;
		} else
			return;

		printk("\t\tOffset#0, type = %s, start = %d, size = %d, "
		       "4-byte mask = 0x%08x\n\t\t\tBuffer = 0x", type_str,
		       key_opt->offset1_start, key_opt->offset1_size,
		       key_opt->offset1_mask);
		for (i = start_offset;
		     i < (start_offset + key_opt->offset1_size); i++) {
			printk("%02x", key_data->packet_cache[i]);
			if ((i % 4) == 0)
				printk(" ");
		}
		return;
	default:
		/* shouldn't reach here */
		WARN_ON(1);
	}
}

static void iq_dump_tbl(void)
{
	int i, j;
	iq_hash_entry_t *p_iq_hash;
	unsigned char *is_static_str;

	/* dump key mask table */
	printk("key mask table\n");
	printk("\t  index  | priority |  key mask  | key option | ref_cnt \n");
	printk("\t---------|----------|------------|------------|---------\n");
	for (i = 0; i < IQ_KEYMASKTBL_SIZE; i++) {
		if (keymask_tbl[i].key_opt.valid == 0)
			break;
		printk("\t%08d | %08d | 0x%08x | 0x%08x | %7u\n", i,
			keymask_tbl[i].prio, keymask_tbl[i].key_mask,
			keymask_tbl[i].key_opt.word,
			atomic_read(&keymask_tbl[i].refcnt));
	}
	printk("\n\n");

	/* dump hash table */
	printk("key hash table\n");
	printk("Total static Entry Count: %d\n", atomic_read(&static_hash_entry_cnt));
	printk("Total dynamic Entry Count: %d\n", atomic_read(&hash_entry_cnt));
	printk("\t  index  |  key mask  | key option |   action   | ref_cnt |"
	       "  hits  | static\n");
	printk("\t\t field = value\n");
	printk("\t---------|------------|------------|------------|---------|"
	       "-------\n");
	for (i = 0; i < IQ_HASHTBL_SIZE; i++) {
		p_iq_hash = &qos_htbl[i];
		if (p_iq_hash->action.valid == 0)
			continue;
		if (p_iq_hash->action.is_static == 1)
			is_static_str = "yes";
		else
			is_static_str = "no ";
		printk("\t%08d | 0x%08x | 0x%08x | 0x%08x | %07u | %06u |  %s\n", i,
			p_iq_hash->key_mask, p_iq_hash->key_opt.word,
			p_iq_hash->action.word, atomic_read(&p_iq_hash->refcnt),
			p_iq_hash->hit_cnt, is_static_str);
		for (j = 0; j < IQ_KEY_FIELD_MAX; j++) {
			if (p_iq_hash->key_mask & (0x1 << j))
				iq_dump_tbl_field(p_iq_hash, j);
		}
		printk("\t--------------------------------\n");
	}
	printk("--------------------------------\n");

	if ((g_hw_info.registered == 1) && (g_hw_info.dump_table != NULL))
		g_hw_info.dump_table(NULL);
}

static int iq_set_hw_status(uint32_t status)
{
	iq_param_t iq_param;
	iq_hash_entry_t *p_iq_hash;
	int i, rc;

	if (g_hw_info.registered == 0)
		return -EPERM;

	if (g_hw_info.set_status != NULL) {
		iq_param.status = status;
		rc = g_hw_info.set_status(&iq_param);
		if (rc)
			return rc;
	}

	/* if the hardware does not register set_status
	 * ingress QoS driver will flush all the HW entries if
	 * disable, or attempt to add the entry if enable */

	/* try catch re-enable or re-disable */
	if (status == g_hw_info.enabled)
		return 0;

	IQOS_LOCK_BH();
	if (status) {
		g_hw_info.enabled = status;
		for (i = 0; i < IQ_HASHTBL_SIZE; i++) {
			p_iq_hash = &qos_htbl[i];
			if (p_iq_hash->action.valid == 0)
				continue;

			iq_add_hw_entry(p_iq_hash);
		}
	} else {
		for (i = 0; i < IQ_HASHTBL_SIZE; i++) {
			p_iq_hash = &qos_htbl[i];
			if (p_iq_hash->action.valid == 0)
				continue;

			iq_delete_hw_entry(p_iq_hash);
		}
		g_hw_info.enabled = status;
	}
	IQOS_UNLOCK_BH();

	return 0;
}

int bcm_iq_set_status(uint32_t status)
{
	iq_set_hw_status(status);

	IQOS_LOCK_BH();

	iqos_enable_g = status;
	/* clear the CPU cong status */
	iqos_cpu_cong_g = 0;

	IQOS_UNLOCK_BH();
	return status;
}
EXPORT_SYMBOL(bcm_iq_set_status);

static int iq_set_hw_accel_cong_ctrl(uint32_t status)
{
	iq_param_t iq_param;

	if (g_hw_info.set_congestion_ctrl != NULL) {
		iq_param.status = status;
		return g_hw_info.set_congestion_ctrl(&iq_param);
	}

	return -1;
}

static int iq_get_hw_accel_cong_ctrl(void)
{
	iq_param_t iq_param;
	int rc;

	if (g_hw_info.get_congestion_ctrl != NULL) {
		rc = g_hw_info.get_congestion_ctrl(&iq_param);
		if (rc)
			return rc;
		return iq_param.status;
	}

	return -1;
}

int iq_get_status(void)
{
	uint32_t cong_ctrl = iq_get_hw_accel_cong_ctrl();

	BCM_LOG_NOTICE(BCM_LOG_ID_IQ, "Ingress QoS status : %s \n",
		       (iqos_enable_g == 1) ? "enabled": "disabled");

	BCM_LOG_NOTICE(BCM_LOG_ID_IQ, "Ingress QoS HW Accel Congestion Control status : %s \n",
		       (cong_ctrl != 0) ? "enabled": "disabled");

	if ((iqos_enet_status_hook_g != NULL) ||
	    (iqos_xtm_status_hook_g != NULL)) {
		printk("\n------------------------IQ Status---------------------\n");
		printk("        dev chnl loThr hiThr  used    dropped     cong\n");
		printk("------ ---- ---- ----- ----- ----- ---------- --------\n");
	}

	/* Dump the Ingress QoS status for various interfaces */
	if (iqos_enet_status_hook_g != NULL)
		iqos_enet_status_hook_g();

	if (iqos_xtm_status_hook_g != NULL)
		iqos_xtm_status_hook_g();

	if ((g_hw_info.registered == 1) && (g_hw_info.get_status != NULL))
		g_hw_info.get_status(NULL);

	return iqos_enable_g;
}

void bcm_iq_flush(void)
{
	iq_hash_entry_t *p_iq_hash;
	iq_keymask_entry_t *curr;
	int i;

	IQOS_LOCK_BH();
	for (i = 0; i < IQ_HASHTBL_SIZE; i++) {
		p_iq_hash = &qos_htbl[i];
		if ((p_iq_hash->action.valid == 0) ||
		    (p_iq_hash->action.is_static == 1))
			continue;

		/* delete from the HW accelerator */
		iq_delete_hw_entry(p_iq_hash);

		/* decrement the refcnt in keymask */
		atomic_dec(&keymask_tbl[p_iq_hash->keymask_idx].refcnt);
		atomic_dec(&hash_entry_cnt);

		memset(p_iq_hash, 0x0, sizeof(iq_hash_entry_t));
	}

	for (i = atomic_read(&keymask_entry_cnt) - 1; i >= 0; i--) {
		curr = &keymask_tbl[i];
		if ((curr->key_opt.valid == 1) &&
		    (atomic_read(&curr->refcnt) == 0))
			 __bcm_iq_delete_keymask_by_index(i);
	}

	if (atomic_read(&hash_entry_cnt) != 0)
		printk("%s:hash_entry_cnt(%d) != 0)", __func__,
		       atomic_read(&hash_entry_cnt));

	atomic_set(&hash_entry_cnt, 0);
	IQOS_UNLOCK_BH();
}
EXPORT_SYMBOL(bcm_iq_flush);

static uint32_t iq_drv_iqctl_keymask_convert(uint32_t iqctl_field_mask)
{
	uint32_t keymask = 0;
	int i;

	for (i = 0; i < IQCTL_KEY_FIELD_MAX; i++) {
		if (iqctl_field_mask & (0x1 << i)) {
			switch (i) {
			case IQCTL_KEY_FIELD_INGRESS_DEVICE:
				keymask |= IQ_KEY_MASK_INGRESS_DEVICE;
				break;
			case IQCTL_KEY_FIELD_SRC_MAC:
				keymask |= IQ_KEY_MASK_SRC_MAC;
				break;
			case IQCTL_KEY_FIELD_DST_MAC:
				keymask |= IQ_KEY_MASK_DST_MAC;
				break;
			case IQCTL_KEY_FIELD_ETHER_TYPE:
				keymask |= IQ_KEY_MASK_ETHER_TYPE;
				break;
			case IQCTL_KEY_FIELD_OUTER_VID:
				keymask |= IQ_KEY_MASK_OUTER_VID;
				break;
			case IQCTL_KEY_FIELD_OUTER_PBIT:
				keymask |= IQ_KEY_MASK_OUTER_PBIT;
				break;
			case IQCTL_KEY_FIELD_INNER_VID:
				keymask |= IQ_KEY_MASK_INNER_VID;
				break;
			case IQCTL_KEY_FIELD_INNER_PBIT:
				keymask |= IQ_KEY_MASK_INNER_PBIT;
				break;
			case IQCTL_KEY_FIELD_L2_PROTO:
				keymask |= IQ_KEY_MASK_L2_PROTO;
				break;
			case IQCTL_KEY_FIELD_L3_PROTO:
				keymask |= IQ_KEY_MASK_L3_PROTO;
				break;
			case IQCTL_KEY_FIELD_IP_PROTO:
				keymask |= IQ_KEY_MASK_IP_PROTO;
				break;
			case IQCTL_KEY_FIELD_SRC_IP:
				keymask |= IQ_KEY_MASK_SRC_IP;
				break;
			case IQCTL_KEY_FIELD_DST_IP:
				keymask |= IQ_KEY_MASK_DST_IP;
				break;
			case IQCTL_KEY_FIELD_DSCP:
				keymask |= IQ_KEY_MASK_DSCP;
				break;
			case IQCTL_KEY_FIELD_IPV6_FLOW_LABEL:
				keymask |= IQ_KEY_MASK_IPV6_FLOW_LABEL;
				break;
			case IQCTL_KEY_FIELD_SRC_PORT:
				keymask |= IQ_KEY_MASK_SRC_PORT;
				break;
			case IQCTL_KEY_FIELD_DST_PORT:
				keymask |= IQ_KEY_MASK_DST_PORT;
				break;
			case IQCTL_KEY_FIELD_OFFSET_0:
				keymask |= IQ_KEY_MASK_OFFSET_0;
				break;
			case IQCTL_KEY_FIELD_OFFSET_1:
				keymask |= IQ_KEY_MASK_OFFSET_1;
				break;
			default:
				printk(KERN_WARNING "%s:Unknown field detected "
				       "%d\n", __func__, i);
				break;
			}
		}
	}
	return keymask;
}

static uint32_t iq_drv_iqctl_keymask_convert_back(uint32_t bcm_iq_keymask)
{
	uint32_t iqctl_field_mask = 0;
	int i;

	for (i = 0; i < IQ_KEY_FIELD_MAX; i++) {
		if (bcm_iq_keymask & (0x1 << i)) {
			switch (i) {
			case IQ_KEY_FIELD_INGRESS_DEVICE:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_INGRESS_DEVICE;
				break;
			case IQ_KEY_FIELD_SRC_MAC:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_SRC_MAC;
				break;
			case IQ_KEY_FIELD_DST_MAC:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_MAC;
				break;
			case IQ_KEY_FIELD_ETHER_TYPE:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_ETHER_TYPE;
				break;
			case IQ_KEY_FIELD_OUTER_VID:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_OUTER_VID;
				break;
			case IQ_KEY_FIELD_OUTER_PBIT:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_OUTER_PBIT;
				break;
			case IQ_KEY_FIELD_INNER_VID:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_INNER_VID;
				break;
			case IQ_KEY_FIELD_INNER_PBIT:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_INNER_PBIT;
				break;
			case IQ_KEY_FIELD_L2_PROTO:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_L2_PROTO;
				break;
			case IQ_KEY_FIELD_L3_PROTO:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_L3_PROTO;
				break;
			case IQ_KEY_FIELD_IP_PROTO:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
				break;
			case IQ_KEY_FIELD_SRC_IP:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_SRC_IP;
				break;
			case IQ_KEY_FIELD_DST_IP:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_IP;
				break;
			case IQ_KEY_FIELD_DSCP:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_DSCP;
				break;
			case IQ_KEY_FIELD_IPV6_FLOW_LABEL:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_IPV6_FLOW_LABEL;
				break;
			case IQ_KEY_FIELD_SRC_PORT:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_SRC_PORT;
				break;
			case IQ_KEY_FIELD_DST_PORT:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_PORT;
				break;
			case IQ_KEY_FIELD_OFFSET_0:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_OFFSET_0;
				break;
			case IQ_KEY_FIELD_OFFSET_1:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_OFFSET_1;
				break;
			default:
				printk(KERN_WARNING "%s:Unknown field detected "
				       "%d\n", __func__, i);
				break;
			}
		}
	}
	return iqctl_field_mask;
}

static int iq_drv_ioctl_subsys_keymask(iqctl_data_t *iq_p, unsigned long arg)
{
	int ret = 0;
	uint32_t keymask;
	iq_key_option_t key_opt;
	int i;

	key_opt.word = 0;
	key_opt.offset0_mask = 0;
	key_opt.offset1_mask = 0;

	switch (iq_p->op) {
	case IQCTL_OP_ADD:
		keymask = iq_drv_iqctl_keymask_convert(iq_p->key_data.key_field_mask);

		/* FIXME!! implement option filling for offset type feature  */

		iq_p->rc = bcm_iq_add_keymask(keymask, &key_opt, iq_p->prio);
		copy_to_user((uint8_t *)arg, iq_p, sizeof(iqctl_data_t));
		break;
	case IQCTL_OP_REM:
		keymask = iq_drv_iqctl_keymask_convert(iq_p->key_data.key_field_mask);

		/* FIXME!! implement option filling for offset type feature  */

		iq_p->rc = bcm_iq_delete_keymask(keymask, &key_opt);
		copy_to_user((uint8_t *)arg, iq_p, sizeof(iqctl_data_t));
		break;
	case IQCTL_OP_GETNEXT:
		for (i = iq_p->nextIx; i < IQ_KEYMASKTBL_SIZE; i++) {
			if (keymask_tbl[i].key_opt.valid == 0)
				break;
			iq_p->key_data.key_field_mask = 
				iq_drv_iqctl_keymask_convert_back(
					keymask_tbl[i].key_mask);
			iq_p->prio = keymask_tbl[i].prio;
			/* FIXME!! implement option for offset feature */
			iq_p->nextIx = i;
			iq_p->refcnt = atomic_read(&keymask_tbl[i].refcnt);
			copy_to_user((uint8_t *)arg, iq_p, sizeof(iqctl_data_t));
			return 0;
		}
		iq_p->nextIx = -1;
		copy_to_user((uint8_t *)arg, iq_p, sizeof(iqctl_data_t));
		return 0;
	default:
		BCM_LOG_ERROR(BCM_LOG_ID_IQ,"Invalid op[%u]", iq_p->op);
		ret = -EINVAL;
	}
	return ret;
}

static uint32_t iq_drv_iqctl_key_convert(iqctl_data_t *iq_p,
					 iq_key_option_t *key_opt,
					 iq_key_data_t *key_data)
{
	uint32_t keymask = 0;
	int i;

	for (i = 0; i < IQCTL_KEY_FIELD_MAX; i++) {
		if (iq_p->key_data.key_field_mask & (0x1 << i)) {
			switch (i) {
			case IQCTL_KEY_FIELD_INGRESS_DEVICE:
				keymask |= IQ_KEY_MASK_INGRESS_DEVICE;
				key_data->ingress_device = iq_p->key_data.ingress_device;
				break;
			case IQCTL_KEY_FIELD_SRC_MAC:
				keymask |= IQ_KEY_MASK_SRC_MAC;
				memcpy(key_data->src_mac, iq_p->key_data.src_mac,
				       BLOG_ETH_ADDR_LEN);
				break;
			case IQCTL_KEY_FIELD_DST_MAC:
				keymask |= IQ_KEY_MASK_DST_MAC;
				memcpy(key_data->dst_mac, iq_p->key_data.dst_mac,
				       BLOG_ETH_ADDR_LEN);
				break;
			case IQCTL_KEY_FIELD_ETHER_TYPE:
				keymask |= IQ_KEY_MASK_ETHER_TYPE;
				key_data->eth_type = iq_p->key_data.eth_type;
				break;
			case IQCTL_KEY_FIELD_OUTER_VID:
				keymask |= IQ_KEY_MASK_OUTER_VID;
				key_data->outer_vid = iq_p->key_data.outer_vid;
				break;
			case IQCTL_KEY_FIELD_OUTER_PBIT:
				keymask |= IQ_KEY_MASK_OUTER_PBIT;
				key_data->outer_pbit = iq_p->key_data.outer_pbit;
				break;
			case IQCTL_KEY_FIELD_INNER_VID:
				keymask |= IQ_KEY_MASK_INNER_VID;
				key_data->inner_vid = iq_p->key_data.inner_vid;
				break;
			case IQCTL_KEY_FIELD_INNER_PBIT:
				keymask |= IQ_KEY_MASK_INNER_PBIT;
				key_data->inner_pbit = iq_p->key_data.inner_pbit;
				break;
			case IQCTL_KEY_FIELD_L2_PROTO:
				keymask |= IQ_KEY_MASK_L2_PROTO;
				key_data->l2_proto = iq_p->key_data.l2_proto;
				break;
			case IQCTL_KEY_FIELD_L3_PROTO:
				keymask |= IQ_KEY_MASK_L3_PROTO;
				key_data->l3_proto = iq_p->key_data.l3_proto;
				break;
			case IQCTL_KEY_FIELD_IP_PROTO:
				keymask |= IQ_KEY_MASK_IP_PROTO;
				key_data->ip_proto = iq_p->key_data.ip_proto;
				break;
			case IQCTL_KEY_FIELD_SRC_IP:
				keymask |= IQ_KEY_MASK_SRC_IP;
				key_data->is_ipv6 = iq_p->key_data.is_ipv6;
				memcpy(key_data->src_ip, iq_p->key_data.src_ip, 16);
				break;
			case IQCTL_KEY_FIELD_DST_IP:
				keymask |= IQ_KEY_MASK_DST_IP;
				key_data->is_ipv6 = iq_p->key_data.is_ipv6;
				memcpy(key_data->dst_ip, iq_p->key_data.dst_ip, 16);
				break;
			case IQCTL_KEY_FIELD_DSCP:
				keymask |= IQ_KEY_MASK_DSCP;
				key_data->dscp = iq_p->key_data.dscp;
				break;
			case IQCTL_KEY_FIELD_IPV6_FLOW_LABEL:
				keymask |= IQ_KEY_MASK_IPV6_FLOW_LABEL;
				key_data->is_ipv6 = iq_p->key_data.is_ipv6;
				key_data->flow_label = iq_p->key_data.flow_label;
				break;
			case IQCTL_KEY_FIELD_SRC_PORT:
				keymask |= IQ_KEY_MASK_SRC_PORT;
				key_data->l4_src_port = iq_p->key_data.l4_src_port;
				break;
			case IQCTL_KEY_FIELD_DST_PORT:
				keymask |= IQ_KEY_MASK_DST_PORT;
				key_data->l4_dst_port = iq_p->key_data.l4_dst_port;
				break;
			case IQCTL_KEY_FIELD_OFFSET_0:
				keymask |= IQ_KEY_MASK_OFFSET_0;
				/* FIXME!! implement me */
				break;
			case IQCTL_KEY_FIELD_OFFSET_1:
				keymask |= IQ_KEY_MASK_OFFSET_1;
				/* FIXME!! implement me */
				break;
			default:
				printk(KERN_WARNING "%s:Unknown field detected "
				       "%d\n", __func__, i);
				break;
			}
		}
	}

	if (keymask != 0)
		key_opt->valid = 1;

	return keymask;
}

static void iq_drv_iqctl_key_convert_back(iqctl_data_t *p_iq,
					  iq_hash_entry_t *p_iq_hash)
{
	uint32_t iqctl_field_mask = 0;
	int i;

	for (i = 0; i < IQ_KEY_FIELD_MAX; i++) {
		if (p_iq_hash->key_mask & (0x1 << i)) {
			switch (i) {
			case IQ_KEY_FIELD_INGRESS_DEVICE:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_INGRESS_DEVICE;
				/* FIXME!! not yet supported fully */
				p_iq->key_data.ingress_device =
					p_iq_hash->key_data.ingress_device;
				break;
			case IQ_KEY_FIELD_SRC_MAC:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_SRC_MAC;
				memcpy(p_iq->key_data.src_mac,
				       p_iq_hash->key_data.src_mac,
				       BLOG_ETH_ADDR_LEN);
				break;
			case IQ_KEY_FIELD_DST_MAC:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_MAC;
				memcpy(p_iq->key_data.dst_mac,
				       p_iq_hash->key_data.dst_mac,
				       BLOG_ETH_ADDR_LEN);
				break;
			case IQ_KEY_FIELD_ETHER_TYPE:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_ETHER_TYPE;
				p_iq->key_data.eth_type =
					p_iq_hash->key_data.eth_type;
				break;
			case IQ_KEY_FIELD_OUTER_VID:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_OUTER_VID;
				p_iq->key_data.outer_vid =
					p_iq_hash->key_data.outer_vid;
				break;
			case IQ_KEY_FIELD_OUTER_PBIT:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_OUTER_PBIT;
				p_iq->key_data.outer_pbit =
					p_iq_hash->key_data.outer_pbit;
				break;
			case IQ_KEY_FIELD_INNER_VID:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_INNER_VID;
				p_iq->key_data.inner_vid =
					p_iq_hash->key_data.inner_vid;
				break;
			case IQ_KEY_FIELD_INNER_PBIT:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_INNER_PBIT;
				p_iq->key_data.inner_pbit =
					p_iq_hash->key_data.inner_pbit;
				break;
			case IQ_KEY_FIELD_L2_PROTO:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_L2_PROTO;
				p_iq->key_data.l2_proto =
					p_iq_hash->key_data.l2_proto;
				break;
			case IQ_KEY_FIELD_L3_PROTO:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_L3_PROTO;
				p_iq->key_data.l3_proto =
					p_iq_hash->key_data.l3_proto;
				break;
			case IQ_KEY_FIELD_IP_PROTO:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
				p_iq->key_data.ip_proto =
					p_iq_hash->key_data.ip_proto;
				break;
			case IQ_KEY_FIELD_SRC_IP:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_SRC_IP;
				p_iq->key_data.is_ipv6 =
					p_iq_hash->key_data.is_ipv6;
				memcpy(p_iq->key_data.src_ip,
				       p_iq_hash->key_data.src_ip, 16);
				break;
			case IQ_KEY_FIELD_DST_IP:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_IP;
				p_iq->key_data.is_ipv6 =
					p_iq_hash->key_data.is_ipv6;
				memcpy(p_iq->key_data.dst_ip,
				       p_iq_hash->key_data.dst_ip, 16);
				break;
			case IQ_KEY_FIELD_DSCP:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_DSCP;
				p_iq->key_data.dscp =
					p_iq_hash->key_data.dscp;
				break;
			case IQ_KEY_FIELD_IPV6_FLOW_LABEL:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_IPV6_FLOW_LABEL;
				p_iq->key_data.flow_label =
					p_iq_hash->key_data.flow_label;
				break;
			case IQ_KEY_FIELD_SRC_PORT:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_SRC_PORT;
				p_iq->key_data.l4_src_port =
					p_iq_hash->key_data.l4_src_port;
				break;
			case IQ_KEY_FIELD_DST_PORT:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_PORT;
				p_iq->key_data.l4_dst_port =
					p_iq_hash->key_data.l4_dst_port;
				break;
			case IQ_KEY_FIELD_OFFSET_0:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_OFFSET_0;
				/* FIXME!! implement me!!! */
				break;
			case IQ_KEY_FIELD_OFFSET_1:
				iqctl_field_mask |= 0x1 << IQCTL_KEY_FIELD_OFFSET_1;
				/* FIXME!! implement me!!! */
				break;
			default:
				printk(KERN_WARNING "%s:Unknown field detected "
				       "%d\n", __func__, i);
				break;
			}
		}
	}
	p_iq->key_data.key_field_mask = iqctl_field_mask;
	if (p_iq_hash->action.is_static == 1)
		p_iq->ent = IQCTL_ENT_STAT;
	else
		p_iq->ent = IQCTL_ENT_DYN;

	switch (p_iq_hash->action.type) {
	case IQ_ACTION_TYPE_NOP:
		p_iq->action = IQCTL_ACTION_NOP;
		break;
	case IQ_ACTION_TYPE_PRIO:
		p_iq->action = IQCTL_ACTION_PRIO;
		p_iq->action_value = p_iq_hash->action.value;
		break;
	case IQ_ACTION_TYPE_DROP:
		p_iq->action = IQCTL_ACTION_DROP;
		break;
	case IQ_ACTION_TYPE_DST_Q:
		p_iq->action = IQCTL_ACTION_DST_Q;
		p_iq->action_value = p_iq_hash->action.value;
		break;
	case IQ_ACTION_TYPE_TRAP:
		p_iq->action = IQCTL_ACTION_TRAP;
		break;
	default:
		break;
	}
	p_iq->refcnt = atomic_read(&p_iq_hash->refcnt);
	p_iq->hitcnt = p_iq_hash->hit_cnt;
}

static int iq_drv_ioctl_subsys_key(iqctl_data_t *iq_p, unsigned long arg)
{
	uint32_t keymask;
	iq_key_option_t key_opt;
	iq_key_data_t key_data;
	iq_action_t action;
	iq_hash_entry_t *p_iq_hash;
	int i;

	key_opt.word = 0;
	key_opt.offset0_mask = 0;
	key_opt.offset1_mask = 0;
	memset(&key_data, 0x0, sizeof(iq_key_data_t));

	keymask = iq_drv_iqctl_key_convert(iq_p, &key_opt, &key_data);

	switch (iq_p->op) {
	case IQCTL_OP_ADD:
		action.word = 0;
		switch (iq_p->action) {
		case IQCTL_ACTION_NOP:
			action.type = IQ_ACTION_TYPE_NOP;
			break;
		case IQCTL_ACTION_PRIO:
			action.type = IQ_ACTION_TYPE_PRIO;
			action.value = iq_p->action_value;
			break;
		case IQCTL_ACTION_DROP:
			action.type = IQ_ACTION_TYPE_DROP;
			break;
		case IQCTL_ACTION_DST_Q:
			action.type = IQ_ACTION_TYPE_DST_Q;
			action.value = iq_p->action_value;
			break;
		case IQCTL_ACTION_TRAP:
			action.type = IQ_ACTION_TYPE_TRAP;
			break;
		default:
			break;
		}

		if (iq_p->ent == IQCTL_ENT_STAT)
			action.is_static = 1;

		iq_p->rc = bcm_iq_add_entry(keymask, &key_opt, &key_data, &action);
		copy_to_user((uint8_t *)arg, iq_p, sizeof(iqctl_data_t));
		break;
	case IQCTL_OP_REM:
		iq_p->rc = bcm_iq_delete_entry(keymask, &key_opt, &key_data);
		copy_to_user((uint8_t *)arg, iq_p, sizeof(iqctl_data_t));
		break;
	case IQCTL_OP_GET:
		action.word = 0;
		iq_p->rc = __bcm_iq_check(&key_data, keymask, &action, 1);
		if (iq_p->rc) {
			copy_to_user((uint8_t *)arg, iq_p, sizeof(iqctl_data_t));
			return 0;
		}

		switch (action.type) {
		case IQ_ACTION_TYPE_NOP:
			iq_p->action = IQCTL_ACTION_NOP;
			break;
		case IQ_ACTION_TYPE_PRIO:
			iq_p->action = IQCTL_ACTION_PRIO;
			iq_p->action_value = action.value;
			break;
		case IQ_ACTION_TYPE_DROP:
			iq_p->action = IQCTL_ACTION_DROP;
			break;
		case IQ_ACTION_TYPE_DST_Q:
			iq_p->action = IQCTL_ACTION_DST_Q;
			iq_p->action_value = action.value;
			break;
		case IQ_ACTION_TYPE_TRAP:
			iq_p->action = IQCTL_ACTION_TRAP;
			break;
		default:
			break;
		}
		if (action.is_static == 1)
			iq_p->ent = IQCTL_ENT_STAT;

		copy_to_user((uint8_t *)arg, iq_p, sizeof(iqctl_data_t));
		break;
	case IQCTL_OP_FLUSH:
		bcm_iq_flush();
		break;
	case IQCTL_OP_DUMP:
		iq_dump_tbl();
		break;
	case IQCTL_OP_GETNEXT:
		for (i = iq_p->nextIx; i < IQ_HASHTBL_SIZE; i++) {
			p_iq_hash = &qos_htbl[i];
			if (p_iq_hash->action.valid == 0)
				continue;
			iq_drv_iqctl_key_convert_back(iq_p, p_iq_hash);
			iq_p->nextIx = i;
			copy_to_user((uint8_t *)arg, iq_p, sizeof(iqctl_data_t));
			return 0;
		}
		iq_p->nextIx = -1;
		copy_to_user((uint8_t *)arg, iq_p, sizeof(iqctl_data_t));
		return 0;
	default:
		BCM_LOG_ERROR(BCM_LOG_ID_IQ,"Invalid op[%u]", iq_p->op);
		return -EINVAL;
	}
	return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iq_drv_ioctl
 * Description  : Main entry point to handle user applications IOCTL requests
 *                Ingress QoS Utility.
 * Returns      : 0 - success or error
 *------------------------------------------------------------------------------
 */
static long iq_drv_ioctl(struct file *filep, unsigned int command,
			 unsigned long arg)
{
	iqctl_ioctl_t cmd;
	iqctl_data_t iq;
	iqctl_data_t *iq_p = &iq;
	int ret = 0;

	if ( command > IQCTL_IOCTL_MAX )
		cmd = IQCTL_IOCTL_MAX;
	else
		cmd = (iqctl_ioctl_t)command;

	copy_from_user(iq_p, (uint8_t *)arg, sizeof(iq));

	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "cmd<%d>%s subsys<%d>%s op<%d>%s arg<0x%lx>",
		      command, iqctl_ioctl_name[command],
		      iq_p->subsys, iqctl_subsys_name[iq_p->subsys],
		      iq_p->op, iqctl_op_name[iq_p->op], arg);

	if (cmd == IQCTL_IOCTL_SYS) {
		switch (iq_p->subsys) {
		case IQCTL_SUBSYS_STATUS:
			switch (iq_p->op) {
			case IQCTL_OP_GET:
				iq_p->status = iqos_enable_g;
				iq_p->stacnt = atomic_read(&static_hash_entry_cnt);
				iq_p->dyncnt = atomic_read(&hash_entry_cnt);
				copy_to_user((uint8_t *)arg, iq_p, sizeof(iq));
				break;
			case IQCTL_OP_SET:
				bcm_iq_set_status(iq_p->status);
				break;
			case IQCTL_OP_DUMP:
				iq_get_status();
				break;
			default:
				BCM_LOG_ERROR(BCM_LOG_ID_IQ, "Invalid op[%u]",
					      iq_p->op);
			}
			break;

		case IQCTL_SUBSYS_KEY:
			ret = iq_drv_ioctl_subsys_key(iq_p, arg);
			break;

		case IQCTL_SUBSYS_KEYMASK:
			ret = iq_drv_ioctl_subsys_keymask(iq_p, arg);
			break;

		case IQCTL_SUBSYS_HW_ACCEL_CONG_CTRL:
			switch (iq_p->op) {
			case IQCTL_OP_SET:
				iq_p->rc = iq_set_hw_accel_cong_ctrl(iq_p->status);
				copy_to_user((uint8_t *)arg, iq_p, sizeof(iq));
				break;
			case IQCTL_OP_GET:
				iq_p->rc = iq_get_hw_accel_cong_ctrl();
				iq_p->status = (uint32_t)iq_p->rc;
				copy_to_user((uint8_t *)arg, iq_p, sizeof(iq));
				break;
			default:
				BCM_LOG_ERROR(BCM_LOG_ID_IQ, "Invalid op[%u]",
					      iq_p->op);
			}
			break;

		default:
			BCM_LOG_ERROR(BCM_LOG_ID_IQ, "Invalid subsys[%u]",
				      iq_p->subsys);
		}
	} else {
		BCM_LOG_ERROR(BCM_LOG_ID_IQ, "Invalid cmd[%u]", command);
		ret = -EINVAL;
	}

	return ret;

} /* iq_drv_ioctl */

/*
 *------------------------------------------------------------------------------
 * Function Name: iq_drv_open
 * Description  : Called when a user application opens this device.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
static int iq_drv_open(struct inode *inode, struct file *filp)
{
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "Access Ingress QoS Char Device");
	return 0;
} /* iq_drv_open */

/* Global file ops */
static struct file_operations iq_fops =
{
	.unlocked_ioctl = iq_drv_ioctl,
#if defined(CONFIG_COMPAT)
	.compat_ioctl = iq_drv_ioctl,
#endif
	.open = iq_drv_open,
};

/*
 *------------------------------------------------------------------------------
 * Function Name: iq_drv_construct
 * Description  : Initial function that is called at system startup that
 *                registers this device.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
static int iq_drv_construct(void)
{
	bcmLog_setLogLevel(BCM_LOG_ID_IQ, BCM_LOG_LEVEL_NOTICE);

	if (register_chrdev(IQ_DRV_MAJOR, IQ_DRV_NAME, &iq_fops)) {
		BCM_LOG_ERROR(BCM_LOG_ID_IQ,
			      "%s Unable to get major number <%d>" CLRnl,
			      __func__, IQ_DRV_MAJOR);
		return -ENODEV;
	}

	printk(IQ_MODNAME " Char Driver " IQ_VER_STR " Registered<%d>"
	       CLRnl, IQ_DRV_MAJOR );

	return IQ_DRV_MAJOR;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iq_drv_destruct
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
static void iq_drv_destruct(void)
{
	unregister_chrdev(IQ_DRV_MAJOR, IQ_DRV_NAME);

	printk(IQ_MODNAME " Char Driver " IQ_VER_STR " Unregistered<%d>"
	       CLRnl, IQ_DRV_MAJOR);
}

/* Hardware registration/de-registration APIs */
int bcm_iq_register_hw(const iq_hw_info_t *hw_info)
{
	if (g_hw_info.registered == 1)
		return -EPERM;

	memcpy(&g_hw_info, hw_info, sizeof(iq_hw_info_t));
	g_hw_info.registered = 1;

	/* the ingress qos driver should be insmoded before
	 * the registration of the HW, we need to go through
	 * the existing created entries and adding the supported
	 * entry to hardware */
	iq_set_hw_status(iqos_enable_g);

	BCM_LOG_INFO(BCM_LOG_ID_IQ, "Done registering HW to Ingress Qos");

	return 0;
}
EXPORT_SYMBOL(bcm_iq_register_hw);

int bcm_iq_unregister_hw(iq_hw_info_t *hw_info)
{
	if (g_hw_info.registered == 0)
		return -EPERM;

	/* making sure the given info matched with the registerd one */
	hw_info->registered = 1;
	if (memcmp(hw_info, &g_hw_info, sizeof(iq_hw_info_t)))
		return -EINVAL;

	/* disable it first */
	iq_set_hw_status(0);

	/* flush all the entries */
	// FIXME!! TBD!!

	memset(&g_hw_info, 0x0, sizeof(iq_hw_info_t));

	return 0;
}
EXPORT_SYMBOL(bcm_iq_unregister_hw);

#ifdef STATIC_ENTRY
static int __init iq_default_static_entry_init(void)
{
	int rc = 0;
#if 0
	iq_key_option_t key_option;
	iq_key_data_t key_data;
	iq_action_t action;

	/* default statis entry for TRAP will not work here, because port object
	 * usually is not created yet */
	action.word = 0;
	action.is_static = 1;
	action.type = IQ_ACTION_TYPE_TRAP;
	action.value = 1;

	/* init for ARP early trap */
	memset(&key_data, 0x0, sizeof(iq_key_data_t));
	key_data.eth_type = ETH_P_ARP;
	rc = iq_add_static_entry(IQ_KEY_MASK_ETHER_TYPE,
				 &key_option, &key_data, &action);
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "default ARP early trap. rc<%d>", rc);

	/* init for PPPoE Discovery early trap */
	memset(&key_data, 0x0, sizeof(iq_key_data_t));
	key_data.eth_type = ETH_P_PPP_DISC;
	rc = iq_add_static_entry(IQ_KEY_MASK_ETHER_TYPE,
				 &key_option, &key_data, &action);
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "default PPP_DISC early trap. rc<%d>", rc);

	/* init for PPPoE Session early trap */
	memset(&key_data, 0x0, sizeof(iq_key_data_t));
	key_data.eth_type = ETH_P_PPP_SES;
	rc = iq_add_static_entry(IQ_KEY_MASK_ETHER_TYPE,
				 &key_option, &key_data, &action);
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "default PPP_SES early trap. rc<%d>", rc);

#endif

	return rc;
}
#endif

static int __init iq_default_entry_init(void)
{
	iq_key_option_t key_option;
	iq_key_data_t key_data;
	iq_action_t action;
	int rc;

#ifdef STATIC_ENTRY
	iq_default_static_entry_init();
#endif
	/* adding the default key mask */
	key_option.word = 0;
	key_option.offset0_mask = 0;
	key_option.offset1_mask = 0;
#define IQ_PROTO_DST_PORT_PRIO	8	/* FIXME! Temporarily define here */
	bcm_iq_add_keymask(IQ_KEY_MASK_IP_PROTO + IQ_KEY_MASK_DST_PORT, &key_option,
		       IQ_PROTO_DST_PORT_PRIO);
#define IQ_IPPROTO_PRIO		9	/* FIXME! Temporarily define here */
	bcm_iq_add_keymask(IQ_KEY_MASK_IP_PROTO, &key_option, IQ_IPPROTO_PRIO);
#define IQ_PROTO_SRC_PORT_PRIO	10	/* FIXME! Temporarily define here */
	bcm_iq_add_keymask(IQ_KEY_MASK_IP_PROTO + IQ_KEY_MASK_SRC_PORT, &key_option,
		       IQ_PROTO_SRC_PORT_PRIO);
#define IQ_PROTO_ETHTYPE_PRIO	14	/* FIXME! Temporarily define here */
	bcm_iq_add_keymask(IQ_KEY_MASK_ETHER_TYPE, &key_option,
		       IQ_PROTO_ETHTYPE_PRIO);

	/* NOTE: SIP and RTSP ports will be added in their modules, but
	 *       they rely on the same IP_PROTO+DST_PORT mask */

	/* Init UDP port # */
	iqos_add_L4port(IPPROTO_UDP, 53, 1, 1);  /* DNS UDP port 53 */
	iqos_add_L4port(IPPROTO_UDP, 67, 1, 1);  /* DHCP UDP port 67 */
	iqos_add_L4port(IPPROTO_UDP, 68, 1, 1);  /* DHCP UDP port 68 */
	iqos_add_L4port(IPPROTO_UDP, 546, 1, 1);  /* DHCP UDP port 546 */
	iqos_add_L4port(IPPROTO_UDP, 547, 1, 1);  /* DHCP UDP port 547 */
	iqos_add_L4port(IPPROTO_UDP, 2427, 1, 1);  /* MGCP UDP port 2427 */
	iqos_add_L4port(IPPROTO_UDP, 2727, 1, 1);  /* MGCP UDP port 2727 */

	/* Init TCP port # */
	iqos_add_L4port(IPPROTO_TCP, 53, 1, 1);  /* DNS TCP port 53 */
	iqos_add_L4port(IPPROTO_TCP, 80, 1, 1);  /* HTTP TCP port 80 */
	iqos_add_L4port(IPPROTO_TCP, 8080, 1, 1);  /* HTTP TCP port 8080 */

	/* Init for DNS SRC Port static */
	key_data.ip_proto = IPPROTO_UDP;
	key_data.l4_src_port = 53;
	action.word = 0;
	action.type = IQ_ACTION_TYPE_PRIO;
	action.is_static = 1;
	action.value = 1;
	rc = bcm_iq_add_entry(IQ_KEY_MASK_IP_PROTO + IQ_KEY_MASK_SRC_PORT,
			  &key_option, &key_data, &action);
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "default DNS SRC Port Entry  rc<%d>", rc);

	key_data.ip_proto = IPPROTO_TCP;
	rc = bcm_iq_add_entry(IQ_KEY_MASK_IP_PROTO + IQ_KEY_MASK_SRC_PORT,
			  &key_option, &key_data, &action);
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "default DNS SRC Port Entry  rc<%d>", rc);

	/* init for PPP_DISC high priority static */
	memset(&key_data, 0x0, sizeof(iq_key_data_t));
	key_data.eth_type = ETH_P_PPP_DISC;
	rc = bcm_iq_add_entry(IQ_KEY_MASK_ETHER_TYPE, &key_option, &key_data,
			      &action);
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "default PPP_DISC high prio. rc<%d>", rc);

	/* init for PPP_SES high priority static */
	key_data.eth_type = ETH_P_PPP_SES;
	rc = bcm_iq_add_entry(IQ_KEY_MASK_ETHER_TYPE, &key_option, &key_data,
			      &action);
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "default PPP_SES high prio. rc<%d>", rc);

	/* init for ARP high priority static */
	key_data.eth_type = ETH_P_ARP;
	rc = bcm_iq_add_entry(IQ_KEY_MASK_ETHER_TYPE, &key_option, &key_data,
			      &action);
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "default ARP high prio. rc<%d>", rc);

	/* init for 8021AG high priority static */
	key_data.eth_type = ETH_P_8021AG;
	rc = bcm_iq_add_entry(IQ_KEY_MASK_ETHER_TYPE, &key_option, &key_data,
			      &action);
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "default 8021AG high prio. rc<%d>", rc);

	return rc;
}

/*
 *------------------------------------------------------------------------------
 * Function     : iq_init
 * Description  : Static construction of ingress QoS subsystem.
 *------------------------------------------------------------------------------
 */
int __init iq_init(void)
{
	int rc;

	if (unlikely(IQ_KEY_FIELD_MAX >= 32)) {
		printk("IQ_KEY_FIELD_MAX is larger than 32.\n");
		return -ERANGE;
	}

	rc = iq_drv_construct();
	if (rc < 0)
		return rc;

	atomic_set(&keymask_entry_cnt, 0);
	atomic_set(&hash_entry_cnt, 0);
	atomic_set(&static_hash_entry_cnt, 0);
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "iqos_cpu_cong_g<0x%p>\n", &iqos_cpu_cong_g);

	/* Init Key and Keymask Tables */
	memset(keymask_tbl, 0x0,
	       sizeof(iq_keymask_entry_t) * IQ_KEYMASKTBL_SIZE);
	memset(qos_htbl, 0x0, sizeof(iq_hash_entry_t) * IQ_HASHTBL_SIZE);

	/* clear the HW accelerator hook */
	memset(&g_hw_info, 0x0, sizeof(iq_hw_info_t));

	iqos_bind(iq_add_keymask, iq_delete_keymask, iq_add_key, iq_delete_key,
		  iq_get_key, bcm_iq_set_status, bcm_iq_flush);

	/* default entry init has to go after iqos_bind. because it uses API
	 * from kernel IQoS module for code simplicity */
	iq_default_entry_init();

	iqos_enable_g = 1;
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ,
		      "iqos_enable_g<0x%p> qos_htbl<0x%p> keymask_tbl<0x%p>\n",
		      &iqos_enable_g, &qos_htbl[0], &keymask_tbl[0]);

	printk("\nBroadcom Ingress QoS %s initialized\n", IQ_VERSION);

	return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function	 : iq_exit
 * Description  : Destruction of Ingress QoS subsystem.
 *------------------------------------------------------------------------------
 */
void iq_exit(void)
{
	iq_drv_destruct();
	iqos_bind(NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	iqos_enable_g = 0;
	printk("\nBroadcom Ingress QoS uninitialized\n");
}


EXPORT_SYMBOL(iqos_enet_status_hook_g);
EXPORT_SYMBOL(iqos_xtm_status_hook_g);
EXPORT_SYMBOL(iqos_fap_ethRxDqmQueue_hook_g);
EXPORT_SYMBOL(iqos_fap_xtmRxDqmQueue_hook_g);

module_init(iq_init);
module_exit(iq_exit);
MODULE_DESCRIPTION(IQ_MODNAME);
MODULE_VERSION(IQ_VERSION);
MODULE_LICENSE("Proprietary");

