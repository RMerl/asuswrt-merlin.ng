// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Xilinx, Inc.
 * Siva Durga Prasad Paladugu <siva.durga.prasad.paladugu@amd.com>>
 */

#include <common.h>
#include <log.h>
#include <asm/cache.h>
#include <asm/ptrace.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <clk.h>
#include <dm.h>
#include <asm/arch/sys_proto.h>
#include <zynqmp_firmware.h>
#include <linux/err.h>

#define MAX_PARENT			100
#define MAX_NODES			6
#define MAX_NAME_LEN			50

#define CLK_TYPE_SHIFT			2

#define PM_API_PAYLOAD_LEN		3

#define NA_PARENT			0xFFFFFFFF
#define DUMMY_PARENT			0xFFFFFFFE

#define CLK_TYPE_FIELD_LEN		4
#define CLK_TOPOLOGY_NODE_OFFSET	16
#define NODES_PER_RESP			3

#define CLK_TYPE_FIELD_MASK		0xF
#define CLK_FLAG_FIELD_MASK		GENMASK(21, 8)
#define CLK_TYPE_FLAG_FIELD_MASK	GENMASK(31, 24)
#define CLK_TYPE_FLAG2_FIELD_MASK	GENMASK(7, 4)
#define CLK_TYPE_FLAG_BITS		8

#define CLK_PARENTS_ID_LEN		16
#define CLK_PARENTS_ID_MASK		0xFFFF

#define END_OF_TOPOLOGY_NODE		1
#define END_OF_PARENTS			1

#define CLK_VALID_MASK			0x1
#define NODE_CLASS_SHIFT		26U
#define NODE_SUBCLASS_SHIFT		20U
#define NODE_TYPE_SHIFT			14U
#define NODE_INDEX_SHIFT		0U

#define CLK_GET_NAME_RESP_LEN		16
#define CLK_GET_TOPOLOGY_RESP_WORDS	3
#define CLK_GET_PARENTS_RESP_WORDS	3
#define CLK_GET_ATTR_RESP_WORDS		1

#define NODE_SUBCLASS_CLOCK_PLL	1
#define NODE_SUBCLASS_CLOCK_OUT	2
#define NODE_SUBCLASS_CLOCK_REF	3

#define NODE_CLASS_CLOCK	2
#define NODE_CLASS_MASK		0x3F

#define CLOCK_NODE_TYPE_MUX	1
#define CLOCK_NODE_TYPE_DIV	4
#define CLOCK_NODE_TYPE_GATE	6

#define PM_CLK_REF_CLK		(0x830c06aU)
#define PM_CLK_PL_ALT_REF_CLK	(0x830c06bU)
#define PM_CLK_MUXED_IRO	(0x830c06cU)
#define PM_CLK_EMIO		(0x830c071U)

#define TOPOLOGY_TYPE_FIXEDFACTOR	0x3

enum clk_type {
	CLK_TYPE_OUTPUT,
	CLK_TYPE_EXTERNAL,
};

struct clock_parent {
	char name[MAX_NAME_LEN];
	int id;
	u32 flag;
};

struct clock_topology {
	u32 type;
	u32 flag;
	u32 type_flag;
};

struct versal_clock {
	char clk_name[MAX_NAME_LEN];
	u32 valid;
	enum clk_type type;
	struct clock_topology node[MAX_NODES];
	u32 num_nodes;
	struct clock_parent parent[MAX_PARENT];
	u32 num_parents;
	u32 clk_id;
};

struct versal_clk_priv {
	struct versal_clock *clk;
};

static ulong pl_alt_ref_clk;
static ulong ref_clk;

struct versal_pm_query_data {
	u32 qid;
	u32 arg1;
	u32 arg2;
	u32 arg3;
};

static struct versal_clock *clock;
static unsigned int clock_max_idx;

#define PM_QUERY_DATA	35

static int versal_pm_query(struct versal_pm_query_data qdata, u32 *ret_payload)
{
	struct pt_regs regs;

	regs.regs[0] = PM_SIP_SVC | PM_QUERY_DATA;
	regs.regs[1] = ((u64)qdata.arg1 << 32) | qdata.qid;
	regs.regs[2] = ((u64)qdata.arg3 << 32) | qdata.arg2;

	smc_call(&regs);

	if (ret_payload) {
		ret_payload[0] = (u32)regs.regs[0];
		ret_payload[1] = upper_32_bits(regs.regs[0]);
		ret_payload[2] = (u32)regs.regs[1];
		ret_payload[3] = upper_32_bits(regs.regs[1]);
		ret_payload[4] = (u32)regs.regs[2];
	}

	return qdata.qid == PM_QID_CLOCK_GET_NAME ? 0 : regs.regs[0];
}

