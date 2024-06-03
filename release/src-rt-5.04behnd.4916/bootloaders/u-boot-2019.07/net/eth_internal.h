/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2001-2015
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Joe Hershberger, National Instruments
 */

#ifndef __ETH_INTERNAL_H
#define __ETH_INTERNAL_H

/* Do init that is common to driver model and legacy networking */
void eth_common_init(void);

/**
 * eth_env_set_enetaddr_by_index() - set the MAC address environment variable
 *
 * This sets up an environment variable with the given MAC address (@enetaddr).
 * The environment variable to be set is defined by <@base_name><@index>addr.
 * If @index is 0 it is omitted. For common Ethernet this means ethaddr,
 * eth1addr, etc.
 *
 * @base_name:	Base name for variable, typically "eth"
 * @index:	Index of interface being updated (>=0)
 * @enetaddr:	Pointer to MAC address to put into the variable
 * @return 0 if OK, other value on error
 */
int eth_env_set_enetaddr_by_index(const char *base_name, int index,
				 uchar *enetaddr);

int eth_mac_skip(int index);
void eth_current_changed(void);
#ifdef CONFIG_DM_ETH
void eth_set_dev(struct udevice *dev);
#else
void eth_set_dev(struct eth_device *dev);
#endif
void eth_set_current_to_next(void);

#endif
