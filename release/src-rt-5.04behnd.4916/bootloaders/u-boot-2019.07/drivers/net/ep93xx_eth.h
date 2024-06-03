/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
 *
 * Copyright (C) 2004, 2005
 * Cory T. Tusar, Videon Central, Inc., <ctusar@videon-central.com>
 */

#ifndef _EP93XX_ETH_H
#define _EP93XX_ETH_H

#include <net.h>

/**
 * #define this to dump device status and queue info during initialization and
 * following errors.
 */
#undef EP93XX_MAC_DEBUG

/**
 * Number of descriptor and status entries in our RX queues.
 * It must be power of 2 !
 */
#define NUMRXDESC		PKTBUFSRX

/**
 * Number of descriptor and status entries in our TX queues.
 */
#define NUMTXDESC		1

/**
 * 944 = (1024 - 64) - 16, Fifo size - Minframesize - 16 (Chip FACT)
 */
#define TXSTARTMAX		944

/**
 * Receive descriptor queue entry
 */
struct rx_descriptor {
	uint32_t word1;
	uint32_t word2;
};

/**
 * Receive status queue entry
 */
struct rx_status {
	uint32_t word1;
	uint32_t word2;
};

#define RX_STATUS_RWE(rx_status) ((rx_status->word1 >> 30) & 0x01)
#define RX_STATUS_RFP(rx_status) ((rx_status->word1 >> 31) & 0x01)
#define RX_STATUS_FRAME_LEN(rx_status) (rx_status->word2 & 0xFFFF)

/**
 * Transmit descriptor queue entry
 */
struct tx_descriptor {
	uint32_t word1;
	uint32_t word2;
};

#define TX_DESC_EOF (1 << 31)

/**
 * Transmit status queue entry
 */
struct tx_status {
	uint32_t word1;
};

#define TX_STATUS_TXWE(tx_status) (((tx_status)->word1 >> 30) & 0x01)
#define TX_STATUS_TXFP(tx_status) (((tx_status)->word1 >> 31) & 0x01)

/**
 * Transmit descriptor queue
 */
struct tx_descriptor_queue {
	struct tx_descriptor *base;
	struct tx_descriptor *current;
	struct tx_descriptor *end;
};

/**
 * Transmit status queue
 */
struct tx_status_queue {
	struct tx_status *base;
	volatile struct tx_status *current;
	struct tx_status *end;
};

/**
 * Receive descriptor queue
 */
struct rx_descriptor_queue {
	struct rx_descriptor *base;
	struct rx_descriptor *current;
	struct rx_descriptor *end;
};

/**
 * Receive status queue
 */
struct rx_status_queue {
	struct rx_status *base;
	volatile struct rx_status *current;
	struct rx_status *end;
};

/**
 * EP93xx MAC private data structure
 */
struct ep93xx_priv {
	struct rx_descriptor_queue	rx_dq;
	struct rx_status_queue		rx_sq;
	void				*rx_buffer[NUMRXDESC];

	struct tx_descriptor_queue	tx_dq;
	struct tx_status_queue		tx_sq;

	struct mac_regs			*regs;
};

#endif
