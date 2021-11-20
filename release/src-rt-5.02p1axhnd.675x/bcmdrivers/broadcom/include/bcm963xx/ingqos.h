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

#ifdef CONFIG_BLOG
#include <linux/blog.h>
#endif

#define IQ_VERSION		"v1.0"

#define IQ_VER_STR		IQ_VERSION
#define IQ_MODNAME		"Broadcom Ingress QoS Module "

/* Ingess QoS Character Device */
#define IQ_DRV_MAJOR		303

#define CC_IQ_STATS

#define IQ_KEYMASKTBL_SIZE	16
#define IQ_HASHTBL_SIZE		512
#define IQ_HASH_BIN_SIZE	4

/* the key field is sorted from port -> L2 -> L3 -> L4 -> MISC.
 * This is done for the sake of sorting the key mask table
 * where L4 result should have higher priority than L3 and on.
 */
typedef enum {
	IQ_KEY_FIELD_INGRESS_DEVICE,
	IQ_KEY_FIELD_SRC_MAC,
	IQ_KEY_FIELD_DST_MAC,
	IQ_KEY_FIELD_ETHER_TYPE,
	IQ_KEY_FIELD_OUTER_VID,
	IQ_KEY_FIELD_OUTER_PBIT,
	IQ_KEY_FIELD_INNER_VID,
	IQ_KEY_FIELD_INNER_PBIT,
	IQ_KEY_FIELD_L2_PROTO,		/* This will be the outermost L2 Proto other than VLAN */
	IQ_KEY_FIELD_L3_PROTO,		/* The first L3 protocol, i.e., usually IPv4, IPv6 */
	IQ_KEY_FIELD_IP_PROTO,
	IQ_KEY_FIELD_SRC_IP,
	IQ_KEY_FIELD_DST_IP,
	IQ_KEY_FIELD_DSCP,
	IQ_KEY_FIELD_IPV6_FLOW_LABEL,
	IQ_KEY_FIELD_SRC_PORT,
	IQ_KEY_FIELD_DST_PORT,
	IQ_KEY_FIELD_OFFSET_0,
	IQ_KEY_FIELD_OFFSET_1,
	IQ_KEY_FIELD_MAX,
} iq_key_field_t;

typedef enum {
	IQ_KEY_MASK_INGRESS_DEVICE = (1 << IQ_KEY_FIELD_INGRESS_DEVICE),
	IQ_KEY_MASK_SRC_MAC = (1 << IQ_KEY_FIELD_SRC_MAC),
	IQ_KEY_MASK_DST_MAC = (1 << IQ_KEY_FIELD_DST_MAC),
	IQ_KEY_MASK_ETHER_TYPE = (1 << IQ_KEY_FIELD_ETHER_TYPE),
	IQ_KEY_MASK_OUTER_VID = (1 << IQ_KEY_FIELD_OUTER_VID),
	IQ_KEY_MASK_OUTER_PBIT = (1 << IQ_KEY_FIELD_OUTER_PBIT),
	IQ_KEY_MASK_INNER_VID = (1 << IQ_KEY_FIELD_INNER_VID),
	IQ_KEY_MASK_INNER_PBIT = (1 << IQ_KEY_FIELD_INNER_PBIT),
	IQ_KEY_MASK_L2_PROTO = (1 << IQ_KEY_FIELD_L2_PROTO),
	IQ_KEY_MASK_L3_PROTO = (1<< IQ_KEY_FIELD_L3_PROTO),
	IQ_KEY_MASK_IP_PROTO = (1 << IQ_KEY_FIELD_IP_PROTO),
	IQ_KEY_MASK_SRC_IP = (1 << IQ_KEY_FIELD_SRC_IP),
	IQ_KEY_MASK_DST_IP = (1 << IQ_KEY_FIELD_DST_IP),
	IQ_KEY_MASK_DSCP = (1 << IQ_KEY_FIELD_DSCP),
	IQ_KEY_MASK_IPV6_FLOW_LABEL = (1 << IQ_KEY_FIELD_IPV6_FLOW_LABEL),
	IQ_KEY_MASK_SRC_PORT = (1 << IQ_KEY_FIELD_SRC_PORT),
	IQ_KEY_MASK_DST_PORT = (1 << IQ_KEY_FIELD_DST_PORT),
	IQ_KEY_MASK_OFFSET_0 = (1 << IQ_KEY_FIELD_OFFSET_0),
	IQ_KEY_MASK_OFFSET_1 = (1 << IQ_KEY_FIELD_OFFSET_1),
} iq_key_mask_t;

