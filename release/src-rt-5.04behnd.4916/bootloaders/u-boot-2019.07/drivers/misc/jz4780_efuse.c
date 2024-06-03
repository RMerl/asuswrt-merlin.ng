// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4780 EFUSE driver
 *
 * Copyright (c) 2014 Imagination Technologies
 * Author: Alex Smith <alex.smith@imgtec.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <errno.h>
#include <mach/jz4780.h>
#include <wait_bit.h>

#define EFUSE_EFUCTRL			0xd0
#define EFUSE_EFUCFG			0xd4
#define EFUSE_EFUSTATE			0xd8
#define EFUSE_EFUDATA(n)		(0xdc + ((n) * 4))

#define EFUSE_EFUCTRL_RD_EN		BIT(0)
#define EFUSE_EFUCTRL_LEN_BIT		16
#define EFUSE_EFUCTRL_LEN_MASK		0x1f
#define EFUSE_EFUCTRL_ADDR_BIT		21
#define EFUSE_EFUCTRL_ADDR_MASK		0x1ff
#define EFUSE_EFUCTRL_CS		BIT(30)

#define EFUSE_EFUCFG_RD_STROBE_BIT	16
#define EFUSE_EFUCFG_RD_STROBE_MASK	0xf
#define EFUSE_EFUCFG_RD_ADJ_BIT		20
#define EFUSE_EFUCFG_RD_ADJ_MASK	0xf

#define EFUSE_EFUSTATE_RD_DONE		BIT(0)

static void jz4780_efuse_read_chunk(size_t addr, size_t count, u8 *buf)
{
	void __iomem *regs = (void __iomem *)NEMC_BASE;
	size_t i;
	u32 val;
	int ret;

	val = EFUSE_EFUCTRL_RD_EN |
		((count - 1) << EFUSE_EFUCTRL_LEN_BIT) |
		(addr << EFUSE_EFUCTRL_ADDR_BIT) |
		((addr > 0x200) ? EFUSE_EFUCTRL_CS : 0);
	writel(val, regs + EFUSE_EFUCTRL);

	ret = wait_for_bit_le32(regs + EFUSE_EFUSTATE,
				EFUSE_EFUSTATE_RD_DONE, true, 10000, false);
	if (ret)
		return;

	if ((count % 4) == 0) {
		for (i = 0; i < count / 4; i++) {
			val = readl(regs + EFUSE_EFUDATA(i));
			put_unaligned(val, (u32 *)(buf + (i * 4)));
		}
	} else {
		val = readl(regs + EFUSE_EFUDATA(0));
		if (count > 2)
			buf[2] = (val >> 16) & 0xff;
		if (count > 1)
			buf[1] = (val >> 8) & 0xff;
		buf[0] = val & 0xff;
	}
}

static inline int jz4780_efuse_chunk_size(size_t count)
{
	if (count >= 32)
		return 32;
	else if ((count / 4) > 0)
		return (count / 4) * 4;
	else
		return count % 4;
}

void jz4780_efuse_read(size_t addr, size_t count, u8 *buf)
{
	size_t chunk;

	while (count > 0) {
		chunk = jz4780_efuse_chunk_size(count);
		jz4780_efuse_read_chunk(addr, chunk, buf);
		addr += chunk;
		buf += chunk;
		count -= chunk;
	}
}

void jz4780_efuse_init(u32 ahb2_rate)
{
	void __iomem *regs = (void __iomem *)NEMC_BASE;
	u32 rd_adj, rd_strobe, tmp;

	rd_adj = (((6500 * (ahb2_rate / 1000000)) / 1000000) + 0xf) / 2;
	tmp = (((35000 * (ahb2_rate / 1000000)) / 1000000) - 4) - rd_adj;
	rd_strobe = ((tmp + 0xf) / 2 < 7) ? 7 : (tmp + 0xf) / 2;

	tmp = (rd_adj << EFUSE_EFUCFG_RD_ADJ_BIT) |
	      (rd_strobe << EFUSE_EFUCFG_RD_STROBE_BIT);
	writel(tmp, regs + EFUSE_EFUCFG);
}
