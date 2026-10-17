/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023 MediaTek Inc.
 */

#ifndef _DT_BINDINGS_MTK_RESET_H_
#define _DT_BINDINGS_MTK_RESET_H_

/* ETHDMA Subsystem resets */
#define ETHDMA_FE_RST			6
#define ETHDMA_PMTR_RST			8
#define ETHDMA_GMAC_RST			23
#define ETHDMA_WDMA0_RST		24
#define ETHDMA_WDMA1_RST		25
#define ETHDMA_WDMA2_RST		26
#define ETHDMA_PPE0_RST			29
#define ETHDMA_PPE1_RST			30
#define ETHDMA_PPE2_RST			31

/* ETHWARP Subsystem resets */
#define ETHWARP_GSW_RST			9
#define ETHWARP_EIP197_RST		10
#define ETHWARP_WOCPU0_RST		32
#define ETHWARP_WOCPU1_RST		33
#define ETHWARP_WOCPU2_RST		34
#define ETHWARP_WOX_NET_MUX_RST		35
#define ETHWARP_WED0_RST		36
#define ETHWARP_WED1_RST		37
#define ETHWARP_WED2_RST		38

#endif /* _DT_BINDINGS_MTK_RESET_H_ */
