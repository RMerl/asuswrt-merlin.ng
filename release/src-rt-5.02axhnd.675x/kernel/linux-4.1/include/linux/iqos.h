#if defined(CONFIG_BCM_KF_NBUFF)
#ifndef __IQOS_H_INCLUDED__
#define __IQOS_H_INCLUDED__

/*
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
#define IQOS_VERSION	"v1.0"
#define IQOS_VER_STR	IQOS_VERSION
#define IQOS_MODNAME	"Broadcom IQoS "

#include <linux/if_ether.h>

typedef enum {
	IQOS_PARAM_TYPE_KEYMASK,
	IQOS_PARAM_TYPE_KEY,
	IQOS_PARAM_TYPE_MAX
} iqos_param_type_t;

typedef enum {
	IQOS_FIELD_INGRESS_DEVICE,
	IQOS_FIELD_SRC_MAC,
	IQOS_FIELD_DST_MAC,
	IQOS_FIELD_ETHER_TYPE,
	IQOS_FIELD_OUTER_VID,
	IQOS_FIELD_OUTER_PBIT,
	IQOS_FIELD_INNER_VID,
	IQOS_FIELD_INNER_PBIT,
	IQOS_FIELD_L2_PROTO,
	IQOS_FIELD_L3_PROTO,
	IQOS_FIELD_IP_PROTO,
	IQOS_FIELD_SRC_IP,
	IQOS_FIELD_DST_IP,
	IQOS_FIELD_DSCP,
	IQOS_FIELD_IPV6_FLOW_LABEL,
	IQOS_FIELD_SRC_PORT,
	IQOS_FIELD_DST_PORT,
	IQOS_FIELD_OFFSET_0,
	IQOS_FIELD_OFFSET_START = IQOS_FIELD_OFFSET_0,
	IQOS_FIELD_OFFSET_0_TYPE,
	IQOS_FIELD_OFFSET_0_START,
	IQOS_FIELD_OFFSET_0_SIZE,
	IQOS_FIELD_OFFSET_0_MASK,
	IQOS_FIELD_OFFSET_1,
	IQOS_FIELD_OFFSET_1_TYPE,
	IQOS_FIELD_OFFSET_1_START,
	IQOS_FIELD_OFFSET_1_SIZE,
	IQOS_FIELD_OFFSET_1_MASK,
	IQOS_FIELD_OFFSET_END = IQOS_FIELD_OFFSET_1_MASK,
	IQOS_FIELD_MAX
} iqos_field_t;

typedef enum {
	IQOS_ACTION_NOP,
	IQOS_ACTION_PRIO,
	IQOS_ACTION_DROP,
	IQOS_ACTION_DST_Q,
	IQOS_ACTION_TRAP,
	IQOS_ACTION_MAX
} iqos_action_t;

typedef enum {
	IQOS_OFFSET_TYPE_L2,
	IQOS_OFFSET_TYPE_L3,
	IQOS_OFFSET_TYPE_L4,
	IQOS_OFFSET_TYPE_MAX
} iqos_offset_type_t;

typedef struct {
	uint32_t type;
	uint32_t start;
	uint32_t size;
	uint32_t mask;
} iqos_offset_data_t;

#define IQOS_PACKET_CACHE_MAX_SIZE	128
typedef struct {
	uint32_t ingress_device;
	uint8_t src_mac[ETH_HLEN];
	uint8_t dst_mac[ETH_HLEN];
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
	uint8_t packet_cache[IQOS_PACKET_CACHE_MAX_SIZE];
} iqos_data_t;

typedef struct {
	uint32_t param_type;
	uint8_t prio;
	uint8_t type;
	uint32_t field_mask;
	uint32_t action;
	uint32_t action_value;
	iqos_data_t data;
	iqos_offset_data_t offset0;
	iqos_offset_data_t offset1;
} iqos_param_t;

typedef enum {
	IQOS_ENT_DYN,
	IQOS_ENT_STAT,
	IQOS_ENT_MAX
} iqos_ent_t;

typedef enum {
	IQOS_PRIO_LOW,
	IQOS_PRIO_HIGH,
	IQOS_PRIO_MAX
} iqos_prio_t;

typedef enum {
	IQOS_CONG_STATUS_LO,
	IQOS_CONG_STATUS_HI,
	IQOS_CONG_STATUS_MAX
} iqos_cong_status_t;

typedef enum {
	IQOS_STATUS_DISABLE,
	IQOS_STATUS_ENABLE,
	IQOS_STATUS_MAX
} iqos_status_t;

typedef struct {
	uint8_t ipProto;
	uint16_t destPort;
	iqos_ent_t ent;
	iqos_prio_t prio;
} iqos_config_t;

typedef int (*iqos_common_hook_t)(iqos_param_t *param);
typedef void (*iqos_void_hook_t)(void);
typedef int (*iqos_int_hook_t)(uint32_t val);

/* the original APIs that are backward supported by the new driver/module */
int iqos_add_L4port(uint8_t ipProto, uint16_t destPort, iqos_ent_t ent,
		    iqos_prio_t prio);
