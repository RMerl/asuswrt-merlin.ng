/*
 *
<:copyright-BRCM:2024:DUAL/GPL:standard

   Copyright (c) 2024 Broadcom 
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
 * File Name : ingqos_types.h
 *
 *******************************************************************************
 */

#ifndef __INGQOS_TYPES_H__
#define __INGQOS_TYPES_H__

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


#define IQ_DSCP_SHIFT		0
#define IQ_DSCP_WIDTH		6

#define IQ_L4DST_PORT_SHIFT	6
#define IQ_L4DST_PORT_WIDTH	16

#define IQ_IPPROTO_SHIFT	22
#define IQ_IPPROTO_WIDTH	8

#define IQ_ETHTYPE_SHIFT	30
#define IQ_ETHTYPE_WIDTH	16

#define IQ_L3_PROTO_SHIFT	46
#define IQ_L3_PROTO_WIDTH	16

#endif /* __INGQOS_TYPES_H__ */
