/*
 * Texas Instruments 3-Port Ethernet Switch Address Lookup Engine APIs
 *
 * Copyright (C) 2012 Texas Instruments
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __TI_CPSW_ALE_H__
#define __TI_CPSW_ALE_H__

struct cpsw_ale_params {
	struct device		*dev;
	void __iomem		*ale_regs;
	unsigned long		ale_ageout;	/* in secs */
	unsigned long		ale_entries;
	unsigned long		ale_ports;
};

struct cpsw_ale {
	struct cpsw_ale_params	params;
	struct timer_list	timer;
	unsigned long		ageout;
	int			allmulti;
};

enum cpsw_ale_control {
	/* global */
	ALE_ENABLE,
	ALE_CLEAR,
	ALE_AGEOUT,
	ALE_P0_UNI_FLOOD,
	ALE_VLAN_NOLEARN,
	ALE_NO_PORT_VLAN,
	ALE_OUI_DENY,
	ALE_BYPASS,
	ALE_RATE_LIMIT_TX,
	ALE_VLAN_AWARE,
	ALE_AUTH_ENABLE,
	ALE_RATE_LIMIT,
	/* port controls */
	ALE_PORT_STATE,
	ALE_PORT_DROP_UNTAGGED,
	ALE_PORT_DROP_UNKNOWN_VLAN,
	ALE_PORT_NOLEARN,
	ALE_PORT_NO_SA_UPDATE,
	ALE_PORT_UNKNOWN_VLAN_MEMBER,
	ALE_PORT_UNKNOWN_MCAST_FLOOD,
	ALE_PORT_UNKNOWN_REG_MCAST_FLOOD,
	ALE_PORT_UNTAGGED_EGRESS,
	ALE_PORT_BCAST_LIMIT,
	ALE_PORT_MCAST_LIMIT,
	ALE_NUM_CONTROLS,
};

enum cpsw_ale_port_state {
	ALE_PORT_STATE_DISABLE	= 0x00,
	ALE_PORT_STATE_BLOCK	= 0x01,
	ALE_PORT_STATE_LEARN	= 0x02,
	ALE_PORT_STATE_FORWARD	= 0x03,
};

/* ALE unicast entry flags - passed into cpsw_ale_add_ucast() */
#define ALE_SECURE			BIT(0)
#define ALE_BLOCKED			BIT(1)
#define ALE_SUPER			BIT(2)
#define ALE_VLAN			BIT(3)

#define ALE_PORT_HOST			BIT(0)
#define ALE_PORT_1			BIT(1)
#define ALE_PORT_2			BIT(2)

#define ALE_MCAST_FWD			0
#define ALE_MCAST_BLOCK_LEARN_FWD	1
#define ALE_MCAST_FWD_LEARN		2
#define ALE_MCAST_FWD_2			3

#define ALE_ENTRY_BITS		68
#define ALE_ENTRY_WORDS	DIV_ROUND_UP(ALE_ENTRY_BITS, 32)

struct cpsw_ale *cpsw_ale_create(struct cpsw_ale_params *params);
int cpsw_ale_destroy(struct cpsw_ale *ale);

void cpsw_ale_start(struct cpsw_ale *ale);
void cpsw_ale_stop(struct cpsw_ale *ale);

int cpsw_ale_set_ageout(struct cpsw_ale *ale, int ageout);
int cpsw_ale_flush(struct cpsw_ale *ale, int port_mask);
int cpsw_ale_flush_multicast(struct cpsw_ale *ale, int port_mask, int vid);
int cpsw_ale_add_ucast(struct cpsw_ale *ale, u8 *addr, int port,
		       int flags, u16 vid);
int cpsw_ale_del_ucast(struct cpsw_ale *ale, u8 *addr, int port,
		       int flags, u16 vid);
int cpsw_ale_add_mcast(struct cpsw_ale *ale, u8 *addr, int port_mask,
		       int flags, u16 vid, int mcast_state);
int cpsw_ale_del_mcast(struct cpsw_ale *ale, u8 *addr, int port_mask,
		       int flags, u16 vid);
int cpsw_ale_add_vlan(struct cpsw_ale *ale, u16 vid, int port, int untag,
			int reg_mcast, int unreg_mcast);
int cpsw_ale_del_vlan(struct cpsw_ale *ale, u16 vid, int port);
void cpsw_ale_set_allmulti(struct cpsw_ale *ale, int allmulti);

int cpsw_ale_control_get(struct cpsw_ale *ale, int port, int control);
int cpsw_ale_control_set(struct cpsw_ale *ale, int port,
			 int control, int value);
void cpsw_ale_dump(struct cpsw_ale *ale, u32 *data);

#endif