static inline int versal_is_valid_clock(u32 clk_id)
{
	if (clk_id >= clock_max_idx)
		return -ENODEV;

	return clock[clk_id].valid;
}

static int versal_get_clock_name(u32 clk_id, char *clk_name)
{
	int ret;

	ret = versal_is_valid_clock(clk_id);
	if (ret == 1) {
		strncpy(clk_name, clock[clk_id].clk_name, MAX_NAME_LEN);
		return 0;
	}

	return ret == 0 ? -EINVAL : ret;
}

static int versal_get_clock_type(u32 clk_id, u32 *type)
{
	int ret;

	ret = versal_is_valid_clock(clk_id);
	if (ret == 1) {
		*type = clock[clk_id].type;
		return 0;
	}

	return ret == 0 ? -EINVAL : ret;
}

static int versal_pm_clock_get_num_clocks(u32 *nclocks)
{
	struct versal_pm_query_data qdata = {0};
	u32 ret_payload[PAYLOAD_ARG_CNT];
	int ret;

	qdata.qid = PM_QID_CLOCK_GET_NUM_CLOCKS;

	ret = versal_pm_query(qdata, ret_payload);
	*nclocks = ret_payload[1];

	return ret;
}

static int versal_pm_clock_get_name(u32 clock_id, char *name)
{
	struct versal_pm_query_data qdata = {0};
	u32 ret_payload[PAYLOAD_ARG_CNT];
	int ret;

	qdata.qid = PM_QID_CLOCK_GET_NAME;
	qdata.arg1 = clock_id;

	ret = versal_pm_query(qdata, ret_payload);
	if (ret)
		return ret;
	memcpy(name, ret_payload, CLK_GET_NAME_RESP_LEN);

	return 0;
}

static int versal_pm_clock_get_topology(u32 clock_id, u32 index, u32 *topology)
{
	struct versal_pm_query_data qdata = {0};
	u32 ret_payload[PAYLOAD_ARG_CNT];
	int ret;

	qdata.qid = PM_QID_CLOCK_GET_TOPOLOGY;
	qdata.arg1 = clock_id;
	qdata.arg2 = index;

	ret = versal_pm_query(qdata, ret_payload);
	memcpy(topology, &ret_payload[1], CLK_GET_TOPOLOGY_RESP_WORDS * 4);

	return ret;
}

static int versal_pm_clock_get_parents(u32 clock_id, u32 index, u32 *parents)
{
	struct versal_pm_query_data qdata = {0};
	u32 ret_payload[PAYLOAD_ARG_CNT];
	int ret;

	qdata.qid = PM_QID_CLOCK_GET_PARENTS;
	qdata.arg1 = clock_id;
	qdata.arg2 = index;

	ret = versal_pm_query(qdata, ret_payload);
	memcpy(parents, &ret_payload[1], CLK_GET_PARENTS_RESP_WORDS * 4);

	return ret;
}

static int versal_pm_clock_get_attributes(u32 clock_id, u32 *attr)
{
	struct versal_pm_query_data qdata = {0};
	u32 ret_payload[PAYLOAD_ARG_CNT];
	int ret;

	qdata.qid = PM_QID_CLOCK_GET_ATTRIBUTES;
	qdata.arg1 = clock_id;

	ret = versal_pm_query(qdata, ret_payload);
	memcpy(attr, &ret_payload[1], CLK_GET_ATTR_RESP_WORDS * 4);

	return ret;
}

