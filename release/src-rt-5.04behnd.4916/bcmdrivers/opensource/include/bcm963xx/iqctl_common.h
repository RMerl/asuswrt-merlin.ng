#ifndef __IQCTL_COMMON_H_INCLUDED__
#define __IQCTL_COMMON_H_INCLUDED__


/*
 *
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
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
 * File Name : iqctl_common.h
 *
 *******************************************************************************
 */

#define IQ_NAME			"ingqos"
#define IQ_DRV_NAME		IQ_NAME
#define IQ_NUM_DEVICES		1
#define IQ_DRV_DEVICE_NAME	"/dev/" IQ_DRV_NAME

#define IQCTL_ERROR		(-1)
#define IQCTL_SUCCESS	 	0

/*
 * Ioctl definitions.
 */
typedef enum {
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
 * processor. Hence start all IOCTL values from 100 to prevent conflicts
 */
	IQCTL_IOCTL_SYS = 100,
	IQCTL_IOCTL_MAX
} iqctl_ioctl_t;

typedef enum {
	IQCTL_SUBSYS_STATUS,
	IQCTL_SUBSYS_KEY,
	IQCTL_SUBSYS_KEYMASK,
	IQCTL_SUBSYS_HW_ACCEL_CONG_CTRL,
	IQCTL_SUBSYS_MAX
} iqctl_subsys_t;

typedef enum {
	IQCTL_OP_GET,
	IQCTL_OP_SET,
	IQCTL_OP_ADD,
	IQCTL_OP_REM,
	IQCTL_OP_DUMP,
	IQCTL_OP_FLUSH,
	IQCTL_OP_GETNEXT,
	IQCTL_OP_MAX
} iqctl_op_t;

typedef enum {
	IQCTL_PROTO_TCP,
	IQCTL_PROTO_UDP,
	IQCTL_PROTO_MAX
} iqctl_proto_t;

typedef enum {
	IQCTL_PROTOTYPE_IP,
	IQCTL_PROTOTYPE_MAX
} iqctl_prototype_t;

typedef enum {
	IQCTL_ENT_DYN,
	IQCTL_ENT_STAT,
	IQCTL_ENT_MAX
} iqctl_ent_t;

typedef enum {
	IQCTL_PRIO_LOW,
	IQCTL_PRIO_HIGH,
	IQCTL_PRIO_MAX
} iqctl_prio_t;

typedef enum {
	IQCTL_CONG_STATUS_LO,
	IQCTL_CONG_STATUS_HI,
	IQCTL_CONG_STATUS_MAX
} iqctl_cong_status_t;

typedef enum {
	IQCTL_STATUS_DISABLE,
	IQCTL_STATUS_ENABLE,
	IQCTL_STATUS_MAX
} iqctl_status_t;

typedef enum {
	IQCTL_KEY_FIELD_INGRESS_DEVICE,
	IQCTL_KEY_FIELD_SRC_MAC,
	IQCTL_KEY_FIELD_DST_MAC,
	IQCTL_KEY_FIELD_ETHER_TYPE,
	IQCTL_KEY_FIELD_OUTER_VID,
	IQCTL_KEY_FIELD_OUTER_PBIT,
	IQCTL_KEY_FIELD_INNER_VID,
	IQCTL_KEY_FIELD_INNER_PBIT,
	/* This will be the outermost L2 Proto other than VLAN */
	IQCTL_KEY_FIELD_L2_PROTO,
	/* The first L3 protocol, i.e., usually IPv4, IPv6 */
	IQCTL_KEY_FIELD_L3_PROTO,
	IQCTL_KEY_FIELD_IP_PROTO,
	IQCTL_KEY_FIELD_SRC_IP,
	IQCTL_KEY_FIELD_DST_IP,
	IQCTL_KEY_FIELD_DSCP,
	IQCTL_KEY_FIELD_IPV6_FLOW_LABEL,
	IQCTL_KEY_FIELD_SRC_PORT,
	IQCTL_KEY_FIELD_DST_PORT,
	IQCTL_KEY_FIELD_OFFSET_0,
	IQCTL_KEY_FIELD_OFFSET_1,
	IQCTL_KEY_FIELD_MAX,
} iqctl_key_field_t;

typedef enum {
	IQCTL_ACTION_NOP,
	IQCTL_ACTION_PRIO,
	IQCTL_ACTION_DROP,
	IQCTL_ACTION_DST_Q,
	IQCTL_ACTION_TRAP,
	IQCTL_ACTION_MAX,
} iqctl_action_t;

typedef struct {
	unsigned int key_field_mask;
	int ingress_device;
	char src_mac[6];  /* FIXME!! use define! */
	char dst_mac[6];  /* FIXME!! use define! */
	int eth_type;
	int outer_vid;
	int outer_pbit;
	int inner_vid;
	int inner_pbit;
	int l2_proto;
	int l3_proto;
	int ip_proto;
	int is_ipv6;
	int src_ip[4];
	int dst_ip[4];
	int dscp;
	int flow_label;
	int l4_src_port;
	int l4_dst_port;
	int l2_offset;
	int l3_offset;
	int l4_offset;
} iqctl_key_data_t;

typedef struct {
	iqctl_subsys_t subsys;
	iqctl_op_t op;
	int rc;
	iqctl_status_t status;
	iqctl_key_data_t key_data;
	iqctl_ent_t ent;
	int prio;
	int action;
	int action_value;
	int nextIx;
	int refcnt;
	int hitcnt;
	int stacnt;
	int dyncnt;
	iqctl_prototype_t prototype;
	int protoval;
} iqctl_data_t;


#endif  /* defined(__IQCTL_COMMON_H_INCLUDED__) */
