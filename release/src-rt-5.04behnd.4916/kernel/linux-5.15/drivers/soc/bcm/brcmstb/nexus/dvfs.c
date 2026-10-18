/* System Control and Management Interface (SCMI) BRCM Protocol
 *
 * Copyright (C) 2019, Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or from the Free Software
 * Foundation at https://www.gnu.org/licenses/ .
 */

#include <linux/brcmstb/avs_dvfs.h>
#include <linux/brcmstb/brcmstb.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/pm_opp.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/string.h>

#include "../../../../firmware/arm_scmi/common.h"
#include "brcm_clk.h"

#define SCMI_PROTOCOL_BRCM 0x80
#define SCMI_MAX_STRINGLEN 116
#define MAX_PSTATES 16

#define SEQ_PRINTF(m, x...)			\
do {						\
	if (m)					\
		seq_printf(m, x);		\
	else					\
		printk(x);			\
} while (0)

enum brcm_protocol_cmd {
	BRCM_SEND_AVS_CMD = 0x3,
	BRCM_CLK_SHOW_CMD = 0x4, /* obsoleted */
	BRCM_PMAP_SHOW_CMD = 0x5,
	BRCM_CLK_SHOW_NEW_CMD = 0x6,
	BRCM_RESET_ENABLE_CMD = 0x7,
	BRCM_RESET_DISABLE_CMD = 0x8,
	BRCM_OVERTEMP_RESET_CMD = 0x9,
	BRCM_SCMI_STATS_SHOW_CMD = 0xa,
	BRCM_SCMI_STATS_RESET_CMD = 0xb,
	BRCM_TRACE_LOG_ON_CMD = 0xc,
	BRCM_GET_CLK_VER_STR = 0xd,
	BRCM_PERF_DOMAIN_ATTRIBUTES = 0xe,
	BRCM_PERF_DESCRIBE_LEVELS = 0xf,
	BRCM_PERF_LEVEL_SET = 0x10,
	BRCM_PERF_LEVEL_GET = 0x11,
	BRCM_OVERTEMP_RESET_THRESHOLD_CMD = 0x12,
};

static const char __maybe_unused *pmap_cores[BCLK_SW_NUM_CORES] = {
	[BCLK_SW_CPU0 - BCLK_SW_OFFSET] = "cpu0",
	[BCLK_SW_V3D - BCLK_SW_OFFSET] = "v3d0",
	[BCLK_SW_SYSIF - BCLK_SW_OFFSET] = "sysif0",
	[BCLK_SW_SCB - BCLK_SW_OFFSET] = "scb0",
	[BCLK_SW_HVD0 - BCLK_SW_OFFSET] = "hvd0",
	[BCLK_SW_RAAGA0 - BCLK_SW_OFFSET] = "raaga0",
	[BCLK_SW_VICE0 - BCLK_SW_OFFSET] = "vice0",
	[BCLK_SW_VICE0_PSS - BCLK_SW_OFFSET] = "vice0_pss",
	[BCLK_SW_VICE1 - BCLK_SW_OFFSET] = "vice1",
	[BCLK_SW_VICE1_PSS - BCLK_SW_OFFSET] = "vice1_pss",
	[BCLK_SW_XPT - BCLK_SW_OFFSET] = "xpt",
	[BCLK_SW_M2MC0 - BCLK_SW_OFFSET] = "m2mc0",
	[BCLK_SW_M2MC1 - BCLK_SW_OFFSET] = "m2mc1",
	[BCLK_SW_MIPMAP0 - BCLK_SW_OFFSET] = "mipmap0",
	[BCLK_SW_TSX0 - BCLK_SW_OFFSET] = "tsx0",
	[BCLK_SW_SMARTCARD0 - BCLK_SW_OFFSET] = "sc0",
	[BCLK_SW_SMARTCARD1 - BCLK_SW_OFFSET] = "sc1",
	[BCLK_SW_VPU0 - BCLK_SW_OFFSET] = "vpu0",
	[BCLK_SW_BNE0 - BCLK_SW_OFFSET] = "bne0",
	[BCLK_SW_ASP0 - BCLK_SW_OFFSET] = "asp0",
	[BCLK_SW_HVD_CABAC0 - BCLK_SW_OFFSET] = "hvd_cabac0",
	[BCLK_SW_AXI0 - BCLK_SW_OFFSET] = "axi0",
	[BCLK_SW_BSTM0 - BCLK_SW_OFFSET] = "bstm0",
	[BCLK_SW_CPU1 - BCLK_SW_OFFSET] = "cpu1",
	[BCLK_SW_CPU2 - BCLK_SW_OFFSET] = "cpu2",
	[BCLK_SW_CPU3 - BCLK_SW_OFFSET] = "cpu3",
	[BCLK_SW_DVPHT_CORE - BCLK_SW_OFFSET] = "dvpht_core",
	[BCLK_SW_HVS0 - BCLK_SW_OFFSET] = "hvs0",
};

static struct scmi_protocol_handle *bph;
static struct platform_device *cpufreq_dev;


static struct brcm_core_info {
	unsigned int num_pstates;
	unsigned int *freqs;
} g_perf_cores[BCLK_SW_NUM_CORES];

static int avs_ret_to_linux_ret(int avs_ret)
{
	int ret;

	/* Convert firmware errors to errno's as much as possible. */
	switch (avs_ret) {
	case AVS_STATUS_SUCCESS:
		ret = 0;
		break;
	case AVS_STATUS_INVALID:
		ret = -EINVAL;
		break;
	case AVS_STATUS_NO_SUPP:
		ret = -ENOTSUPP;
		break;
	case AVS_STATUS_NO_MAP:
		ret = -ENOENT;
		break;
	case AVS_STATUS_MAP_SET:
		ret = -EEXIST;
		break;

	default:
	case AVS_STATUS_FAILURE:
		ret = -EIO;
		break;
	}

	return ret;
}

