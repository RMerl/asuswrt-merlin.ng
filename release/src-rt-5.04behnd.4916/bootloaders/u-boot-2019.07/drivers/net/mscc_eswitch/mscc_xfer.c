// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <linux/io.h>
#include "mscc_xfer.h"

#define QS_XTR_FLUSH_FLUSH		GENMASK(1, 0)
#define QS_INJ_CTRL_GAP_SIZE(x)		((x) << 21)
#define QS_INJ_CTRL_EOF			BIT(19)
#define QS_INJ_CTRL_SOF			BIT(18)
#define QS_INJ_CTRL_VLD_BYTES(x)	((x) << 16)

#define XTR_EOF_0     ntohl(0x80000000u)
#define XTR_EOF_1     ntohl(0x80000001u)
#define XTR_EOF_2     ntohl(0x80000002u)
#define XTR_EOF_3     ntohl(0x80000003u)
#define XTR_PRUNED    ntohl(0x80000004u)
#define XTR_ABORT     ntohl(0x80000005u)
#define XTR_ESCAPE    ntohl(0x80000006u)
#define XTR_NOT_READY ntohl(0x80000007u)

#define BUF_CELL_SZ		60
#define XTR_VALID_BYTES(x)	(4 - ((x) & 3))

int mscc_send(void __iomem *regs, const unsigned long *mscc_qs_offset,
	      u32 *ifh, size_t ifh_len, u32 *buff, size_t buff_len)
{
	int i, count = (buff_len + 3) / 4, last = buff_len % 4;

	writel(QS_INJ_CTRL_GAP_SIZE(1) | QS_INJ_CTRL_SOF,
	       regs + mscc_qs_offset[MSCC_QS_INJ_CTRL]);

	for (i = 0; i < ifh_len; i++)
		writel(ifh[i], regs + mscc_qs_offset[MSCC_QS_INJ_WR]);

	for (i = 0; i < count; i++)
		writel(buff[i], regs + mscc_qs_offset[MSCC_QS_INJ_WR]);

	/* Add padding */
	while (i < (BUF_CELL_SZ / 4)) {
		writel(0, regs + mscc_qs_offset[MSCC_QS_INJ_WR]);
		i++;
	}

	/* Indicate EOF and valid bytes in last word */
	writel(QS_INJ_CTRL_GAP_SIZE(1) |
	       QS_INJ_CTRL_VLD_BYTES(buff_len < BUF_CELL_SZ ? 0 : last) |
	       QS_INJ_CTRL_EOF, regs + mscc_qs_offset[MSCC_QS_INJ_CTRL]);

	/* Add dummy CRC */
	writel(0, regs + mscc_qs_offset[MSCC_QS_INJ_WR]);

	return 0;
}

int mscc_recv(void __iomem *regs, const unsigned long *mscc_qs_offset,
	      u32 *rxbuf, size_t ifh_len, bool byte_swap)
{
	u8 grp = 0; /* Recv everything on CPU group 0 */
	int i, byte_cnt = 0;
	bool eof_flag = false, pruned_flag = false, abort_flag = false;

	if (!(readl(regs + mscc_qs_offset[MSCC_QS_XTR_DATA_PRESENT]) &
	      BIT(grp)))
		return -EAGAIN;

	/* skip IFH */
	for (i = 0; i < ifh_len; i++)
		readl(regs + mscc_qs_offset[MSCC_QS_XTR_RD]);

	while (!eof_flag) {
		u32 val = readl(regs + mscc_qs_offset[MSCC_QS_XTR_RD]);
		u32 cmp = val;

		if (byte_swap)
			cmp = ntohl(val);

		switch (cmp) {
		case XTR_NOT_READY:
			debug("%d NOT_READY...?\n", byte_cnt);
			break;
		case XTR_ABORT:
			*rxbuf = readl(regs + mscc_qs_offset[MSCC_QS_XTR_RD]);
			abort_flag = true;
			eof_flag = true;
			debug("XTR_ABORT\n");
			break;
		case XTR_EOF_0:
		case XTR_EOF_1:
		case XTR_EOF_2:
		case XTR_EOF_3:
			byte_cnt += XTR_VALID_BYTES(val);
			*rxbuf = readl(regs + mscc_qs_offset[MSCC_QS_XTR_RD]);
			eof_flag = true;
			debug("EOF\n");
			break;
		case XTR_PRUNED:
			/* But get the last 4 bytes as well */
			eof_flag = true;
			pruned_flag = true;
			debug("PRUNED\n");
			/* fallthrough */
		case XTR_ESCAPE:
			*rxbuf = readl(regs + mscc_qs_offset[MSCC_QS_XTR_RD]);
			byte_cnt += 4;
			rxbuf++;
			debug("ESCAPED\n");
			break;
		default:
			*rxbuf = val;
			byte_cnt += 4;
			rxbuf++;
		}
	}

	if (abort_flag || pruned_flag || !eof_flag) {
		debug("Discarded frame: abort:%d pruned:%d eof:%d\n",
		      abort_flag, pruned_flag, eof_flag);
		return -EAGAIN;
	}

	return byte_cnt;
}

void mscc_flush(void __iomem *regs, const unsigned long *mscc_qs_offset)
{
	/* All Queues flush */
	setbits_le32(regs + mscc_qs_offset[MSCC_QS_XTR_FLUSH],
		     QS_XTR_FLUSH_FLUSH);

	/* Allow to drain */
	mdelay(1);

	/* All Queues normal */
	clrbits_le32(regs + mscc_qs_offset[MSCC_QS_XTR_FLUSH],
		     QS_XTR_FLUSH_FLUSH);
}
