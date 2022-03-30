/* SPDX-License-Identifier: GPL-2.0+
 *
 *	Copyright 2019 Broadcom Ltd.
 */

#ifndef _BCMBCA_XRD_API_H_
#define _BCMBCA_XRD_API_H_

int bcmbca_xrdp_init(void);
int bcmbca_xrdp_send(void *buffer, int length, uint8_t tx_port);
int bcmbca_xrdp_recv(uint8_t **buffer, uint16_t *length, uint8_t *rx_port);

extern int dt_probe(void);
extern uint32_t dt_get_ports_num(void);
extern mac_dev_t *dt_get_mac_dev(uint32_t port);
extern phy_dev_t *dt_get_phy_dev(uint32_t port);

#endif