static int __versal_clock_get_topology(struct clock_topology *topology,
				       u32 *data, u32 *nnodes)
{
	int i;

	for (i = 0; i < PM_API_PAYLOAD_LEN; i++) {
		if (!(data[i] & CLK_TYPE_FIELD_MASK))
			return END_OF_TOPOLOGY_NODE;
		topology[*nnodes].type = data[i] & CLK_TYPE_FIELD_MASK;
		topology[*nnodes].flag = FIELD_GET(CLK_FLAG_FIELD_MASK,
						   data[i]);
		topology[*nnodes].type_flag =
				FIELD_GET(CLK_TYPE_FLAG_FIELD_MASK, data[i]);
		topology[*nnodes].type_flag |=
			FIELD_GET(CLK_TYPE_FLAG2_FIELD_MASK, data[i]) <<
			CLK_TYPE_FLAG_BITS;
		debug("topology type:0x%x, flag:0x%x, type_flag:0x%x\n",
		      topology[*nnodes].type, topology[*nnodes].flag,
		      topology[*nnodes].type_flag);
		(*nnodes)++;
	}

	return 0;
}

static int versal_clock_get_topology(u32 clk_id,
				     struct clock_topology *topology,
				     u32 *num_nodes)
{
	int j, ret;
	u32 pm_resp[PM_API_PAYLOAD_LEN] = {0};

	*num_nodes = 0;
	for (j = 0; j <= MAX_NODES; j += 3) {
		ret = versal_pm_clock_get_topology(clock[clk_id].clk_id, j,
						   pm_resp);
		if (ret)
			return ret;
		ret = __versal_clock_get_topology(topology, pm_resp, num_nodes);
		if (ret == END_OF_TOPOLOGY_NODE)
			return 0;
	}

	return 0;
}

static int __versal_clock_get_parents(struct clock_parent *parents, u32 *data,
				      u32 *nparent)
{
	int i;
	struct clock_parent *parent;

	for (i = 0; i < PM_API_PAYLOAD_LEN; i++) {
		if (data[i] == NA_PARENT)
			return END_OF_PARENTS;

		parent = &parents[i];
		parent->id = data[i] & CLK_PARENTS_ID_MASK;
		if (data[i] == DUMMY_PARENT) {
			strcpy(parent->name, "dummy_name");
			parent->flag = 0;
		} else {
			parent->flag = data[i] >> CLK_PARENTS_ID_LEN;
			if (versal_get_clock_name(parent->id, parent->name))
				continue;
		}
		debug("parent name:%s\n", parent->name);
		*nparent += 1;
	}

	return 0;
}

static int versal_clock_get_parents(u32 clk_id, struct clock_parent *parents,
				    u32 *num_parents)
{
	int j = 0, ret;
	u32 pm_resp[PM_API_PAYLOAD_LEN] = {0};

	*num_parents = 0;
	do {
		/* Get parents from firmware */
		ret = versal_pm_clock_get_parents(clock[clk_id].clk_id, j,
						  pm_resp);
		if (ret)
			return ret;

		ret = __versal_clock_get_parents(&parents[j], pm_resp,
						 num_parents);
		if (ret == END_OF_PARENTS)
			return 0;
		j += PM_API_PAYLOAD_LEN;
	} while (*num_parents <= MAX_PARENT);

	return 0;
}

static u32 versal_clock_get_div(u32 clk_id)
{
	u32 ret_payload[PAYLOAD_ARG_CNT];
	u32 div;

	xilinx_pm_request(PM_CLOCK_GETDIVIDER, clk_id, 0, 0, 0, ret_payload);
	div = ret_payload[1];

	return div;
}

static u32 versal_clock_set_div(u32 clk_id, u32 div)
{
	u32 ret_payload[PAYLOAD_ARG_CNT];

	xilinx_pm_request(PM_CLOCK_SETDIVIDER, clk_id, div, 0, 0, ret_payload);

	return div;
}

static u64 versal_clock_get_ref_rate(u32 clk_id)
{
	if (clk_id == PM_CLK_REF_CLK || clk_id == PM_CLK_MUXED_IRO || clk_id == PM_CLK_EMIO)
		return ref_clk;
	else if (clk_id == PM_CLK_PL_ALT_REF_CLK)
		return pl_alt_ref_clk;
	else
		return 0;
}

static int versal_clock_get_fixed_factor_rate(u32 clock_id, u32 parent_id)
{
	struct versal_pm_query_data qdata = {0};
	u32 ret_payload[PAYLOAD_ARG_CNT];
	u32 mult, div;
	u32 parent_rate;
	int ret;

	qdata.qid = PM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS;
	qdata.arg1 = clock_id;

	ret = versal_pm_query(qdata, ret_payload);
	if (ret)
		return ret;

	mult = ret_payload[1];
	div = ret_payload[2];

	parent_rate = versal_clock_get_ref_rate(parent_id);
	return parent_rate * mult / div;

}

