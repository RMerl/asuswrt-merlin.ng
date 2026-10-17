/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * NC-SI PHY
 *
 * Copyright (C) 2019, IBM Corporation.
 */

#include <phy.h>

bool ncsi_active(void);
void ncsi_receive(struct ethernet_hdr *et, struct ip_udp_hdr *ip,
		  unsigned int len);
void ncsi_probe_packages(void);
