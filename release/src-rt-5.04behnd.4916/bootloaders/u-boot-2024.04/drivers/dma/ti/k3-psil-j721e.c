// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2019-2023 Texas Instruments Incorporated - https://www.ti.com
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
static struct psil_ep j721e_src_ep_map[] = {
	/* MCU_CPSW0 */
	PSIL_ETHERNET(0x7000),
	/* MAIN_CPSW0 */
	PSIL_ETHERNET(0x4a00),
};

/* PSI-L destination thread IDs, used for TX (DMA_MEM_TO_DEV) */
static struct psil_ep j721e_dst_ep_map[] = {
	/* MCU_CPSW0 */
	PSIL_ETHERNET(0xf000),
	PSIL_ETHERNET(0xf001),
	PSIL_ETHERNET(0xf002),
	PSIL_ETHERNET(0xf003),
	PSIL_ETHERNET(0xf004),
	PSIL_ETHERNET(0xf005),
	PSIL_ETHERNET(0xf006),
	PSIL_ETHERNET(0xf007),
	/* MAIN_CPSW0 */
	PSIL_ETHERNET(0xca00),
	PSIL_ETHERNET(0xca01),
	PSIL_ETHERNET(0xca02),
	PSIL_ETHERNET(0xca03),
	PSIL_ETHERNET(0xca04),
	PSIL_ETHERNET(0xca05),
	PSIL_ETHERNET(0xca06),
	PSIL_ETHERNET(0xca07),
};

struct psil_ep_map j721e_ep_map = {
	.name = "j721e",
	.src = j721e_src_ep_map,
	.src_count = ARRAY_SIZE(j721e_src_ep_map),
	.dst = j721e_dst_ep_map,
	.dst_count = ARRAY_SIZE(j721e_dst_ep_map),
};