static u32 versal_clock_mux(u32 clk_id)
{
	int i;
	u32 id = clk_id & 0xFFF;

	for (i = 0; i < clock[id].num_nodes; i++)
		if (clock[id].node[i].type == CLOCK_NODE_TYPE_MUX)
			return 1;

	return 0;
}

static u32 versal_clock_get_parentid(u32 clk_id)
{
	u32 parent_id = 0;
	u32 ret_payload[PAYLOAD_ARG_CNT];
	u32 id = clk_id & 0xFFF;

	if (versal_clock_mux(clk_id)) {
		xilinx_pm_request(PM_CLOCK_GETPARENT, clk_id, 0, 0, 0,
				  ret_payload);
		parent_id = ret_payload[1];
	}

	debug("parent_id:0x%x\n", clock[clock[id].parent[parent_id].id].clk_id);
	return clock[clock[id].parent[parent_id].id].clk_id;
}

static u64 versal_clock_get_pll_rate(u32 clk_id)
{
	u32 ret_payload[PAYLOAD_ARG_CNT];
	u32 fbdiv;
	u32 res;
	u32 frac;
	u64 freq;
	u32 parent_rate, parent_id, parent_ref_clk_id;
	u32 id = clk_id & 0xFFF;

	xilinx_pm_request(PM_CLOCK_GETSTATE, clk_id, 0, 0, 0, ret_payload);
	res = ret_payload[1];
	if (!res) {
		printf("0%x PLL not enabled\n", clk_id);
		return 0;
	}

	parent_id = clock[clock[id].parent[0].id].clk_id;
	parent_ref_clk_id = versal_clock_get_parentid(parent_id);
	parent_rate = versal_clock_get_ref_rate(parent_ref_clk_id);

	xilinx_pm_request(PM_CLOCK_GETDIVIDER, clk_id, 0, 0, 0, ret_payload);
	fbdiv = ret_payload[1];
	xilinx_pm_request(PM_CLOCK_PLL_GETPARAM, clk_id, 2, 0, 0, ret_payload);
	frac = ret_payload[1];

	freq = (fbdiv * parent_rate) >> (1 << frac);

	return freq;
}

static u32 versal_clock_gate(u32 clk_id)
{
	u32 id = clk_id & 0xFFF;
	int i;

	for (i = 0; i < clock[id].num_nodes; i++)
		if (clock[id].node[i].type == CLOCK_NODE_TYPE_GATE)
			return 1;

	return 0;
}

static u32 versal_clock_div(u32 clk_id)
{
	int i;
	u32 id = clk_id & 0xFFF;

	for (i = 0; i < clock[id].num_nodes; i++)
		if (clock[id].node[i].type == CLOCK_NODE_TYPE_DIV)
			return 1;

	return 0;
}

static u32 versal_clock_pll(u32 clk_id, u64 *clk_rate)
{
	if (((clk_id >> NODE_SUBCLASS_SHIFT) & NODE_CLASS_MASK) ==
	    NODE_SUBCLASS_CLOCK_PLL &&
	    ((clk_id >> NODE_CLASS_SHIFT) & NODE_CLASS_MASK) ==
	    NODE_CLASS_CLOCK) {
		*clk_rate = versal_clock_get_pll_rate(clk_id);
		return 1;
	}

	return 0;
}

static u64 versal_clock_calc(u32 clk_id)
{
	u32 parent_id;
	u64 clk_rate;
	u32 div;
	struct clock_topology topology;

	if (versal_clock_pll(clk_id, &clk_rate))
		return clk_rate;

	parent_id = versal_clock_get_parentid(clk_id);
	if (((parent_id >> NODE_SUBCLASS_SHIFT) &
	     NODE_CLASS_MASK) == NODE_SUBCLASS_CLOCK_REF) {
		topology = clock[clk_id & 0x3FF].node[0];
		if (topology.type == TOPOLOGY_TYPE_FIXEDFACTOR)
			return versal_clock_get_fixed_factor_rate(clk_id, parent_id);
		return versal_clock_get_ref_rate(parent_id);
	}

	if (!parent_id)
		return 0;

	clk_rate = versal_clock_calc(parent_id);

	if (versal_clock_div(clk_id)) {
		div = versal_clock_get_div(clk_id);
		clk_rate =  DIV_ROUND_CLOSEST(clk_rate, div);
	}

	return clk_rate;
}