typedef enum {
	IQ_KEY_OFFSET_L2,
	IQ_KEY_OFFSET_L3,
	IQ_KEY_OFFSET_L4,
	IQ_KEY_OFFSET_MAX
} iq_key_offset_type_t;

typedef struct {
	union {
		uint32_t word;
		struct {
			uint32_t valid : 1;
			uint32_t offset0_type : 2;
			uint32_t offset0_start : 7;
			uint32_t offset0_size : 4; /* = real size - 1, so it covers 1 to 16 */
			uint32_t offset1_type : 2;
			uint32_t offset1_start : 7;
			uint32_t offset1_size : 4; /* = real size - 1, so it covers 1 to 16 */
			uint32_t unused : 5;
		};
	};
	uint32_t offset0_mask;
	uint32_t offset1_mask;
} iq_key_option_t;

#define IQ_PACKET_CACHE_MAX_SIZE	128
typedef struct {
	uint32_t ingress_device;
	uint8_t src_mac[BLOG_ETH_ADDR_LEN];
	uint8_t dst_mac[BLOG_ETH_ADDR_LEN];
	uint16_t eth_type;
	uint16_t outer_vid;
	uint8_t outer_pbit;
	uint16_t inner_vid;
	uint8_t inner_pbit;
	uint16_t l2_proto;
	uint16_t l3_proto;
	uint8_t ip_proto;
	uint8_t is_ipv6;
	uint32_t src_ip[4];
	uint32_t dst_ip[4];
	uint8_t dscp;
	uint32_t flow_label;
	uint16_t l4_src_port;
	uint16_t l4_dst_port;
	uint16_t l2_offset;
	uint16_t l3_offset;
	uint16_t l4_offset;
	/* used for storing part of packet buffer for offset check */
	uint8_t packet_cache[IQ_PACKET_CACHE_MAX_SIZE];
} iq_key_data_t;

typedef enum {
	IQ_ACTION_TYPE_NOP,
	IQ_ACTION_TYPE_PRIO,
	IQ_ACTION_TYPE_DROP,
	IQ_ACTION_TYPE_DST_Q,
	IQ_ACTION_TYPE_TRAP,
	IQ_ACTION_TYPE_MAX,
} iq_action_type_t;

typedef union {
	uint32_t word;
	struct {
		uint32_t valid : 1;
		uint32_t is_static : 1;
		uint32_t type : 4;
		uint32_t value : 8;
		uint32_t unused : 18;
	};
} iq_action_t;

typedef struct {
	uint32_t key_mask;
	iq_key_option_t key_opt;
	iq_key_data_t key_data;
	iq_action_t action;
	uint32_t prio;
	uint32_t status;
} iq_param_t;

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

/* It is recommneded to keep the low thresh > 50% */
/* Ethernet Ingress QoS low and high thresholds as % of Ring size */
#define IQ_ENET_LO_THRESH_PCT	66
#define IQ_ENET_HI_THRESH_PCT	75

/* Ethernet Ingress QoS low and high thresholds as % of Ring size */
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
#define IQ_XTM_LO_THRESH_PCT	66
#define IQ_XTM_HI_THRESH_PCT	75
#endif

/* CMF Fwd Ingress QoS low and high thresholds as % of Ring size */

typedef struct {
	uint32_t registered : 1;
	uint32_t enabled : 1;
	uint32_t unused : 30;
	uint32_t mask_capability;
	int (*add_entry)(void *iq_param);
	int (*delete_entry)(void *iq_param);
	int (*set_status)(void *iq_param);

	/* iq_param pointer is not currently used */
	int (*get_status)(void *iq_param);

	/* iq_param pointer is not currently used */
	int (*dump_table)(void *iq_param);

	int (*set_congestion_ctrl)(void *iq_param);
	int (*get_congestion_ctrl)(void *iq_param);
} iq_hw_info_t;

/*
 * to register HW support to Ingress QoS driver
 * Input:
 * 	hw_info: pointer to the hw_info that's described above
 * Output:
 * 	return: 0 if succeed, else otherwise
 */
int bcm_iq_register_hw(const iq_hw_info_t *hw_info);

/*
 * to unregister HW support from Ingress QoS driver
 * Input:
 * 	hw_info: pointer to the hw_info that's described above
 * Output:
 * 	return: 0 if succeed, else otherwise
 */