static int brcm_send_cmd_via_scmi(unsigned int cmd, unsigned int sub_cmd,
				  unsigned int protocol,
				  unsigned int num_in, unsigned int num_out,
				  u32 *params)
{
	int ret, ret_out;
	struct scmi_xfer *t;
	__le32 *p;
	int i, j = 0;

	if (!bph)
		return -ENODEV;

	if ((num_in || num_out) && !params)
		return -EINVAL;

	ret = bph->xops->xfer_get_init(bph, cmd,
				       sizeof(u32) * (num_in + 2),
				       sizeof(u32) * (num_out + 1), &t);
	if (ret)
		return ret;

	p = (__le32 *)t->tx.buf;
	if (cmd == BRCM_SEND_AVS_CMD) {
		/* First word is meta-info to be used by EL3 */
		p[0] = cpu_to_le32((num_out << 16) | (num_in << 8) | sub_cmd);
		/* Then the full AVS command */
		p[1] = cpu_to_le32(sub_cmd);
		j = 2;
	}

	for (i = 0; i < num_in; i++)
		p[i + j] = cpu_to_le32(params[i]);

	ret = bph->xops->do_xfer(bph, t);

	if (!ret) {
		p = t->rx.buf;
		ret_out = le32_to_cpu(p[0]);
		for (i = 0; i < num_out; i++)
			params[i] = (u32)le32_to_cpu(p[i + 1]);
	}

	bph->xops->xfer_put(bph, t);

	if (cmd == BRCM_SEND_AVS_CMD)
		ret = ret ? ret : avs_ret_to_linux_ret(ret_out);
	else
		ret = ret ? ret : ret_out;

	return ret;
}

static int brcm_send_avs_cmd_via_scmi(unsigned int sub_cmd, unsigned int num_in,
				      unsigned int num_out, u32 *params)
{
	int ret;

	ret = brcm_send_cmd_via_scmi(BRCM_SEND_AVS_CMD, sub_cmd,
				     SCMI_PROTOCOL_BRCM, num_in, num_out,
				     params);

	return ret;
}

static int brcm_send_show_cmd_via_scmi(struct seq_file *s, unsigned int cmd)
{
#define MAX_SHOW_CHARS			256

	char buf[MAX_SHOW_CHARS];
	char *pbuf = buf;
	int state = 0;
	u32 params[SCMI_MAX_STRINGLEN/4 + 1]; /* state + string len */
	char * const str = (char *) &params[0]; /* out */
	ssize_t n_avail, len;

	buf[0] = 0;
	do {
		bool continuation = false;

		params[0] = state; /* in */
		state = brcm_send_cmd_via_scmi(cmd, 0,
					     SCMI_PROTOCOL_BRCM,
					     1, ARRAY_SIZE(params),
					     params);
		if (state < 0)
			break;

		n_avail = MAX_SHOW_CHARS - (pbuf - &buf[0]) - 1;
		len = strnlen(str, sizeof(params) - 1);
		if (n_avail <= 0 || len > n_avail)
			return -EIO;

		/* If line ends in '\', drop the '\' character */
		if (len > 0 && str[len - 1] == '\\') {
			continuation = true;
			str[len - 1] = 0;
			len--;
		}

		strncat(pbuf, str, n_avail);
		pbuf += (len < n_avail) ? len : n_avail;
		n_avail = MAX_SHOW_CHARS - (pbuf - &buf[0]) - 1;

		/* If line ends in '\', hold off on printing it */
		if (continuation && n_avail > 1)
			continue;

		/* Print a line */
		pbuf[0] = 0;
		SEQ_PRINTF(s, "%s\n", buf);
		pbuf = &buf[0];
		buf[0] = 0;
	} while (state > 0);

	return state ? state : 0;
}

static int scmi_brcm_protocol_init(const struct scmi_protocol_handle *bph)
{
	u32 version;

	bph->xops->version_get(bph, &version);

	pr_debug("Brcm SCMI Version %d.%d\n",
		 PROTOCOL_REV_MAJOR(version), PROTOCOL_REV_MINOR(version));

	return 0;
}

static const struct scmi_protocol scmi_brcm_protocol = {
	.id	= SCMI_PROTOCOL_BRCM,
	.owner	= THIS_MODULE,
	.instance_init = scmi_brcm_protocol_init,
};

static int brcmstb_send_avs_cmd(unsigned int cmd, unsigned int in,
			    unsigned int out, u32 args[AVS_MAX_PARAMS])
{
	int ret = -ENODEV;

	if (bph)
		ret = brcm_send_avs_cmd_via_scmi(cmd, in, out, args);
	else if (cpufreq_dev)
		ret = brcmstb_issue_avs_command(cpufreq_dev, cmd,
						in, out, args);
	return ret;
}

#if IS_ENABLED(CONFIG_BRCMSTB_NEXUS_CLK_API)
int brcm_pmap_show(void)
{
	int ret;

	ret = brcm_send_show_cmd_via_scmi(NULL, BRCM_PMAP_SHOW_CMD);

	return ret;
}
EXPORT_SYMBOL(brcm_pmap_show);

/* Taken from perf.c; structure is derived from the SCMI API */
enum scmi_performance_protocol_cmd {
	PERF_DOMAIN_ATTRIBUTES = 0x3,
	PERF_DESCRIBE_LEVELS = 0x4,
};

/* Taken from perf.c; structure is derived from the SCMI API */
struct scmi_perf_set_level {
	__le32 domain;
	__le32 level;
};

/* Taken from perf.c; structure is derived from the SCMI API */
struct scmi_msg_resp_perf_attributes {
	__le16 num_domains;
	__le16 flags;
	__le32 stats_addr_low;
	__le32 stats_addr_high;
	__le32 stats_size;
};

/* Taken from perf.c; structure is derived from the SCMI API */
struct scmi_msg_perf_describe_levels {
	__le32 domain;
	__le32 level_index;
};

/* Taken from perf.c; structure is derived from the SCMI API */
struct scmi_msg_resp_perf_describe_levels {
	__le16 num_returned;
	__le16 num_remaining;
	struct {
		__le32 perf_val;
		__le32 power;
		__le16 transition_latency_us;
		__le16 reserved;
	} opp[];
};

/* Taken from perf.c and modifid for our use */
static int scmi_perf_mb_level_get(u32 domain, u32 *level)
{
	int ret;
	struct scmi_xfer *t;

	ret = bph->xops->xfer_get_init(bph, BRCM_PERF_LEVEL_GET, sizeof(u32),
				       sizeof(u32), &t);
	if (ret)
		return ret;

	t->hdr.poll_completion = false;
	put_unaligned_le32(domain, t->tx.buf);

	bph->xops->do_xfer(bph, t);
	if (!ret)
		*level = get_unaligned_le32(t->rx.buf);

	bph->xops->xfer_put(bph, t);
	return ret;
}

