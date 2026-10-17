/* SPDX-License-Identifier: GPL-2.0+
 *
 * (C) Copyright 2016 Nexell
 * Youngbok, Park <park@nexell.co.kr>
 */

#define NEXELL_L2C_SEC_ID	0
#define NEXELL_MALI_SEC_ID	2
#define NEXELL_MIPI_SEC_ID	4
#define NEXELL_TOFF_SEC_ID	6

int write_sec_reg_by_id(void __iomem *reg, int val, int id);
int read_sec_reg_by_id(void __iomem *reg, int id);
int read_sec_reg(void __iomem *reg);
int write_sec_reg(void __iomem *reg, int val);
