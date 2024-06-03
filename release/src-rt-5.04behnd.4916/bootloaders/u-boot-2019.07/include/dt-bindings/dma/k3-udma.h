// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com
 */

#ifndef __DT_TI_UDMA_H
#define __DT_TI_UDMA_H

#define UDMA_TR_MODE		0
#define UDMA_PKT_MODE		1

#define UDMA_DIR_TX		0
#define UDMA_DIR_RX		1

#define PSIL_STATIC_TR_NONE	0
#define PSIL_STATIC_TR_XY	1
#define PSIL_STATIC_TR_MCAN	2

#define UDMA_PDMA_TR_XY(id)				\
	ti,psil-config##id {				\
		linux,udma-mode = <UDMA_TR_MODE>;	\
		statictr-type = <PSIL_STATIC_TR_XY>;	\
	}

#define UDMA_PDMA_PKT_XY(id)				\
	ti,psil-config##id {				\
		linux,udma-mode = <UDMA_PKT_MODE>;	\
		statictr-type = <PSIL_STATIC_TR_XY>;	\
	}

#endif /* __DT_TI_UDMA_H */
