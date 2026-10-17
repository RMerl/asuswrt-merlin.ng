// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com
 *  Author: Peter Ujfalusi <peter.ujfalusi@ti.com>
 */

#include <linux/kernel.h>

#include "k3-psil-priv.h"

#define PSIL_ETHERNET(x)				\
	{						\
		.thread_id = x,				\
		.ep_config = {				\
			.ep_type = PSIL_EP_NATIVE,	\
			.pkt_mode = 1,			\
			.needs_epib = 1,		\
			.psd_size = 16,			\
		},					\
	}

/* PSI-L source thread IDs, used for RX (DMA_DEV_TO_MEM) */
static struct psil_ep am654_src_ep_map[] = {
	/* PRU_ICSSG0 */
	PSIL_ETHERNET(0x4100),
	PSIL_ETHERNET(0x4101),
	PSIL_ETHERNET(0x4102),
	PSIL_ETHERNET(0x4103),
	/* PRU_ICSSG1 */
	PSIL_ETHERNET(0x4200),
	PSIL_ETHERNET(0x4201),
	PSIL_ETHERNET(0x4202),
	PSIL_ETHERNET(0x4203),
	/* PRU_ICSSG2 */
	PSIL_ETHERNET(0x4300),
	PSIL_ETHERNET(0x4301),
	PSIL_ETHERNET(0x4302),
	PSIL_ETHERNET(0x4303),
	/* CPSW0 */
	PSIL_ETHERNET(0x7000),
};

/* PSI-L destination thread IDs, used for TX (DMA_MEM_TO_DEV) */
static struct psil_ep am654_dst_ep_map[] = {
	/* PRU_ICSSG0 */
	PSIL_ETHERNET(0xc100),
	/* PSIL: 0xc101 - 0xc103 unused */
	PSIL_ETHERNET(0xc104),
	/* PSIL: 0xc105 - 0xc107 unused */
	/* PRU_ICSSG1 */
	PSIL_ETHERNET(0xc200),
	/* PSIL: 0xc201 - 0xc203 unused */
	PSIL_ETHERNET(0xc204),
	/* PSIL: 0xc205 - 0xc207 unused */
	/* PRU_ICSSG2 */
	PSIL_ETHERNET(0xc300),
	/* PSIL: 0xc301 - 0xc303 unused */
	PSIL_ETHERNET(0xc304),
	/* PSIL: 0xc305 - 0xc307 unused */
	/* CPSW0 */
	PSIL_ETHERNET(0xf000),
	/* PSIL: 0xf001 - 0xf007 unused */
};

struct psil_ep_map am654_ep_map = {
	.name = "am654",
	.src = am654_src_ep_map,
	.src_count = ARRAY_SIZE(am654_src_ep_map),
	.dst = am654_dst_ep_map,
	.dst_count = ARRAY_SIZE(am654_dst_ep_map),
};