static int versal_clock_get_rate(u32 clk_id, u64 *clk_rate)
{
	if (((clk_id >>  NODE_SUBCLASS_SHIFT) &
	     NODE_CLASS_MASK) == NODE_SUBCLASS_CLOCK_REF)
		*clk_rate = versal_clock_get_ref_rate(clk_id);

	if (versal_clock_pll(clk_id, clk_rate))
		return 0;

	if (((clk_id >> NODE_SUBCLASS_SHIFT) &
	     NODE_CLASS_MASK) == NODE_SUBCLASS_CLOCK_OUT &&
	    ((clk_id >> NODE_CLASS_SHIFT) &
	     NODE_CLASS_MASK) == NODE_CLASS_CLOCK) {
		if (!versal_clock_gate(clk_id) && !versal_clock_mux(clk_id))
			return -EINVAL;
		*clk_rate = versal_clock_calc(clk_id);
		return 0;
	}

	return 0;
}

#if IS_ENABLED(CONFIG_CMD_CLK)
static void versal_clk_dump(struct udevice __always_unused *dev)
{
	u64 clk_rate = 0;
	u32 type, ret, i = 0;

	printf("\n ****** VERSAL CLOCKS *****\n");

	printf("pl_alt_ref_clk:%ld ref_clk:%ld\n", pl_alt_ref_clk, ref_clk);
	for (i = 0; i < clock_max_idx; i++) {
		debug("%s\n", clock[i].clk_name);
		ret = versal_get_clock_type(i, &type);
		if (ret || type != CLK_TYPE_OUTPUT)
			continue;

		ret = versal_clock_get_rate(clock[i].clk_id, &clk_rate);

		if (ret != -EINVAL)
			printf("clk: %s  freq:%lld\n",
			       clock[i].clk_name, clk_rate);
	}
}
#endif

static void versal_get_clock_info(void)
{
	int i, ret;
	u32 attr, type = 0, nodetype, subclass, class;

	for (i = 0; i < clock_max_idx; i++) {
		ret = versal_pm_clock_get_attributes(i, &attr);
		if (ret)
			continue;

		clock[i].valid = attr & CLK_VALID_MASK;

		/* skip query for Invalid clock */
		ret = versal_is_valid_clock(i);
		if (ret != CLK_VALID_MASK)
			continue;

		clock[i].type = ((attr >> CLK_TYPE_SHIFT) & 0x1) ?
				CLK_TYPE_EXTERNAL : CLK_TYPE_OUTPUT;
		nodetype = (attr >> NODE_TYPE_SHIFT) & NODE_CLASS_MASK;
		subclass = (attr >> NODE_SUBCLASS_SHIFT) & NODE_CLASS_MASK;
		class = (attr >> NODE_CLASS_SHIFT) & NODE_CLASS_MASK;

		clock[i].clk_id = (class << NODE_CLASS_SHIFT) |
				  (subclass << NODE_SUBCLASS_SHIFT) |
				  (nodetype << NODE_TYPE_SHIFT) |
				  (i << NODE_INDEX_SHIFT);

		ret = versal_pm_clock_get_name(clock[i].clk_id,
					       clock[i].clk_name);
		if (ret)
			continue;
		debug("clk name:%s, Valid:%d, type:%d, clk_id:0x%x\n",
		      clock[i].clk_name, clock[i].valid,
		      clock[i].type, clock[i].clk_id);
	}

	/* Get topology of all clock */
	for (i = 0; i < clock_max_idx; i++) {
		ret = versal_get_clock_type(i, &type);
		if (ret || type != CLK_TYPE_OUTPUT)
			continue;
		debug("clk name:%s\n", clock[i].clk_name);
		ret = versal_clock_get_topology(i, clock[i].node,
						&clock[i].num_nodes);
		if (ret)
			continue;

		ret = versal_clock_get_parents(i, clock[i].parent,
					       &clock[i].num_parents);
		if (ret)
			continue;
	}
}

