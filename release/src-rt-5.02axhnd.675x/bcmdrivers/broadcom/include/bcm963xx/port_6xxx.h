/*
<:copyright-broadcom

 Copyright (c) 2007 Broadcom Corporation
 All Rights Reserved
 No portions of this material may be reproduced in any form without the
 written permission of:
          Broadcom Corporation
          16215 Alton Parkway
          Irvine, California 92619
 All information contained in this document is Broadcom Corporation
 company private, proprietary, and trade secret.

:>
*/

#ifndef __PORT_6XXX_H
#define __PORT_6XXX_H

/* bcm_port_t */
typedef int bcm_port_t;
/* bcm_vlan_t */
typedef uint16 bcm_vlan_t;
/* bcm_cos_t */
typedef uint8 bcm_cos_t;
/* Return Value Definitions */
#define BCM_E_NONE   0;
#define BCM_E_ERROR  1;

/* Total switch ports including MIPS port */
#define TOTAL_SWITCH_PORTS  9

/* definitions for op_map */
#define PORT_REPLACE_VID    0x1
#define PORT_REPLACE_8021P  0x2
#define PORT_REPLACE_CFI    0x4
#define PORT_REPLACE_TPID   0x8
#define PORT_STRIP_TAG      0x10

/* Switch Port Tag Replacement API */
int bcm_port_replace_tag_set(int unit, bcm_port_t port, uint32 vlan_tag);
int bcm_port_replace_tag_get(int unit, bcm_port_t port, uint32 *vlan_tag);
int bcm_port_replace_tpid_set(int unit, bcm_port_t port, uint16 tpid);
int bcm_port_replace_tpid_get(int unit, bcm_port_t port, uint16 *tpid);
int bcm_port_replace_tci_set(int unit, bcm_port_t port, bcm_vlan_t tci);
int bcm_port_replace_tci_get(int unit, bcm_port_t port, bcm_vlan_t *tci);
int bcm_port_replace_vid_set(int unit, bcm_port_t port, bcm_vlan_t vid);
int bcm_port_replace_vid_get(int unit, bcm_port_t port, bcm_vlan_t *vid);
int bcm_port_replace_8021p_set(int unit, bcm_port_t port, uint8 priority);
int bcm_port_replace_8021p_get(int unit, bcm_port_t port, uint8 *priority);
int bcm_port_replace_cfi_set(int unit, bcm_port_t port, uint8 cfi);
int bcm_port_replace_cfi_get(int unit, bcm_port_t port, uint8 *cfi);
int bcm_port_tag_mangle_set(int unit, bcm_port_t port, uint8 op_map);
int bcm_port_tag_mangle_get(int unit, bcm_port_t port, uint8 *op_map);


/* definitions for type in threshold_set/get functions */
#define QUEUE_HI_HYSTERESIS_THRESHOLD 0x1
#define QUEUE_HI_PAUSE_THRESHOLD      0x2
#define QUEUE_HI_DROP_THRESHOLD       0x3
#define QUEUE_LOW_DROP_THRESHOLD      0x4
#define TOTAL_HYSTERESIS_THRESHOLD    0x5
#define TOTAL_PAUSE_THRESHOLD         0x6
#define TOTAL_DROP_THRESHOLD          0x7

/* definitions for buf_ctrl_map */
#define TX_QUEUE_PAUSE 0x1
#define TX_QUEUE_DROP  0x2
#define TOTAL_PAUSE    0x4
#define TOTAL_DROP     0x8

/* Switch Buffer Management API */
int bcm_buffer_threshold_get(int unit, bcm_cos_t priority, uint8 type, 
                             uint16 *value);
int bcm_buffer_threshold_set(int unit, bcm_cos_t priority, uint8 type, 
                             uint16 value);
int bcm_buffer_control_get(int unit, uint8 *buf_ctrl_map);
int bcm_buffer_control_set(int unit, uint8 buf_ctrl_map);

#endif
