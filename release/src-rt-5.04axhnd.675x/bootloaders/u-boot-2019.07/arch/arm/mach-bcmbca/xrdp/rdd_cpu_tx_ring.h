// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Broadcom
 */
/*
    
*/

/*
 * cpu_tx_ring.h
 */

#ifndef _CPU_TX_RING_H_
#define _CPU_TX_RING_H_


#if defined(CONFIG_BCM63146)
int rdd_cpu_tx_new(uint8_t *buffer, uint32_t length, uint8_t tx_port);
#endif
int rdd_cpu_tx(uint32_t length, uint16_t bn0, uint16_t bn1, uint8_t bns_num, uint8_t tx_port);


#endif /* _CPU_TX_RING_H_ */