/* Taken from perf.c and modifid for our use */
static int scmi_perf_mb_level_set(u32 domain, u32 level)
{
	int ret;
	struct scmi_xfer *t;
	struct scmi_perf_set_level *lvl;

	ret = bph->xops->xfer_get_init(bph, BRCM_PERF_LEVEL_SET, sizeof(*lvl),
				       0, &t);
	if (ret)
		return ret;

	t->hdr.poll_completion = false;
	lvl = t->tx.buf;
	lvl->domain = cpu_to_le32(domain);
	lvl->level = cpu_to_le32(level);

	ret = bph->xops->do_xfer(bph, t);
	bph->xops->xfer_put(bph, t);

	return ret;
}

/* Taken from perf.c and modifid for our use */
static int
scmi_perf_describe_levels_get(u32 domain)
{
	int ret, cnt;
	u32 pstate_count = 0;
	u16 num_returned, num_remaining;
	struct scmi_xfer *t;
	struct scmi_msg_perf_describe_levels *dom_info;
	struct scmi_msg_resp_perf_describe_levels *level_info;
	struct brcm_core_info *core;

	if (domain >= BCLK_SW_NUM_CORES)
		return -EINVAL;
	core = g_perf_cores + domain;

	ret = bph->xops->xfer_get_init(bph, BRCM_PERF_DESCRIBE_LEVELS,
				       sizeof(*dom_info), 0, &t);
	if (ret)
		return ret;

	dom_info = t->tx.buf;
	level_info = t->rx.buf;

	do {
		dom_info->domain = cpu_to_le32(domain);
		/* Set the number of OPPs to be skipped/already read */
		dom_info->level_index = cpu_to_le32(pstate_count);

		ret = bph->xops->do_xfer(bph, t);
		if (ret)
			break;

		num_returned = le16_to_cpu(level_info->num_returned);
		num_remaining = le16_to_cpu(level_info->num_remaining);
		if (core->num_pstates == 0) {
			size_t size;

			core->num_pstates = num_remaining + num_returned;
			size = sizeof(core->freqs[0]) * core->num_pstates;
			if (core->num_pstates > MAX_PSTATES) {
				dev_err(bph->dev, "No. of pstates exceeded maximum\n");
				break;
			}
			core->freqs = devm_kzalloc(bph->dev, size, GFP_KERNEL);
		} else if (pstate_count + num_remaining > core->num_pstates) {
			dev_err(bph->dev, "No. of pstates exceeded specified\n");
			break;
		}

		for (cnt = 0; cnt < num_returned; cnt++)
			core->freqs[cnt]  = le32_to_cpu(level_info->opp[cnt].perf_val);
		pstate_count += num_returned;

		bph->xops->reset_rx_to_maxsz(bph, t);
		/*
		 * check for both returned and remaining to avoid infinite
		 * loop due to buggy firmware
		 */
	} while (num_returned && num_remaining);

	bph->xops->xfer_put(bph, t);

	return ret;
}


int brcm_pmap_num_pstates(unsigned int core_id, unsigned int *num_pstates)
{
	unsigned int domain = core_id - BCLK_SW_OFFSET;

	if (domain >= BCLK_SW_NUM_CORES)
		return -EINVAL;

	*num_pstates = g_perf_cores[domain].num_pstates;

	return 0;
}
EXPORT_SYMBOL(brcm_pmap_num_pstates);

static int __maybe_unused get_all_pmap_freq_info(int core_id,
						 unsigned int *num_pstates,
						 unsigned int *p_cur_pstate,
						 unsigned int *freqs)
{
	unsigned int domain = core_id - BCLK_SW_OFFSET;
	u32 cur_pstate;
	size_t size;
	int ret;

	if (domain >= BCLK_SW_NUM_CORES)
		return -EINVAL;

	ret = scmi_perf_mb_level_get(domain, &cur_pstate);
	if (ret)
		return -EIO;
	*p_cur_pstate = cur_pstate;
	*num_pstates = g_perf_cores[domain].num_pstates;
	size = sizeof(g_perf_cores[0].freqs[0]) * *num_pstates;
	memcpy(freqs, g_perf_cores[domain].freqs, size);

	return ret;
}

int brcm_pmap_get_pstate(unsigned int core_id, unsigned int *p_cur_pstate)
{
	unsigned int domain = core_id - BCLK_SW_OFFSET;
	u32 cur_pstate;
	int ret;

	if (domain >= BCLK_SW_NUM_CORES)
		return -EINVAL;
	ret = scmi_perf_mb_level_get(domain, &cur_pstate);
	if (ret)
		return -EIO;
	*p_cur_pstate = cur_pstate;

	return 0;
}
EXPORT_SYMBOL(brcm_pmap_get_pstate);

int brcm_pmap_set_pstate(unsigned int core_id, unsigned int pstate)
{
	unsigned int domain = core_id - BCLK_SW_OFFSET;

	if (domain >= BCLK_SW_NUM_CORES ||
	    pstate >= g_perf_cores[domain].num_pstates)
		return -EINVAL;

	return scmi_perf_mb_level_set(domain, pstate);
}
EXPORT_SYMBOL(brcm_pmap_set_pstate);

int brcm_pmap_get_pstate_freqs(unsigned int core_id, u32 *freqs)
{
	unsigned int domain = core_id - BCLK_SW_OFFSET;
	size_t size;

	if (domain >= BCLK_SW_NUM_CORES)
		return -EINVAL;

	size = sizeof(g_perf_cores[0].freqs[0]) *
		g_perf_cores[domain].num_pstates;
	memcpy(freqs, g_perf_cores[domain].freqs, size);

	return 0;
}
EXPORT_SYMBOL(brcm_pmap_get_pstate_freqs);

int brcm_reset_assert(unsigned int reset_id)
{
	int ret;
	u32 params = reset_id - BRST_SW_OFFSET;

	if (params >= BRST_SW_NUM_CORES)
		return -EINVAL;

	ret = brcm_send_cmd_via_scmi(BRCM_RESET_ENABLE_CMD, 0,
				       SCMI_PROTOCOL_BRCM,
				       1, 0,
				       &params);
	return ret;
}
EXPORT_SYMBOL(brcm_reset_assert);

int brcm_reset_deassert(unsigned int reset_id)
{
	int ret;
	u32 params = reset_id - BRST_SW_OFFSET;

	if (params >= BRST_SW_NUM_CORES)
		return -EINVAL;

	ret = brcm_send_cmd_via_scmi(BRCM_RESET_DISABLE_CMD, 0,
				       SCMI_PROTOCOL_BRCM,
				       1, 0,
				       &params);
	return ret;
}
EXPORT_SYMBOL(brcm_reset_deassert);
#endif

