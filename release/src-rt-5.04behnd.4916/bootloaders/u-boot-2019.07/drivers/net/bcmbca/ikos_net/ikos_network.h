// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2021 Broadcom Corporation
   All Rights Reserved

    
*/
/*
 * ikos_network.h
 *
 *  Created on: Jun 2021
 *      Author: dima.mamut@broadcom.com
 */
#ifndef _IKOS_NETWORK_H_
#define _IKOS_NETWORK_H_

#include <common.h>
#include <dm.h>
#include <net.h>
#include <environment.h>
#include <command.h>
#include <dm/lists.h>
#include <dm/device-internal.h>

void run_ikos_net_test(int port_id, unsigned int  packet_size, int iteration);
void xrdp_eth_init(void);



#define QEMU_ACTIVE_PORT  (0)

#endif /* _IKOS_NETWORK_H_ */
