// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Nexell
 * Youngbok, Park <park@nexell.co.kr>
 */

#include <linux/types.h>
#include <asm/io.h>
#include <asm/arch/nexell.h>
#include <asm/arch/sec_reg.h>
#include <linux/linkage.h>

#define NEXELL_SMC_BASE			0x82000000

#define NEXELL_SMC_FN(n)		(NEXELL_SMC_BASE +  (n))

#define NEXELL_SMC_SEC_REG_WRITE	NEXELL_SMC_FN(0x0)
#define NEXELL_SMC_SEC_REG_READ		NEXELL_SMC_FN(0x1)

#define SECURE_ID_SHIFT			8

#define SEC_4K_OFFSET			((4 * 1024) - 1)
#define SEC_64K_OFFSET			((64 * 1024) - 1)

asmlinkage int __invoke_nexell_fn_smc(u32, u32, u32, u32);

int write_sec_reg_by_id(void __iomem *reg, int val, int id)
{
	int ret = 0;
	u32 off = 0;

	switch (id) {
	case NEXELL_L2C_SEC_ID:
	case NEXELL_MIPI_SEC_ID:
	case NEXELL_TOFF_SEC_ID:
		off = (u32)reg & SEC_4K_OFFSET;
		break;
	case NEXELL_MALI_SEC_ID:
		off = (u32)reg & SEC_64K_OFFSET;
		break;
	}
	ret = __invoke_nexell_fn_smc(NEXELL_SMC_SEC_REG_WRITE |
			((1 << SECURE_ID_SHIFT) + id), off, val, 0);
	return ret;
}

int read_sec_reg_by_id(void __iomem *reg, int id)
{
	int ret = 0;
	u32 off = 0;

	switch (id) {
	case NEXELL_L2C_SEC_ID:
	case NEXELL_MIPI_SEC_ID:
	case NEXELL_TOFF_SEC_ID:
		off = (u32)reg & SEC_4K_OFFSET;
		break;
	case NEXELL_MALI_SEC_ID:
		off = (u32)reg & SEC_64K_OFFSET;
		break;
	}
	ret = __invoke_nexell_fn_smc(NEXELL_SMC_SEC_REG_READ |
			((1 << SECURE_ID_SHIFT) + id), off, 0, 0);
	return ret;
}

int write_sec_reg(void __iomem *reg, int val)
{
	int ret = 0;

	ret = __invoke_nexell_fn_smc(NEXELL_SMC_SEC_REG_WRITE,
				     (u32)reg, val, 0);
	return ret;
}

int read_sec_reg(void __iomem *reg)
{
	int ret = 0;

	ret = __invoke_nexell_fn_smc(NEXELL_SMC_SEC_REG_READ, (u32)reg, 0, 0);
	return ret;
}
