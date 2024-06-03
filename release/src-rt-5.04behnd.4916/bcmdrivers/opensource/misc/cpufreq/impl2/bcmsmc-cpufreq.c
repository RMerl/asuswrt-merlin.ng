/*
   <:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

   :>
   */

/* CPU Frequency scaling support for BCMSMC devices */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/version.h>
#include <avs_svc.h>

struct bcmsmc_cpufreq_t { 
    unsigned int op_mode;
    uint32_t init_freq;
    uint32_t cur_pstate;
    uint32_t num_of_pstates;
    struct platform_device *pdev;
} *g_bcmsmc_cpufreq = NULL;

static char *mode2str(unsigned int mode)
{
    switch (mode) {
    case AVS_MODE:
        return "AVS";
    case DFS_MODE:
        return "DFS";
    case DVS_MODE:
        return "DVS";
    case DVFS_MODE:
        return "DVFS";
    }
    return NULL;
}

static int bcmsmc_find_freq_in_table(struct cpufreq_policy *policy, unsigned long rate)
{
    int i;
    struct bcmsmc_cpufreq_t *ctx = (struct bcmsmc_cpufreq_t *)policy->driver_data;

    if (rate == 0)
        return 0;

    for (i = 0; i < ctx->num_of_pstates; i++)
    {
        if (rate == policy->freq_table[i].frequency)
            return policy->freq_table[i].driver_data; 
    }
    return -EINVAL;
}

static struct cpufreq_frequency_table* bcmsmc_build_freq_table(struct bcmsmc_cpufreq_t *ctx)
{
    int ret;
    uint32_t total_states = 1;
    uint32_t freq;
    int i;
    struct device *dev = &ctx->pdev->dev;
    struct cpufreq_frequency_table *freq_table;

    if (ctx->op_mode != AVS_MODE)
    {
        ret = bcm68xx_get_pmap(NULL, NULL, NULL, &total_states, NULL);
        if (ret)
        {
            dev_err(dev, "%s: failed to get num of supported pstates\n", __func__);
            return ERR_PTR(ret);
        }

        if (total_states == 0)
        {
            dev_err(dev, "%s: Pstates switch not supported\n", __func__);
            return ERR_PTR(-ENOTSUPP);
        }
    }

    ctx->num_of_pstates = total_states;

    freq_table = devm_kcalloc(dev, total_states + 1, sizeof(*freq_table), GFP_KERNEL);
    if (!freq_table)
    {
        dev_err(dev,"%s: could not allocate clock table\n", __func__);
        return ERR_PTR(-ENOMEM);
    }

    for(i = 0; i < total_states; i++)
    {
        ret = bcm68xx_get_cpu_freq(i, &freq, NULL);
        if (ret)
        {
            dev_err(dev,"%s: failed to get cpu frequency for %d state\n", __func__, i);
            return ERR_PTR(ret);
        }
        freq_table[i].frequency = freq;
        freq_table[i].driver_data = i;
    }
    freq_table[i].frequency = CPUFREQ_TABLE_END; 
    return freq_table;
}

static int bcm_avs_target_index(struct cpufreq_policy *policy, unsigned int index)
{
    struct bcmsmc_cpufreq_t *ctx = policy->driver_data;
    int ret = bcm68xx_set_pstate(policy->freq_table[index].driver_data, NULL);
    if (!ret)
        ctx->cur_pstate = policy->freq_table[index].driver_data;
    return ret;
}

static unsigned int bcm_avs_cpufreq_get(unsigned int cpu)
{
    struct cpufreq_policy *policy = cpufreq_cpu_get(cpu);
    struct bcmsmc_cpufreq_t *ctx = policy->driver_data;
    int ret;
    uint32_t freq = 0;

    ret = bcm68xx_get_cpu_freq(ctx->cur_pstate, &freq, NULL);
    if (ret) 
        return ret;
    else
        return freq;
}

static int bcm_avs_cpufreq_init(struct cpufreq_policy *policy)
{
    struct platform_device *pdev;
    struct bcmsmc_cpufreq_t *ctx;
    struct device *dev;
    struct cpufreq_frequency_table *freq_table;
    int32_t init_idx;
    int ret;
    unsigned int transition_latency = 500000;

    pdev = cpufreq_get_driver_data();
    ctx = platform_get_drvdata(pdev);
    policy->driver_data = ctx;
    dev = &pdev->dev;

    freq_table = bcmsmc_build_freq_table(ctx);
    if (IS_ERR(freq_table)) 
        return PTR_ERR(freq_table);

    policy->freq_table = freq_table;
    policy->transition_delay_us = transition_latency;
    policy->cpuinfo.transition_latency = transition_latency;

    /* All cores share the same clock and thus the same policy. */
    cpumask_setall(policy->cpus);

    init_idx = bcmsmc_find_freq_in_table(policy, ctx->init_freq);
    if (init_idx < 0)
    {
        dev_warn(dev, "The initial frequency %d is not supported, choosing the max\n", ctx->init_freq);
        init_idx = 0;
    }

    ret = bcm68xx_set_pmap(ctx->op_mode, init_idx, NULL);
    if (!ret)
    {
        policy->cur = policy->freq_table[init_idx].frequency; 
        ctx->cur_pstate = policy->freq_table[init_idx].driver_data;
        dev_info(dev, "registered at %s mode\n", mode2str(ctx->op_mode));
        return 0;
    }
    dev_err(dev, "couldn't initialize driver (%d)\n", ret);

    return ret;
}