int iqos_rem_L4port(uint8_t ipProto, uint16_t destPort, iqos_ent_t ent);
int iqos_prio_L4port(uint8_t ipProto, uint16_t destPort);

/* the new APIs */

/* APIs for setting up keymask:
 * WARNING!! one will have to perform iqos_flush() to delete all the dynamic
 * entry before making any change of the keymasks.
 * Deleting a keymask that has key refer to it will fail and return error */
int iqos_keymask_param_start(iqos_param_t *param);
int iqos_keymask_param_field_set(iqos_param_t *param, uint32_t field, uint32_t *val_ptr);
int iqos_keymask_commit_and_add(iqos_param_t *param, uint8_t prio);
int iqos_keymask_commit_and_delete(iqos_param_t *param);

/* APIs for setting up key,
 * example can be found in iqos_[add/rem/prio]_L4port */
int iqos_key_param_start(iqos_param_t *param);
int iqos_key_param_field_set(iqos_param_t *param, uint32_t field,
			     uint32_t *val_ptr, uint32_t val_size);
int iqos_key_param_action_set(iqos_param_t *param, uint32_t action,
			      uint32_t value);
int iqos_key_commit_and_add(iqos_param_t *param, uint8_t type);
int iqos_key_commit_and_delete(iqos_param_t *param, uint8_t type);
int iqos_key_commit_and_get(iqos_param_t *param);

/* API to flush all the dynamic entries, and
 * delete keymask if no key refers to it */
void iqos_flush(void);

/* API to set the status for IQOS to enabled(1) or disabled(0) */
int iqos_set_status(uint32_t status);

void iqos_bind(iqos_common_hook_t iqos_add_keymask,
	       iqos_common_hook_t iqos_rem_keymask,
	       iqos_common_hook_t iqos_add_key,
	       iqos_common_hook_t iqos_rem_key,
	       iqos_common_hook_t iqos_get_key,
	       iqos_int_hook_t iqos_set_status,
	       iqos_void_hook_t iqos_flush);


#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define IQOS_LOCK_IRQSAVE()         spin_lock_irqsave( &iqos_cong_lock_g, flags )
#define IQOS_UNLOCK_IRQRESTORE()    spin_unlock_irqrestore( &iqos_cong_lock_g, flags )
#define IQOS_LOCK_BH()              spin_lock_bh( &iqos_lock_g )
#define IQOS_UNLOCK_BH()            spin_unlock_bh( &iqos_lock_g )
#else
#define IQOS_LOCK_IRQSAVE()		local_irq_save(flags)
#define IQOS_UNLOCK_IRQRESTORE()	local_irq_restore(flags)
#define IQOS_LOCK_BH()			NULL_STMT
#define IQOS_UNLOCK_BH()		NULL_STMT
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#define IQOS_RXCHNL_MAX		4
#define IQOS_RXCHNL_DISABLED	0
#define IQOS_RXCHNL_ENABLED	1
#define IQOS_MAX_RX_RING_SIZE	4096

typedef enum {
	IQOS_IF_ENET,
	IQOS_IF_ENET_RXCHNL0 = IQOS_IF_ENET,
	IQOS_IF_ENET_RXCHNL1,
	IQOS_IF_ENET_RXCHNL2,
	IQOS_IF_ENET_RXCHNL3,
	IQOS_IF_XTM,
	IQOS_IF_XTM_RXCHNL0 = IQOS_IF_XTM,
	IQOS_IF_XTM_RXCHNL1,
	IQOS_IF_XTM_RXCHNL2,
	IQOS_IF_XTM_RXCHNL3,
	IQOS_IF_FWD,
	IQOS_IF_FWD_RXCHNL0 = IQOS_IF_FWD,
	IQOS_IF_FWD_RXCHNL1,
	IQOS_IF_WL,
	IQOS_IF_USB,
	IQOS_IF_MAX,
} iqos_if_t;

iqos_cong_status_t iqos_get_sys_cong_status(void);
iqos_cong_status_t iqos_get_cong_status(iqos_if_t iface, uint32_t chnl);
uint32_t iqos_set_cong_status(iqos_if_t iface, uint32_t chnl,
			      iqos_cong_status_t status);
#endif
#endif /* defined(__IQOS_H_INCLUDED__) */
#endif