int bcm_iq_unregister_hw(iq_hw_info_t *hw_info);

/*
 * to add key mask into the key mask table
 * Input:
 * 	key_mask: bit mask of the field that will be used
 * 	key_opt: used when additional field is needed
 * 	prio: priority used when inserting the key mask into the table.
 * 	      The larger the value is the higher the priority is.
 * 	      If previosly added entry with the same priority exists,
 * 	      it will return error.
 * Output:
 *      return: 0 if succeed, else otherwise.
 *
 * Note: 
 *      1) iq must be flushed and disabled before adding new mask entry.
 *      2) if the same entry has been created with different priority, it will
 *         return -EEXIST, please delete the previous one
 */
int bcm_iq_add_keymask(uint32_t key_mask, iq_key_option_t *key_opt,
		       uint8_t prio);

/*
 * to delete key mask based on the info from the key mask table
 * Input:
 * 	key_mask: bit mask of the field that will be used
 * 	key_opt: used when additional field is needed
 * Output:
 *      return: 0 if succeed, else otherwise.
 */
int bcm_iq_delete_keymask(uint32_t key_mask, iq_key_option_t *key_opt);

/*
 * to delete key mask with the given index from the key mask table
 * Input:
 * 	index: the index of the key mask that will be deleted
 * Output:
 *      return: 0 if succeed, else otherwise.
 */
int bcm_iq_delete_keymask_by_index(uint32_t index);

/*
 * to add an ingress QoS entry
 * Input:
 * 	key_mask: bit mask of the field that will be used
 * 	key_opt: used when additional field is needed
 * 	key_data: contains the value of the data
 * 	action: contains the value of the action
 * Output:
 *      return: 0 if succeed, else otherwise.
 */
int bcm_iq_add_entry(uint32_t key_mask, iq_key_option_t *key_opt,
		     iq_key_data_t *key_data, iq_action_t *action);

/*
 * to delete an ingress QoS entry
 * Input:
 * 	key_mask: bit mask of the field that will be used
 * 	key_opt: used when additional field is needed
 * 	key_data: contains the value of the data
 * Output:
 *      return: 0 if succeed, else otherwise.
 */
int bcm_iq_delete_entry(uint32_t key_mask, iq_key_option_t *key_opt,
			iq_key_data_t *key_data);

/*
 * to set the status of this ingress QoS module
 * Input:
 *      status: 0: disabled, 1: enabled
 * Output:
 *      return: 0 if succeed, else otherwise
 */
int bcm_iq_set_status(uint32_t status);

/*
 * to flush all the existing entries and the mask
 */
void bcm_iq_flush(void);

/*
 * to check if there is a match of ingress QoS entry
 * Input:
 * 	key_data: contains the value of the data parsed from packet
 * Output:
 * 	action: contains the result of the action if an entry is found
 *      return: 0 if entry is found, else otherwise.
 */
int bcm_iq_check(iq_key_data_t *key_data, iq_action_t *action);

/*
 * to check if there is a match of ingress QoS entry
 * Input:
 * 	key_data: contains the value of the data parsed from packet
 * 	key_mask: contains the field mask of the data from packet
 * Output:
 * 	action: contains the result of the action if an entry is found
 *      return: 0 if entry is found, else otherwise.
 */
int bcm_iq_check_with_mask(iq_key_data_t *key_data, uint32_t key_mask,
			   iq_action_t *action);

#ifdef CONFIG_BLOG
/*
 * to check a blog if there is a match of ingress QoS entry
 * Input:
 * 	blog: a blog with all the important values filled
 * Output:
 * 	action: contains the result of the action if an entry is found
 *      return: 0 if entry is found, else otherwise.
 */
int bcm_iq_check_blog(struct blog_t *blog, iq_action_t *action);
#endif

/*
 * to check a skb if there is a match of ingress QoS entry
 * Input:
 * 	skb: sk_buff pointer, the code will parse the packet
 * Output:
 * 	action: contains the result of the action if an entry is found
 *      return: 0 if entry is found, else otherwise.
 */
int bcm_iq_check_skb(struct sk_buff *skb, iq_action_t *action);


/* function type definitions for FAP based */
typedef void (*iqos_status_hook_t)(void);
typedef uint32_t (*iqos_fap_ethRxDqmQueue_hook_t)(uint32_t chnl);
typedef uint32_t (*iqos_fap_xtmRxDqmQueue_hook_t)(uint32_t chnl);

#endif  /* defined(__INGQOS_H_INCLUDED__) */

