/*
 * (C) 2009 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_cluster.h>

static void
cluster_help(void)
{
	printf(
"cluster match options:\n"
"  --cluster-total-nodes <num>		Set number of total nodes in cluster\n"
"  [!] --cluster-local-node <num>	Set the local node number\n"
"  [!] --cluster-local-nodemask <num>	Set the local node mask\n"
"  --cluster-hash-seed <num>		Set seed value of the Jenkins hash\n");
}

enum {
	O_CL_TOTAL_NODES = 0,
	O_CL_LOCAL_NODE,
	O_CL_LOCAL_NODEMASK,
	O_CL_HASH_SEED,
	F_CL_TOTAL_NODES    = 1 << O_CL_TOTAL_NODES,
	F_CL_LOCAL_NODE     = 1 << O_CL_LOCAL_NODE,
	F_CL_LOCAL_NODEMASK = 1 << O_CL_LOCAL_NODEMASK,
	F_CL_HASH_SEED      = 1 << O_CL_HASH_SEED,
};

#define s struct xt_cluster_match_info
static const struct xt_option_entry cluster_opts[] = {
	{.name = "cluster-total-nodes", .id = O_CL_TOTAL_NODES,
	 .type = XTTYPE_UINT32, .min = 1, .max = XT_CLUSTER_NODES_MAX,
	 .flags = XTOPT_MAND | XTOPT_PUT, XTOPT_POINTER(s, total_nodes)},
	{.name = "cluster-local-node", .id = O_CL_LOCAL_NODE,
	 .excl = F_CL_LOCAL_NODEMASK, .flags = XTOPT_INVERT,
	 .type = XTTYPE_UINT32, .min = 1, .max = XT_CLUSTER_NODES_MAX},
	{.name = "cluster-local-nodemask", .id = O_CL_LOCAL_NODEMASK,
	 .excl = F_CL_LOCAL_NODE, .type = XTTYPE_UINT32,
	 .min = 1, .max = XT_CLUSTER_NODES_MAX,
	 .flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, node_mask)},
	{.name = "cluster-hash-seed", .id = O_CL_HASH_SEED,
	 .type = XTTYPE_UINT32, .flags = XTOPT_MAND | XTOPT_PUT,
	 XTOPT_POINTER(s, hash_seed)},
	XTOPT_TABLEEND,
};

static void cluster_parse(struct xt_option_call *cb)
{
	struct xt_cluster_match_info *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_CL_LOCAL_NODE:
		if (cb->invert)
			info->flags |= XT_CLUSTER_F_INV;
		info->node_mask = 1 << (cb->val.u32 - 1);
		break;
	case O_CL_LOCAL_NODEMASK:
		if (cb->invert)
			info->flags |= XT_CLUSTER_F_INV;
		break;
	}
}

static void cluster_check(struct xt_fcheck_call *cb)
{
	const struct xt_cluster_match_info *info = cb->data;
	unsigned int test;

	test = F_CL_TOTAL_NODES | F_CL_LOCAL_NODE | F_CL_HASH_SEED;
	if ((cb->xflags & test) == test) {
		if (info->node_mask >= (1ULL << info->total_nodes))
			xtables_error(PARAMETER_PROBLEM,
				      "cluster match: "
				      "`--cluster-local-node' "
				      "must be <= `--cluster-total-nodes'");
		return;
	}

	test = F_CL_TOTAL_NODES | F_CL_LOCAL_NODEMASK | F_CL_HASH_SEED;
	if ((cb->xflags & test) == test) {
		if (info->node_mask >= (1ULL << info->total_nodes))
			xtables_error(PARAMETER_PROBLEM,
				      "cluster match: "
				      "`--cluster-local-nodemask' too big "
				      "for `--cluster-total-nodes'");
		return;
	}
	if (!(cb->xflags & (F_CL_LOCAL_NODE | F_CL_LOCAL_NODEMASK)))
		xtables_error(PARAMETER_PROBLEM,
			      "cluster match: `--cluster-local-node' or"
			      "`--cluster-local-nodemask' is missing");
}

static void
cluster_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_cluster_match_info *info = (void *)match->data;

	printf(" cluster ");
	if (info->flags & XT_CLUSTER_F_INV)
		printf("!node_mask=0x%08x", info->node_mask);
	else
		printf("node_mask=0x%08x", info->node_mask);

	printf(" total_nodes=%u hash_seed=0x%08x",
		info->total_nodes, info->hash_seed);
}

static void
cluster_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_cluster_match_info *info = (void *)match->data;

	if (info->flags & XT_CLUSTER_F_INV)
		printf(" ! --cluster-local-nodemask 0x%08x", info->node_mask);
	else
		printf(" --cluster-local-nodemask 0x%08x", info->node_mask);

	printf(" --cluster-total-nodes %u --cluster-hash-seed 0x%08x",
		info->total_nodes, info->hash_seed);
}

static int cluster_xlate(struct xt_xlate *xl,
			 const struct xt_xlate_mt_params *params)
{
	int node, shift_value = 1, comma_needed = 0;
	uint32_t temp_node_mask, node_id = 0, needs_set = 0;
	const struct xt_cluster_match_info *info = (void *)params->match->data;
	const char *jhash_st = "jhash ct original saddr mod";
	const char *pkttype_st = "meta pkttype set host";

	if (!(info->node_mask & (info->node_mask - 1))) {
		if (info->node_mask <= 2)
			xt_xlate_add(xl, "%s %u seed 0x%08x eq %u %s", jhash_st,
					info->total_nodes, info->hash_seed,
					info->node_mask, pkttype_st);
		else {
			temp_node_mask = info->node_mask;
			while (1) {
				temp_node_mask = temp_node_mask >> shift_value;
				node_id++;
				if (temp_node_mask == 0)
					break;
			}
			xt_xlate_add(xl, "%s %u seed 0x%08x eq %u %s", jhash_st,
					info->total_nodes, info->hash_seed,
					node_id, pkttype_st);
		}
	} else {
		xt_xlate_add(xl, "%s %u seed 0x%08x ", jhash_st,
				info->total_nodes, info->hash_seed);
		for (node = 0; node < 32; node++) {
			if (info->node_mask & (1u << node)) {
				if (needs_set == 0) {
					xt_xlate_add(xl, "{ ");
					needs_set = 1;
				}

				if (comma_needed)
					xt_xlate_add(xl, ", ");
				xt_xlate_add(xl, "%u", node);
				comma_needed++;
			}
		}
		if (needs_set)
			xt_xlate_add(xl, " }");
		xt_xlate_add(xl, " %s", pkttype_st);
	}

	return 1;
}

static struct xtables_match cluster_mt_reg = {
	.family		= NFPROTO_UNSPEC,
	.name		= "cluster",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_cluster_match_info)),
	.userspacesize  = XT_ALIGN(sizeof(struct xt_cluster_match_info)),
 	.help		= cluster_help,
	.print		= cluster_print,
	.save		= cluster_save,
	.x6_parse	= cluster_parse,
	.x6_fcheck	= cluster_check,
	.x6_options	= cluster_opts,
	.xlate		= cluster_xlate,
};

void _init(void)
{
	xtables_register_match(&cluster_mt_reg);
}