int brcm_overtemp_reset(unsigned int reset_temp)
{
	int ret;
	u32 params = reset_temp;

	ret = brcm_send_cmd_via_scmi(BRCM_OVERTEMP_RESET_CMD, 0,
				     SCMI_PROTOCOL_BRCM,
				     1, 0,
				     &params);
	return ret;
}
EXPORT_SYMBOL(brcm_overtemp_reset);

int brcm_overtemp_reset_threshold(unsigned int reset_temp, unsigned int park_high,
				  unsigned int park_low)
{
	int ret;
	u32 params[3];

	params[0] = reset_temp;
	params[1] = park_high;
	params[2] = park_low;
	ret = brcm_send_cmd_via_scmi(BRCM_OVERTEMP_RESET_THRESHOLD_CMD, 0,
				     SCMI_PROTOCOL_BRCM,
				     3, 0,
				     params);
	return ret;
}
EXPORT_SYMBOL(brcm_overtemp_reset_threshold);

static int brcm_scmi_get_clk_ver_str(char *str, unsigned int buf_len)
{
#define MAX_CLK_VER_CHARS 64
	u32 params[(MAX_CLK_VER_CHARS + 3) / 4];
	const int nparams = ARRAY_SIZE(params);
	const size_t max_len = buf_len < sizeof(params) ? buf_len : sizeof(params);
	int ret;

	if (!str)
		return -EINVAL;
	*str = 0;

	ret = brcm_send_cmd_via_scmi(BRCM_GET_CLK_VER_STR, 0,
				     SCMI_PROTOCOL_BRCM, 0, nparams, params);

	if (ret == 0)
		strncpy(str, (const char *)params, max_len - 1);
	str[max_len - 1] = 0;

	return ret;
}

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>

struct trace_log_dfs_info {
	struct dentry *d_tlroot;
	uint32_t active;
	uint32_t flag;
	uint64_t logbuf_paddr;
	uint32_t logbuf_size;
};

static struct dentry *rootdir;
static struct trace_log_dfs_info tldfs;


static int brcm_scmi_stats_show(struct seq_file *s, void *data)
{
	int ret;

	ret = brcm_send_show_cmd_via_scmi(s, BRCM_SCMI_STATS_SHOW_CMD);

	return ret;
}

static int brcm_scmi_stats_reset(void)
{
	int ret;
	u32 param;

	ret = brcm_send_cmd_via_scmi(BRCM_SCMI_STATS_RESET_CMD,
				     0, SCMI_PROTOCOL_BRCM, 0, 0, &param);
	return ret;
}

static int brcm_scmi_clk_summary_show(struct seq_file *s, void *data)
{
	int ret;

	ret = brcm_send_show_cmd_via_scmi(s, BRCM_CLK_SHOW_NEW_CMD);

	return ret;
}

static int brcm_scmi_pmap_show(struct seq_file *s, void *data)
{
	int ret;

	ret = brcm_send_show_cmd_via_scmi(s, BRCM_PMAP_SHOW_CMD);

	return ret;
}

/* Our debugfs helpers */
static int filp_to_core_id(struct file *filp)
{
	const char **p = filp->f_path.dentry->d_inode->i_private;

	return BCLK_SW_OFFSET + (p - &pmap_cores[0]);
}

static int uint_from_buf(const char __user *ubuf, size_t len,
			 unsigned int *result)
{
	char buf[32];

	if (len > sizeof(buf) - 1)
		return -EINVAL;
	if (copy_from_user(buf, ubuf, len))
		return -EFAULT;
	buf[len] = '\0';
	return kstrtouint(buf, 10, result);
}

static int brcm_scmi_stats_summary_open(struct inode *inode, struct file *file)
{
	return single_open(file, brcm_scmi_stats_show, inode->i_private);
}

static ssize_t brcm_scmi_stats_reset_write(struct file *filp,
					   const char __user *ubuf,
					   size_t len, loff_t *offp)
{
	int ret = brcm_scmi_stats_reset();

	return ret ? ret : len;
}

static int brcm_scmi_clk_summary_open(struct inode *inode, struct file *file)
{
	return single_open(file, brcm_scmi_clk_summary_show, inode->i_private);
}

static int brcm_scmi_pmap_open(struct inode *inode, struct file *file)
{
	return single_open(file, brcm_scmi_pmap_show, inode->i_private);
}

static ssize_t brcm_scmi_pmap_enable_wt(struct file *filp,
					const char __user *ubuf,
					size_t len, loff_t *offp)
{
	const int core_id = filp_to_core_id(filp);
	unsigned int en;
	int ret;

	ret = uint_from_buf(ubuf, len, &en);
	if (ret < 0)
		return ret;
	if (en)
		ret = brcm_clk_prepare_enable(core_id);
	else
		brcm_clk_disable_unprepare(core_id);

	return ret < 0 ? ret : len;
}

static ssize_t brcm_scmi_pmap_cur_freq_rd(struct file *filp, char __user *ubuf,
					  size_t count, loff_t *offp)
{
	int ret;
	char buf[32];
	unsigned int freqs[MAX_PSTATES], size, num_pstates, cur_pstate;
	const int core_id = filp_to_core_id(filp);

	ret = get_all_pmap_freq_info(core_id, &num_pstates, &cur_pstate,
				     freqs);
	if (ret < 0)
		return ret;
	size = snprintf(buf, sizeof(buf), "%u\n", freqs[cur_pstate]);
	return simple_read_from_buffer(ubuf, count, offp, buf, size);
}

static ssize_t brcm_scmi_pmap_cur_freq_wt(struct file *filp,
					  const char __user *ubuf,
					  size_t len, loff_t *offp)
{
	int ret, i;
	unsigned int freqs[MAX_PSTATES], num_pstates, cur_pstate, freq;
	const int core_id = filp_to_core_id(filp);

	ret = get_all_pmap_freq_info(core_id, &num_pstates, &cur_pstate,
				     freqs);
	if (ret < 0)
		return ret;
	ret = uint_from_buf(ubuf, len, &freq);
	for (i = 0; i < num_pstates; i++)
		if (freqs[i] == freq)
			break;
	if (i >= num_pstates)
		return -EINVAL;
	ret = brcm_pmap_set_pstate(core_id, i);
	if (ret < 0)
		return ret;
	return len;
}