static int versal_clock_setup(void)
{
	int ret;

	ret = versal_pm_clock_get_num_clocks(&clock_max_idx);
	if (ret)
		return ret;

	debug("%s, clock_max_idx:0x%x\n", __func__, clock_max_idx);
	clock = calloc(clock_max_idx, sizeof(*clock));
	if (!clock)
		return -ENOMEM;

	versal_get_clock_info();

	return 0;
}

static int versal_clock_get_freq_by_name(char *name, struct udevice *dev,
					 ulong *freq)
{
	struct clk clk;
	int ret;

	ret = clk_get_by_name(dev, name, &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get %s\n", name);
		return ret;
	}

	*freq = clk_get_rate(&clk);
	if (IS_ERR_VALUE(*freq)) {
		dev_err(dev, "failed to get rate %s\n", name);
		return -EINVAL;
	}

	return 0;
}

static int versal_clk_probe(struct udevice *dev)
{
	int ret;
	struct versal_clk_priv *priv = dev_get_priv(dev);

	debug("%s\n", __func__);

	ret = versal_clock_get_freq_by_name("pl_alt_ref_clk",
					    dev, &pl_alt_ref_clk);
	if (ret < 0)
		return -EINVAL;

	ret = versal_clock_get_freq_by_name("ref_clk", dev, &ref_clk);
	if (ret < 0)
		return -EINVAL;

	ret = versal_clock_setup();
	if (ret < 0)
		return ret;

	priv->clk = clock;

	return ret;
}

static ulong versal_clk_get_rate(struct clk *clk)
{
	struct versal_clk_priv *priv = dev_get_priv(clk->dev);
	u32 id = clk->id;
	u32 clk_id;
	u64 clk_rate = 0;

	debug("%s\n", __func__);

	clk_id = priv->clk[id].clk_id;

	versal_clock_get_rate(clk_id, &clk_rate);

	return clk_rate;
}

static ulong versal_clk_set_rate(struct clk *clk, ulong rate)
{
	struct versal_clk_priv *priv = dev_get_priv(clk->dev);
	u32 id = clk->id;
	u32 clk_id;
	u64 clk_rate = 0;
	u32 div;
	int ret;

	debug("%s\n", __func__);

	clk_id = priv->clk[id].clk_id;

	ret = versal_clock_get_rate(clk_id, &clk_rate);
	if (ret) {
		printf("Clock is not a Gate:0x%x\n", clk_id);
		return 0;
	}

	do {
		if (versal_clock_div(clk_id)) {
			div = versal_clock_get_div(clk_id);
			clk_rate *= div;
			div = DIV_ROUND_CLOSEST(clk_rate, rate);
			versal_clock_set_div(clk_id, div);
			debug("%s, div:%d, newrate:%lld\n", __func__,
			      div, DIV_ROUND_CLOSEST(clk_rate, div));
			return DIV_ROUND_CLOSEST(clk_rate, div);
		}
		clk_id = versal_clock_get_parentid(clk_id);
	} while (((clk_id >> NODE_SUBCLASS_SHIFT) &
		 NODE_CLASS_MASK) != NODE_SUBCLASS_CLOCK_REF);

	printf("Clock didn't has Divisors:0x%x\n", priv->clk[id].clk_id);

	return clk_rate;
}

static int versal_clk_enable(struct clk *clk)
{
	struct versal_clk_priv *priv = dev_get_priv(clk->dev);
	u32 clk_id;

	clk_id = priv->clk[clk->id].clk_id;

	if (versal_clock_gate(clk_id))
		return xilinx_pm_request(PM_CLOCK_ENABLE, clk_id, 0, 0, 0, NULL);

	return 0;
}

static struct clk_ops versal_clk_ops = {
	.set_rate = versal_clk_set_rate,
	.get_rate = versal_clk_get_rate,
	.enable = versal_clk_enable,
#if IS_ENABLED(CONFIG_CMD_CLK)
	.dump = versal_clk_dump,
#endif
};

static const struct udevice_id versal_clk_ids[] = {
	{ .compatible = "xlnx,versal-clk" },
	{ }
};

U_BOOT_DRIVER(versal_clk) = {
	.name = "versal-clk",
	.id = UCLASS_CLK,
	.of_match = versal_clk_ids,
	.probe = versal_clk_probe,
	.ops = &versal_clk_ops,
	.priv_auto	= sizeof(struct versal_clk_priv),
};
