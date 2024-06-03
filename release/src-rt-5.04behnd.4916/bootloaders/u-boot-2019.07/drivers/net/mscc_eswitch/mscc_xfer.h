/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>

enum mscc_regs_qs {
	MSCC_QS_XTR_RD,
	MSCC_QS_XTR_FLUSH,
	MSCC_QS_XTR_DATA_PRESENT,
	MSCC_QS_INJ_WR,
	MSCC_QS_INJ_CTRL,
};

int mscc_send(void __iomem *regs, const unsigned long *mscc_qs_offset,
	      u32 *ifh, size_t ifh_len, u32 *buff, size_t buff_len);
int mscc_recv(void __iomem *regs, const unsigned long *mscc_qs_offset,
	      u32 *rxbuf, size_t ifh_len, bool byte_swap);
void mscc_flush(void __iomem *regs, const unsigned long *mscc_qs_offset);