static ssize_t brcm_scmi_pmap_all_freqs(struct file *filp, char __user *ubuf,
					size_t count, loff_t *offp)
{
	int ret, i;
	char buf[16 * MAX_PSTATES];
	unsigned int freqs[MAX_PSTATES], size, num_pstates;
	const int core_id = filp_to_core_id(filp);

	ret = brcm_pmap_get_pstate_freqs(core_id, freqs);
	if (ret)
		return ret;

	ret = brcm_pmap_num_pstates(core_id, &num_pstates);
	if (ret < 0)
		return ret;

	for (size = 0, i = 0; i < num_pstates; i++) {
		ret = snprintf(buf + size, sizeof(buf) - size, "%u\n",
			       freqs[i]);
		if (ret < 0)
			return ret;
		size += ret;
	}
	return simple_read_from_buffer(ubuf, count, offp, buf, size);
}

static ssize_t brcm_scmi_pmap_cur_pstate_rd(struct file *filp,
					    char __user *ubuf,
					    size_t count, loff_t *offp)
{
	int ret;
	char buf[32];
	unsigned int pstate, size;
	const int core_id = filp_to_core_id(filp);

	ret = brcm_pmap_get_pstate(core_id, &pstate);
	if (ret < 0)
		return ret;
	size = snprintf(buf, sizeof(buf), "%u\n", pstate);
	return simple_read_from_buffer(ubuf, count, offp, buf, size);
}

static ssize_t brcm_scmi_pmap_cur_pstate_wt(struct file *filp,
					    const char __user *ubuf,
					    size_t len, loff_t *offp)
{
	int ret;
	unsigned int pstate;
	const int core_id = filp_to_core_id(filp);

	ret = uint_from_buf(ubuf, len, &pstate);
	if (ret < 0)
		return ret;
	ret = brcm_pmap_set_pstate(core_id, pstate);
	if (ret < 0)
		return ret;
	return len;
}

static ssize_t brcm_scmi_pmap_num_pstates(struct file *filp, char __user *ubuf,
					  size_t count, loff_t *offp)
{
	int ret;
	char buf[32];
	unsigned int num_pstates, size;
	const int core_id = filp_to_core_id(filp);

	ret = brcm_pmap_num_pstates(core_id, &num_pstates);
	if (ret < 0)
		return ret;
	size = snprintf(buf, sizeof(buf), "%u\n", num_pstates);
	return simple_read_from_buffer(ubuf, count, offp, buf, size);
}

static int brcm_send_trace_log_cmd_via_scmi(unsigned int cmd)
{
	int ret;
	u32 params[5];
	int i = 0;

	params[i++] = tldfs.active;
	params[i++] = tldfs.flag;
	params[i++] = lower_32_bits(tldfs.logbuf_paddr);
	params[i++] = upper_32_bits(tldfs.logbuf_paddr);
	params[i++] = tldfs.logbuf_size;

	ret = brcm_send_cmd_via_scmi(cmd, 0,
				     SCMI_PROTOCOL_BRCM,
				     ARRAY_SIZE(params),
				     0,
				     params);
	return ret;
}

static ssize_t brcm_trace_log_on_rd(struct file *filp,
				    char __user *ubuf,
				    size_t count, loff_t *offp)
{
	char buf[4];
	unsigned int size;
	int ret;

	size = snprintf(buf, sizeof(buf), "%u\n", tldfs.active);
	ret = simple_read_from_buffer(ubuf, count, offp, buf, size);

	return ret;
}

static ssize_t brcm_trace_log_on_wt(struct file *filp,
				    const char __user *ubuf,
				    size_t len, loff_t *offp)
{
	int sts = len, ret;
	unsigned int on;

	ret = uint_from_buf(ubuf, len, &on);
	if (ret < 0) {
		sts = ret;
		goto trace_out;
	}

	if (tldfs.active == on)
		goto trace_out;

	tldfs.active = on;
	ret = simple_write_to_buffer(&on, 4, offp, ubuf, len);
	if (ret < 0) {
		sts = ret;
		goto trace_out;
	}

	ret = brcm_send_trace_log_cmd_via_scmi(BRCM_TRACE_LOG_ON_CMD);
	if (ret < 0)
		sts = ret;
trace_out:
	return sts;
}

static const struct file_operations brcm_scmi_stats_reset_fops = {
	.write		= brcm_scmi_stats_reset_write,
};