static ssize_t show_bcm_avs_pstate(struct cpufreq_policy *policy, char *buf)
{
    uint32_t pstate;

    if (bcm68xx_get_pstate(&pstate, NULL, NULL))
        return sprintf(buf, "<unknown>\n");

    return sprintf(buf, "%u\n", pstate);
}

static ssize_t show_bcm_avs_mode(struct cpufreq_policy *policy, char *buf)
{
    uint32_t pmap;
    uint32_t mode;

    if (bcm68xx_get_pmap(&mode, &pmap, NULL, NULL, NULL))
        return sprintf(buf, "<unknown>\n");

    return sprintf(buf, "%s %u\n", mode2str(mode), pmap);
}

static ssize_t show_bcm_avs_frequency(struct cpufreq_policy *policy, char *buf)
{
    struct bcmsmc_cpufreq_t *ctx = policy->driver_data;
    uint32_t frequency;

    if (bcm68xx_get_cpu_freq(ctx->cur_pstate, &frequency, NULL))
        return sprintf(buf, "<unknown>\n");

    return sprintf(buf, "%uMHz\n",frequency);
}

cpufreq_freq_attr_ro(bcm_avs_pstate);
cpufreq_freq_attr_ro(bcm_avs_mode);
cpufreq_freq_attr_ro(bcm_avs_frequency);

static struct freq_attr *bcm_avs_cpufreq_attr[] = {
    &cpufreq_freq_attr_scaling_available_freqs,
    &bcm_avs_pstate,
    &bcm_avs_mode,
    &bcm_avs_frequency,
    NULL
};

static struct cpufreq_driver bcm_avs_driver = {
    .flags		= CPUFREQ_NEED_INITIAL_FREQ_CHECK,
    .verify		= cpufreq_generic_frequency_table_verify,
    .target_index	= bcm_avs_target_index,
    .get		= bcm_avs_cpufreq_get,
    .init		= bcm_avs_cpufreq_init,
    .attr		= bcm_avs_cpufreq_attr,
    .name		= "bcmsmc-dvfs",
};

static const struct of_device_id bcmsmc_cpufreq_of_match[] = {
    { .compatible = "brcm,bcmsmc-cpufreq", .data = NULL, },
    {},
};

MODULE_DEVICE_TABLE(of, bcmsmc_cpufreq_of_match);

static int bcmsmc_cpufreq_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const struct of_device_id *match;
    const char *mode = "avs";
    uint32_t init_freq = 0;
    int ret;

    match = of_match_device(bcmsmc_cpufreq_of_match, dev);
    if (!match)
    {
        dev_err(dev, "Failed to find cpufreq driver\n");
        return -ENODEV;
    }

    ret = bcm68xx_get_pmap(NULL, NULL, NULL, NULL, NULL);
    if (ret)
    {
        dev_err(dev, "AVS disabled - Frequency scalling is not possible\n");
        return -ENODEV;
    }

    g_bcmsmc_cpufreq = devm_kzalloc(dev, sizeof(*g_bcmsmc_cpufreq), GFP_KERNEL);
    if (!g_bcmsmc_cpufreq)
    {
        ret = -ENOMEM;
        goto error;
    }

    of_property_read_string(dev->of_node, "op-mode", &mode);
    if (strcmp(mode, "avs") == 0)
        g_bcmsmc_cpufreq->op_mode = AVS_MODE;
    else if (strcmp(mode, "dfs") == 0)
        g_bcmsmc_cpufreq->op_mode = DFS_MODE;
    else if (strcmp(mode, "dvfs") == 0)
        g_bcmsmc_cpufreq->op_mode = DVFS_MODE;
    else
    {
        dev_err(dev, "%s not supported operational mode\n", mode);
        ret = -ENOTSUPP;
        goto error;
    }
    if (g_bcmsmc_cpufreq->op_mode != AVS_MODE)
        of_property_read_u32(dev->of_node, "clock-frequency", &init_freq);

    g_bcmsmc_cpufreq->init_freq = init_freq;
    g_bcmsmc_cpufreq->pdev = pdev;
    platform_set_drvdata(pdev, g_bcmsmc_cpufreq);

    bcm_avs_driver.driver_data = pdev;
    ret = cpufreq_register_driver(&bcm_avs_driver);
    if (ret)
        goto error;

    return 0;

error:

    if (g_bcmsmc_cpufreq)
    {
        devm_kfree(dev, g_bcmsmc_cpufreq);
        g_bcmsmc_cpufreq = NULL;
    }
    return ret;
}

static struct platform_driver bcmsmc_cpufreq_driver = {
    .probe = bcmsmc_cpufreq_probe,
    .driver = {
        .name = "bcmsmc-cpufreq",
        .of_match_table = bcmsmc_cpufreq_of_match,
    },
};

static int __init bcmsmc_cpufreq_drv_reg(void)
{
    return platform_driver_register(&bcmsmc_cpufreq_driver);
}

late_initcall(bcmsmc_cpufreq_drv_reg);

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BVG CPUFreq driver for PON platforms");
MODULE_LICENSE("GPL v2");
