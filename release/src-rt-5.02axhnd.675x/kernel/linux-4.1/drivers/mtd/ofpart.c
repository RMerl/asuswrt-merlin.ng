/*
 * Flash partitions described by the OF (or flattened) device tree
 *
 * Copyright © 2006 MontaVista Software Inc.
 * Author: Vitaly Wool <vwool@ru.mvista.com>
 *
 * Revised to handle newer style flash binding by:
 *   Copyright © 2007 David Gibson, IBM Corporation.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/mtd/mtd.h>
#include <linux/slab.h>
#include <linux/mtd/partitions.h>

static bool node_has_compatible(struct device_node *pp)
{
	return of_get_property(pp, "compatible", NULL);
}

static int parse_ofpart_partitions(struct mtd_info *master,
				   struct mtd_partition **pparts,
				   struct mtd_part_parser_data *data)
{
	struct device_node *node;
	const char *partname;
	struct device_node *pp;
	int nr_parts, i;


	if (!data)
		return 0;

	node = data->of_node;
	if (!node)
		return 0;

	/* First count the subnodes */
	nr_parts = 0;
	for_each_child_of_node(node,  pp) {
		if (node_has_compatible(pp))
			continue;

		nr_parts++;
	}

	if (nr_parts == 0)
		return 0;

	*pparts = kzalloc(nr_parts * sizeof(**pparts), GFP_KERNEL);
	if (!*pparts)
		return -ENOMEM;

	i = 0;
	for_each_child_of_node(node,  pp) {
		const __be32 *reg;
		int len;
		int a_cells, s_cells;

		if (node_has_compatible(pp))
			continue;

		reg = of_get_property(pp, "reg", &len);
		if (!reg) {
			nr_parts--;
			continue;
		}

		a_cells = of_n_addr_cells(pp);
		s_cells = of_n_size_cells(pp);
		(*pparts)[i].offset = of_read_number(reg, a_cells);
		(*pparts)[i].size = of_read_number(reg + a_cells, s_cells);

		partname = of_get_property(pp, "label", &len);
		if (!partname)
			partname = of_get_property(pp, "name", &len);
		(*pparts)[i].name = partname;

		if (of_get_property(pp, "read-only", &len))
			(*pparts)[i].mask_flags |= MTD_WRITEABLE;

		if (of_get_property(pp, "lock", &len))
			(*pparts)[i].mask_flags |= MTD_POWERUP_LOCK;

		i++;
	}

	if (!i) {
		of_node_put(pp);
		pr_err("No valid partition found on %s\n", node->full_name);
		kfree(*pparts);
		*pparts = NULL;
		return -EINVAL;
	}

	return nr_parts;
}

static struct mtd_part_parser ofpart_parser = {
	.owner = THIS_MODULE,
	.parse_fn = parse_ofpart_partitions,
	.name = "ofpart",
};

static int parse_ofoldpart_partitions(struct mtd_info *master,
				      struct mtd_partition **pparts,
				      struct mtd_part_parser_data *data)
{
	struct device_node *dp;
	int i, plen, nr_parts;
	const struct {
		__be32 offset, len;
	} *part;
	const char *names;

	if (!data)
		return 0;

	dp = data->of_node;
	if (!dp)
		return 0;

	part = of_get_property(dp, "partitions", &plen);
	if (!part)
		return 0; /* No partitions found */

	pr_warning("Device tree uses obsolete partition map binding: %s\n",
			dp->full_name);

	nr_parts = plen / sizeof(part[0]);

	*pparts = kzalloc(nr_parts * sizeof(*(*pparts)), GFP_KERNEL);
	if (!*pparts)
		return -ENOMEM;

	names = of_get_property(dp, "partition-names", &plen);

	for (i = 0; i < nr_parts; i++) {
		(*pparts)[i].offset = be32_to_cpu(part->offset);
		(*pparts)[i].size   = be32_to_cpu(part->len) & ~1;
		/* bit 0 set signifies read only partition */
		if (be32_to_cpu(part->len) & 1)
			(*pparts)[i].mask_flags = MTD_WRITEABLE;

		if (names && (plen > 0)) {
			int len = strlen(names) + 1;

			(*pparts)[i].name = names;
			plen -= len;
			names += len;
		} else {
			(*pparts)[i].name = "unnamed";
		}

		part++;
	}

	return nr_parts;
}

static struct mtd_part_parser ofoldpart_parser = {
	.owner = THIS_MODULE,
	.parse_fn = parse_ofoldpart_partitions,
	.name = "ofoldpart",
};

static int __init ofpart_parser_init(void)
{
	register_mtd_parser(&ofpart_parser);
	register_mtd_parser(&ofoldpart_parser);
	return 0;
}

static void __exit ofpart_parser_exit(void)
{
	deregister_mtd_parser(&ofpart_parser);
	deregister_mtd_parser(&ofoldpart_parser);
}

module_init(ofpart_parser_init);
module_exit(ofpart_parser_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Parser for MTD partitioning information in device tree");
MODULE_AUTHOR("Vitaly Wool, David Gibson");
/*
 * When MTD core cannot find the requested parser, it tries to load the module
 * with the same name. Since we provide the ofoldpart parser, we should have
 * the corresponding alias.
 */
MODULE_ALIAS("ofoldpart");