static const struct file_operations brcm_scmi_stats_summary_fops = {
	.open		= brcm_scmi_stats_summary_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations brcm_scmi_clk_summary_fops = {
	.open		= brcm_scmi_clk_summary_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations brcm_scmi_pmap_fops = {
	.open		= brcm_scmi_pmap_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations brcm_scmi_pmap_enable_fops = {
	.write		= brcm_scmi_pmap_enable_wt,
};

static const struct file_operations brcm_scmi_pmap_cur_freq_fops = {
	.read		= brcm_scmi_pmap_cur_freq_rd,
	.write		= brcm_scmi_pmap_cur_freq_wt,
};

static const struct file_operations brcm_scmi_pmap_all_freqs_fops = {
	.read		= brcm_scmi_pmap_all_freqs,
};

static const struct file_operations brcm_scmi_pmap_cur_pstate_fops = {
	.read		= brcm_scmi_pmap_cur_pstate_rd,
	.write		= brcm_scmi_pmap_cur_pstate_wt,
};

static const struct file_operations brcm_trace_log_on_fops = {
	.read		= brcm_trace_log_on_rd,
	.write		= brcm_trace_log_on_wt,
};

static const struct file_operations brcm_scmi_pmap_num_pstates_fops = {
	.read		= brcm_scmi_pmap_num_pstates,
};

/**
 * brcm_trace_log_debug_init - lazily populate the debugfs brcm_trace_log
 * directory
 *
 * This function populates the debugfs brcm_trace directory once at boot-time
 * when we know that debugfs is setup. It should only be called once at
 * boot-time.
 */
static int brcm_trace_log_debug_init(void)
{
	struct dentry *d;

	memset(&tldfs, 0, sizeof(struct trace_log_dfs_info));

	tldfs.d_tlroot = debugfs_create_dir("brcm-trace", NULL);

	if (!tldfs.d_tlroot)
		return -ENOMEM;

	d = debugfs_create_file("tracing_on", 0644, tldfs.d_tlroot,
				&tldfs.active, &brcm_trace_log_on_fops);

	if (!d)
		return -ENOMEM;

	debugfs_create_x32("flag", 0644, tldfs.d_tlroot, &tldfs.flag);
	debugfs_create_x64("phys_addr64", 0644, tldfs.d_tlroot,
			   &tldfs.logbuf_paddr);
	debugfs_create_u32("buf_size", 0644, tldfs.d_tlroot,
			   &tldfs.logbuf_size);

	return 0;
}

/**
 * brcm_scmi_debug_init - lazily populate the debugfs brcm_scmi directory
 *
 * clks are often initialized very early during boot before memory can be
 * dynamically allocated and well before debugfs is setup. This function
 * populates the debugfs brcm_scmi directory once at boot-time when we
 * know that debugfs is setup. It should only be called once at boot-time.
 */
static int brcm_scmi_debug_init(void)
{
	struct dentry *d, *d_cores;
	unsigned int i, n;

	rootdir = debugfs_create_dir("brcm-scmi", NULL);

	if (!rootdir)
		return -ENOMEM;

	d = debugfs_create_file("clk_summary", 0444, rootdir, NULL,
				&brcm_scmi_clk_summary_fops);

	if (!d)
		return -ENOMEM;

	d = debugfs_create_file("pmap", 0444, rootdir, NULL,
				&brcm_scmi_pmap_fops);

	if (!d)
		return -ENOMEM;

	d = debugfs_create_file("stats_summary", 0644, rootdir, NULL,
				&brcm_scmi_stats_summary_fops);

	if (!d)
		return -ENOMEM;

	d = debugfs_create_file("stats_reset", 0644, rootdir, NULL,
				&brcm_scmi_stats_reset_fops);

	if (!d)
		return -ENOMEM;

	d_cores = debugfs_create_dir("pmap_cores", rootdir);
	if (!d_cores)
		return -ENOMEM;

	for (i = 0; i < BCLK_SW_NUM_CORES; i++) {
		struct dentry *d_core;

		if (brcm_pmap_num_pstates(i + BCLK_SW_OFFSET, &n))
			continue;
		if (n == 0 || !pmap_cores[i])
			continue;
		d_core = debugfs_create_dir(pmap_cores[i], d_cores);
		if (!d_core)
			return -ENOMEM;

		d = debugfs_create_file("enable", 0644, d_core, &pmap_cores[i],
					&brcm_scmi_pmap_enable_fops);
		if (!d)
			return -ENOMEM;

		d = debugfs_create_file("cur_freq", 0644, d_core,
					&pmap_cores[i],
					&brcm_scmi_pmap_cur_freq_fops);
		if (!d)
			return -ENOMEM;

		d = debugfs_create_file("all_freqs", 0444, d_core,
					&pmap_cores[i],
					&brcm_scmi_pmap_all_freqs_fops);
		if (!d)
			return -ENOMEM;

		d = debugfs_create_file("cur_pstate", 0644, d_core,
					&pmap_cores[i],
					&brcm_scmi_pmap_cur_pstate_fops);
		if (!d)
			return -ENOMEM;

		d = debugfs_create_file("num_pstates", 0444, d_core,
					&pmap_cores[i],
					&brcm_scmi_pmap_num_pstates_fops);
		if (!d)
			return -ENOMEM;
	}



	return 0;
}

#endif

/**
 * brcmstb_stb_dvfs_get_pstate() - Get the pstate for a core/island.
 *
 * @idx: index; 0 == cpu/combined, 1 == reserved, 2 == HVD core, ...) (in).
 * @pstate: the current pstate (out).
 * @info: four values, each taking a byte: [31:24] reserved, [23:16] num
 *     cores, [15:8] num pstates, [7:0] idx given (out).
 *
 * Return: 0 on success
 */
int brcmstb_stb_dvfs_get_pstate(unsigned int idx, unsigned int *pstate,
				u32 *info)
{
	u32 args[AVS_MAX_PARAMS];
	int ret = -ENODEV;

	args[0] = idx;

	ret = brcmstb_send_avs_cmd(AVS_CMD_GET_PSTATE, 1, 2, args);
	if (!ret) {
		*pstate = args[0];
		*info = args[1];
	}
	return ret;
}
EXPORT_SYMBOL(brcmstb_stb_dvfs_get_pstate);

/**
 * brcmstb_stb_dvfs_set_pstate() -- Set the pstate for a core/island.
 *
 * @idx: index; 0 == cpu/combined, 1 == reserved, 2 == HVD core, ... (in).
 * @pstate: desired pstate (in).
 * @clk_writes -- the number of clocks regs to write [0..3] (in).
 * @clk_params: array of (3*num_clk_writes) u32s; every set of
 *     three u32s is { addr, data, mask } of a clock register write (in).
 *
 *  Return: 0 on success.
 */
int brcmstb_stb_dvfs_set_pstate(unsigned int idx, unsigned int pstate,
				unsigned int clk_writes,
				const u32 *clk_params)
{
	u32 args[AVS_MAX_PARAMS];
	unsigned int i, j, num_in;
	int ret = -ENODEV;

	args[0] = (pstate & 0xff) | ((idx & 0xff) << 8)
		| ((clk_writes & 0xff) << 16);
	for (i = 0, num_in = 1; i < clk_writes; i++)
		for (j = 0; j < 3; j++, num_in++)
			args[3 * i + 1 + j] = clk_params[3 * i + j];
	if (bph) {
		ret = brcm_send_avs_cmd_via_scmi(AVS_CMD_SET_PSTATE,
						 num_in, 0, args);
	} else if (cpufreq_dev) {
		if (idx || clk_writes)
			ret = -EINVAL;
		else
			ret = brcmstb_issue_avs_command(cpufreq_dev,
							AVS_CMD_SET_PSTATE,
							num_in, 0, args);
	}
	return ret;
}
EXPORT_SYMBOL(brcmstb_stb_dvfs_set_pstate);

/**
 * brcmstb_stb_avs_read_debug() -- get debug value via EL3/AVS.
 *
 * @debug_idx: see AVS API documentation (in).
 * @value: value of the indicated debug_idx (out).
 *
 * Return: 0 on success.
 */
int brcmstb_stb_avs_read_debug(unsigned int debug_idx, u32 *value)
{
	u32 args[AVS_MAX_PARAMS];
	int ret = -ENODEV;

	args[0] = debug_idx;

	ret = brcmstb_send_avs_cmd(AVS_CMD_READ_DEBUG, 1, 2, args);
	if (ret)
		return ret;

	if (!ret)
		*value = args[1];

	return ret;
}
EXPORT_SYMBOL(brcmstb_stb_avs_read_debug);

/**
 * brcmstb_stb_avs_get_pmic_info -- get PMIC information via EL3/AVS.
 *
 * @info: PMIC information (out).
 *
 * Return: 0 on success.
 */
int brcmstb_stb_avs_get_pmic_info(struct brcmstb_avs_pmic_info *info)
{
	u32 args[AVS_MAX_PARAMS];
	unsigned int i, byte_offset;
	int ret;

	/* Fetch PMIC information page #0 */
	ret = brcmstb_send_avs_cmd(AVS_CMD_GET_PMIC_INFO, 0, 4, args);
	if (ret)
		return ret;

	/* Now fill in the structure */
	info->num_pmic_devices = args[0];
	info->num_regulators = args[0] >> 8;
	info->num_gpios = args[0] >> 16;
	for (i = 0; i < ARRAY_SIZE(info->ext_infos); i++) {
		byte_offset = 8 * i;
		info->ext_infos[i].i2c_addr = args[1] >> byte_offset;
		info->ext_infos[i].chip_id = args[2] >> byte_offset;
		info->ext_infos[i].caps = args[3] >> byte_offset;
	}

	/* Request the second page of PMIC information */
	args[0] = 1;
	ret = brcmstb_send_avs_cmd(AVS_CMD_GET_PMIC_INFO, 1, 4, args);
	if (ret == 0) {
		for (i = 0; i < info->num_pmic_devices; i++) {
			info->ext_infos2[i].version = args[i] & 0xff;
			info->ext_infos2[i].variant = (args[i] >> 8) & 0x0f;
			info->ext_infos2[i].gid = (args[i] >> 16) & 0x0f;
			info->ext_infos2[i].esd = (args[i] >> 24) & 0x0f;
		}
	}

	return 0;
}
EXPORT_SYMBOL(brcmstb_stb_avs_get_pmic_info);

/**
 * brcmstb_stb_avs_set_pmic_config -- set PMIC configuration via EL3/AVS.
 *
 * @pmic: PMIC index (in)
 * @ovr_temp: over-temperature threshold (in)
 * @standby_regulators: regulators selected for S3_STANDBY
 *
 * Return: 0 on success.
 */
int brcmstb_stb_avs_set_pmic_config(u8 pmic,
				    u32 ovr_temp,
				    u32 standby_regulators)
{
	u32 num_in = 3, num_out = 1;
	u32 args[AVS_MAX_PARAMS];
	int ret;

	args[0] = pmic;
	args[1] = ovr_temp;
	args[2] = standby_regulators;

	ret = brcmstb_send_avs_cmd(AVS_CMD_SET_PMIC_CONFIG, num_in, num_out,
				   args);
	if (ret)
		return ret;

	if ((args[0] & 0xff) != pmic) {
		pr_err("Invalid PMIC return value: %d vs %d\n",
		       pmic, args[0]);
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL(brcmstb_stb_avs_set_pmic_config);

/**
 * brcmstb_stb_avs_get_pmic_status -- get PMIC status via EL3/AVS.
 *
 * @pmic: PMIC index (in)
 * @die_temp: PMIC die temperature (out)
 * @ext_therm_temp: External thermistor temperature (out)
 * @overall_power: Overall power consumption (out)
 *
 * Return: 0 on success.
 */
int brcmstb_stb_avs_get_pmic_status(u8 pmic,
				    u32 *die_temp,
				    u32 *ext_therm_temp,
				    u32 *overall_power)
{
	u32 num_in = 1, num_out = 4;
	u32 args[AVS_MAX_PARAMS];
	int ret;

	args[0] = pmic;

	ret = brcmstb_send_avs_cmd(AVS_CMD_GET_PMIC_STATUS, num_in, num_out,
				   args);
	if (ret)
		return ret;

	if ((args[0] & 0xff) != pmic) {
		pr_err("Invalid PMIC return value: %d vs %d\n",
		       pmic, args[0]);
		return -EINVAL;
	}

	*die_temp = args[1];
	*ext_therm_temp = args[2];
	*overall_power = args[3];

	return ret;
}
EXPORT_SYMBOL(brcmstb_stb_avs_get_pmic_status);

/**
 * brcmstb_stb_avs_get_pmic_reg_info -- get PMIC regulator configuration via
 * EL3/AVS.
 *
 * @pmic: PMIC index (in)
 * @die_temp: PMIC die temperature (out)
 * @ext_therm_temp: External thermistor temperature (out)
 * @overall_power: Overall power consumption (out)
 *
 * Return: 0 on success.
 */
int brcmstb_avs_get_pmic_reg_info(u8 regulator, u16 *nom_volt)
{
	u32 num_in = 1, num_out = 2;
	u32 args[AVS_MAX_PARAMS];
	int ret;

	args[0] = regulator;

	ret = brcmstb_send_avs_cmd(AVS_CMD_GET_PMIC_REG_INFO, num_in,
			       num_out, args);
	if (ret)
		return ret;

	if ((args[0] & 0xff) != regulator) {
		pr_err("Invalid regulator return value: %d vs %d\n",
		       regulator, args[0]);
		ret = -EINVAL;
	}

	*nom_volt = args[1];

	return ret;
}
EXPORT_SYMBOL(brcmstb_avs_get_pmic_reg_info);

/**
 * brcmstb_stb_avs_set_pmic_reg_config -- set PMIC regulator configuration via
 * EL3/AVS.
 *
 * @pmic: PMIC index (in)
 * @die_temp: PMIC die temperature (out)
 * @ext_therm_temp: External thermistor temperature (out)
 * @overall_power: Overall power consumption (out)
 *
 * Return: 0 on success.
 */
int brcmstb_avs_set_pmic_reg_config(u8 regulator, u16 voltage,
				    u16 over_current_thres)
{
	u32 num_in = 2, num_out = 1;
	u32 args[AVS_MAX_PARAMS];
	int ret;

	args[0] = regulator;
	args[1] = voltage;
	args[1] |= (u32)over_current_thres << 16;

	ret = brcmstb_send_avs_cmd(AVS_CMD_SET_PMIC_REG_CONFIG, num_in, num_out,
				   args);
	if (ret)
		return ret;

	if ((args[0] & 0xff) != regulator) {
		pr_err("Invalid regulator return value: %d vs %d\n",
		       regulator, args[0]);
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL(brcmstb_avs_set_pmic_reg_config);

/**
 * brcmstb_stb_avs_get_pmic_reg_status -- get PMIC regulator status via
 * EL3/AVS.
 *
 * @pmic: PMIC index (in)
 * @die_temp: PMIC die temperature (out)
 * @ext_therm_temp: External thermistor temperature (out)
 * @overall_power: Overall power consumption (out)
 *
 * Return: 0 on success.
 */
int brcmstb_avs_get_pmic_reg_status(u8 regulator, u16 *voltage,
				    u16 *curr)
{
	u32 num_in = 1, num_out = 2;
	u32 args[AVS_MAX_PARAMS];
	int ret;

	args[0] = regulator;

	ret = brcmstb_send_avs_cmd(AVS_CMD_GET_PMIC_REG_STATUS, num_in, num_out,
			       args);
	if (ret)
		return ret;

	if ((args[0] & 0xff) != regulator) {
		pr_err("Invalid regulator return value: %d vs %d\n",
		       regulator, args[0]);
		ret = -EINVAL;
	}

	*voltage = args[1];
	*curr = args[1] >> 16;

	return ret;
}
EXPORT_SYMBOL(brcmstb_avs_get_pmic_reg_status);

static const char *get_s_hash(const char *str, int *len)
{
	const char *p = strstr(str, ",S=");

	*len = 0;
	if (p) {
		p += 3;
		*len = strspn(p, "abcdef0123456789");
	}
	return p;
}


static void clk_blob_ver_check(struct scmi_device *sdev)
{
	static const char fmt0[] = "ClkVerTest: %4s: %s\n";
	static const char fmt1[] = "%16s: %s\n";
	struct device_node *dn = sdev->dev.of_node;
	char ams_clk_ver_str[64];
	const char *bolt_clk_ver_str = NULL;
	ams_clk_ver_str[0] = 0;

	if (dn
	    && of_property_read_string(dn, "brcm,clk-ver", &bolt_clk_ver_str) == 0
	    && brcm_scmi_get_clk_ver_str(ams_clk_ver_str, sizeof(ams_clk_ver_str)) == 0
	    && ams_clk_ver_str[0]) {

		if (strncmp(bolt_clk_ver_str, ams_clk_ver_str, sizeof(ams_clk_ver_str))) {
			/* Clk ver string is different, let us look at S-string */
			int n0, n1;
			const char *p0 = get_s_hash(bolt_clk_ver_str, &n0);
			const char *p1 = get_s_hash(ams_clk_ver_str,  &n1);
			bool s_hash_equal = n0 && n0 == n1 && !strncmp(p0, p1, n0);
			bool no_s_hash_compare = !n0 || !n1;

			if (s_hash_equal) {
				dev_err(&sdev->dev, fmt0, "WARN",
					"Clock version string mismatch, but ...");
				dev_err(&sdev->dev, fmt1, "",
					"S-hash strs equal, so probably OK");
			} else if (no_s_hash_compare) {
				dev_err(&sdev->dev, fmt0, "FAIL",
					"Clock version mismatch!");
			} else {
				dev_err(&sdev->dev, fmt0, "FAIL",
					"Clock version mismatch, this is serious!");
			}
			dev_err(&sdev->dev, "%16s: BOLT    %s\n", "", bolt_clk_ver_str);
			dev_err(&sdev->dev, "%16s: AMS/VAF %s\n", "", ams_clk_ver_str);
		} else {
			dev_info(&sdev->dev, fmt0, "PASS", ams_clk_ver_str);
		}
	} else {
		dev_info(&sdev->dev, fmt0, "FAIL", "by default; cannot test");
	}
}


static int brcm_scmi_dvfs_probe(struct scmi_device *sdev)
{
	const struct scmi_handle *sh = sdev->handle;
	const void *ops;
	int i, value = 0;

	ops = sh->devm_protocol_get(sdev, SCMI_PROTOCOL_BRCM, &bph);
	if (!bph)
		return -EPROBE_DEFER;

	clk_blob_ver_check(sdev);

	/* This tells AVS we are using the new API */
	(void)brcmstb_stb_avs_read_debug(0, &value);

	brcm_clk_init(&sdev->dev);

	/* Grab perf info */
	for (i = 0; i < BCLK_SW_NUM_CORES; i++) {
		int ret = scmi_perf_describe_levels_get(i);

		if (ret && ret != -ENOENT)
			dev_err(&sdev->dev, "error getting %s perf info\n", pmap_cores[i]);
	}

#ifdef CONFIG_DEBUG_FS
	brcm_scmi_debug_init();
	brcm_trace_log_debug_init();
#endif
	return 0;
}

static void brcm_scmi_dvfs_remove(struct scmi_device *sdev)
{
}

static const struct scmi_device_id brcm_scmi_id_table[] = {
	{ SCMI_PROTOCOL_BRCM, "brcmstb-scmi" },
	{ },
};
MODULE_DEVICE_TABLE(scmi, brcm_scmi_id_table);


static const struct scmi_device_id brcm_scmi_perf_id_table[] = {
	{ SCMI_PROTOCOL_PERF, "brcmstb-dvfs" },
	{ },
};
MODULE_DEVICE_TABLE(brcm_scmi_perf, brcm_scmi_perf_id_table);

static struct scmi_driver brcmstb_scmi_dvfs_drv = {
	.name		= "brcmstb-scmi-dvfs",
	.probe		= brcm_scmi_dvfs_probe,
	.remove		= brcm_scmi_dvfs_remove,
	.id_table	= brcm_scmi_id_table,
};

static int __init brcmstb_scmi_driver_init(void)
{
	int ret;

	ret = scmi_protocol_register(&scmi_brcm_protocol);
	if (ret)
		return ret;

	ret = scmi_driver_register(&brcmstb_scmi_dvfs_drv, THIS_MODULE,
				   KBUILD_MODNAME);
	if (ret)
		scmi_protocol_unregister(&scmi_brcm_protocol);

	return ret;
}
module_init(brcmstb_scmi_driver_init);

static void __exit brcmstb_scmi_driver_exit(void)
{
	scmi_driver_unregister(&brcmstb_scmi_dvfs_drv);
	scmi_protocol_unregister(&scmi_brcm_protocol);
}
module_exit(brcmstb_scmi_driver_exit);

MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL v2");
