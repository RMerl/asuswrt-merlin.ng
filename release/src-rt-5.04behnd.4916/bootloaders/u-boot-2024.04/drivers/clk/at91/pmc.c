// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <asm/io.h>
#include <clk-uclass.h>
#include <linux/clk-provider.h>
#include "pmc.h"

static int at91_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count != 2) {
		debug("AT91: clk: Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	clk->id = AT91_TO_CLK_ID(args->args[0], args->args[1]);

	return 0;
}

const struct clk_ops at91_clk_ops = {
	.of_xlate	= at91_clk_of_xlate,
	.set_rate	= ccf_clk_set_rate,
	.get_rate	= ccf_clk_get_rate,
	.enable		= ccf_clk_enable,
	.disable	= ccf_clk_disable,
};

/**
 * pmc_read() - read content at address base + off into val
 *
 * @base: base address
 * @off: offset to read from
 * @val: where the content of base + off is stored
 *
 * @return: void
 */
void pmc_read(void __iomem *base, unsigned int off, unsigned int *val)
{
	*val = readl(base + off);
}

/**
 * pmc_write() - write content of val at address base + off
 *
 * @base: base address
 * @off: offset to write to
 * @val: content to be written at base + off
 *
 * @return: void
 */
void pmc_write(void __iomem *base, unsigned int off, unsigned int val)
{
	writel(val, base + off);
}

/**
 * pmc_update_bits() - update a set of bits at address base + off
 *
 * @base: base address
 * @off: offset to be updated
 * @mask: mask of bits to be updated
 * @bits: the new value to be updated
 *
 * @return: void
 */
void pmc_update_bits(void __iomem *base, unsigned int off,
		     unsigned int mask, unsigned int bits)
{
	unsigned int tmp;

	tmp = readl(base + off);
	tmp &= ~mask;
	writel(tmp | (bits & mask), base + off);
}

/**
 * at91_clk_mux_val_to_index() - get parent index in mux table
 *
 * @table: clock mux table
 * @num_parents: clock number of parents
 * @val: clock id who's mux index should be retrieved
 *
 * @return: clock index in mux table or a negative error number in case of
 *		failure
 */
int at91_clk_mux_val_to_index(const u32 *table, u32 num_parents, u32 val)
{
	int i;

	if (!table || !num_parents)
		return -EINVAL;

	for (i = 0; i < num_parents; i++) {
		if (table[i] == val)
			return i;
	}

	return -EINVAL;
}

/**
 * at91_clk_mux_index_to_val() - get parent ID corresponding to an entry in
 *	clock's mux table
 *
 * @table: clock's mux table
 * @num_parents: clock's number of parents
 * @index: index in mux table which clock's ID should be retrieved
 *
 * @return: clock ID or a negative error number in case of failure
 */
int at91_clk_mux_index_to_val(const u32 *table, u32 num_parents, u32 index)
{
	if (!table || !num_parents || index < 0 || index > num_parents)
		return -EINVAL;

	return table[index];
}

int at91_clk_setup(const struct pmc_clk_setup *setup, int size)
{
	struct clk *c, *parent;
	int i, ret;

	if (!size)
		return 0;

	if (!setup)
		return -EINVAL;

	for (i = 0; i < size; i++) {
		ret = clk_get_by_id(setup[i].cid, &c);
		if (ret)
			return ret;

		if (setup[i].pid) {
			ret = clk_get_by_id(setup[i].pid, &parent);
			if (ret)
				return ret;

			ret = clk_set_parent(c, parent);
			if (ret)
				return ret;

			if (setup[i].prate) {
				ret = clk_set_rate(parent, setup[i].prate);
				if (ret < 0)
					return ret;
			}
		}

		if (setup[i].rate) {
			ret = clk_set_rate(c, setup[i].rate);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}
