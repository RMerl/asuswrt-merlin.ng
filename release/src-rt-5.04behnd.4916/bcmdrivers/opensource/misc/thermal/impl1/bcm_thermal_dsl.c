#include <linux/printk.h>
#include "pmc_core_api.h"
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/cpu.h>
#include <linux/unistd.h>
#include "shared_utils.h"

#include <linux/bcm_version_compat.h>

static unsigned int __iomem
#if defined(CONFIG_BCM94908) && !defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER) && !defined(CONFIG_CPU_FREQ)
    *bcr_cluster_clk_ctrl0,
    *bcr_cluster_clk_pattern0,
#endif
    *bcr_bac_cpu_therm_temp;

    struct bcm_biucfg_reg_addr
{
    const char *name;
    struct resource *res;
    unsigned int __iomem **paddr;
};

#define REG_ADDR(reg)   { .name = #reg, .paddr = &bcr_##reg, }
static struct bcm_biucfg_reg_addr bra[] = {
#if defined(CONFIG_BCM94908) && !defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER) && !defined(CONFIG_CPU_FREQ)
    REG_ADDR(cluster_clk_ctrl0),
    REG_ADDR(cluster_clk_pattern0),
#endif
    REG_ADDR(bac_cpu_therm_temp),
};

static void bcm_biucfg_unmap_reg_addr(void)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(bra) && *(bra[i].paddr); i++) {
        iounmap(*(bra[i].paddr));
        pr_info("iounmapped reg %s <0x%llx 0x%llx> from %px\n",
            bra[i].name, (unsigned long long)bra[i].res->start,
            (unsigned long long)resource_size(bra[i].res),
            *(bra[i].paddr));
        *(bra[i].paddr) = NULL;
    }
}

static int bcm_biucfg_map_reg_addr(struct platform_device *pdev)
{
    int i, ret;

    pr_info("pdev=%px name=%s num_resources=%d\n",
        pdev, pdev->name, pdev->num_resources);

    for (i = 0; i < pdev->num_resources; i++) {
        struct resource *r = &pdev->resource[i];

        pr_info("i=%d name=%s start=%llx end=%llx flags=0x%lx type=%lu\n",
            i, r->name, (unsigned long long)r->start,
            (unsigned long long)r->end, r->flags, resource_type(r));
    }

    for (i = 0; i < ARRAY_SIZE(bra); i++) {
        bra[i].res = platform_get_resource_byname(pdev,
            IORESOURCE_MEM, bra[i].name);
        if (!bra[i].res) {
            pr_err("Error: failed to get reg %s\n", bra[i].name);
            ret = -ENOENT;
            goto Error;
        }

        *(bra[i].paddr) = ioremap(bra[i].res->start,
            resource_size(bra[i].res));
        if (!*(bra[i].paddr)) {
            pr_err("Error: failed to ioremap reg %s\n", bra[i].name);
            ret = -ENXIO;
            goto Error;
        }

        pr_info("ioremapped reg %s <0x%llx 0x%llx> to %px\n",
            bra[i].name, (unsigned long long)bra[i].res->start,
            (unsigned long long)resource_size(bra[i].res),
            *(bra[i].paddr));
    }

    return 0;

Error:
    bcm_biucfg_unmap_reg_addr();
    return ret;
}

void bcm_cpufreq_set_freq_max(unsigned maxdiv);

#if !defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER) && !defined(CONFIG_CPU_FREQ)
void bcm_cpufreq_set_freq_max(unsigned maxdiv)
{
#ifdef CONFIG_BCM94908
    *bcr_cluster_clk_pattern0 = (maxdiv == 1 ? 0xffffffff : 0x55555555);
    *bcr_cluster_clk_ctrl0 = 1 << 4;	// enable user clock-patterns
#endif
}
#endif

static struct cpumask brcm_cpu_absent_mask;

// mark cpu as not-present to prevent other code from onlining;
// (duplicates [unexported] set_cpu_present for module);
// also update sysfs cpu offline status
static void brcm_cpu_absent(unsigned int cpu)
{
    cpumask_clear_cpu(cpu, (struct cpumask *)cpu_present_mask);
    get_cpu_device(cpu)->offline = true;
    cpumask_set_cpu(cpu, &brcm_cpu_absent_mask);
}

// mark cpu as present to allow other code from onlining;
// (duplicates [unexported] set_cpu_present for module);
// also update sysfs cpu offline status
static void brcm_cpu_present(unsigned int cpu)
{
    cpumask_set_cpu(cpu, (struct cpumask *)cpu_present_mask);
    get_cpu_device(cpu)->offline = false;
    cpumask_clear_cpu(cpu, &brcm_cpu_absent_mask);
}

static int broadcom_cpu_set_state(unsigned long state)
{
    int cpuIndex, rc;

    switch (state)
    {
    case 0:
#if defined (CONFIG_BCM947622) || defined (CONFIG_BCM963178) || defined (CONFIG_BCM96756)
        for (cpuIndex = 0; cpuIndex < num_possible_cpus() - 1; cpuIndex++) {
#else
        for (cpuIndex = 1; cpuIndex < num_possible_cpus(); cpuIndex++) {
#endif
            if (!cpu_online(cpuIndex))
            {
                printk("take CPU#%d online\n", cpuIndex);
                brcm_cpu_present(cpuIndex); // mark present before cpu_up
                if ((rc = add_cpu(cpuIndex)) != 0)
                    brcm_cpu_absent(cpuIndex);
            }
        }
        break;
    case 1:
#if defined (CONFIG_BCM947622) || defined (CONFIG_BCM963178) || defined (CONFIG_BCM96756)
        // Take the second to last possible CPU offline
        cpuIndex = num_possible_cpus() - 2;
#else
        // Take the last possible CPU offline
        cpuIndex = num_possible_cpus() - 1;
#endif
        if (cpuIndex < 1)
            break;

        if (cpu_online(cpuIndex)) {
            printk("take CPU#%d offline\n", cpuIndex);
#if defined(CONFIG_HOTPLUG_CPU)
            if ((rc = remove_cpu(cpuIndex)) == 0)
                brcm_cpu_absent(cpuIndex);
#endif
        }
        break;
    case 2:
#if defined (CONFIG_BCM947622) || defined (CONFIG_BCM963178) || defined (CONFIG_BCM96756)
        // Keep the last possible CPU online
        for (cpuIndex = 0; cpuIndex < num_possible_cpus() - 1; cpuIndex++) {
#else
        for (cpuIndex = 1; cpuIndex < num_possible_cpus(); cpuIndex++) {
#endif
            if (cpu_online(cpuIndex)) {
                printk("take CPU#%d offline\n", cpuIndex);
#if defined(CONFIG_HOTPLUG_CPU)
                if ((rc = remove_cpu(cpuIndex)) == 0)
                    brcm_cpu_absent(cpuIndex);
#endif
            }
        }
        break;
    }

    return 0;
}

int bcm_thermal_get_temperature(int *temp)
{
#if defined (CONFIG_BCM963158) || defined (CONFIG_BCM947622) || defined (CONFIG_BCM963178) || \
    defined (CONFIG_BCM96756) || defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
    int ret, adc = -1;

    ret = GetPVTKH2(kTEMPERATURE, 0, &adc);
    if (ret) {
        printk("Failed to get RAIL 0 temperature, ret=%d\n", ret);
        return ret;
    }
    *temp = pmc_convert_pvtmon(kTEMPERATURE, adc);

    ret = *bcr_bac_cpu_therm_temp;
    if (!(ret & (1<<31))) {
        printk("Failed to get CPU temperature, ret=%d\n", ret);
        return -1;
    }
    ret = pmc_convert_pvtmon(kTEMPERATURE, ret & 0x3ff);
    if (ret > (int)*temp)
        *temp = ret;
#else
    int regVal = *bcr_bac_cpu_therm_temp;
    regVal &= 0x000003ff; 
    *temp = (4133500 - regVal * 4906) / 10; // for 4908 only
#endif
    return 0;  
}

int thermal_driver_chip_probe(struct platform_device *plat_dev)
{
    if (bcm_biucfg_map_reg_addr(plat_dev))
        return -1;

    return 0;
}

int thermal_driver_chip_remove(struct platform_device *plat_dev)
{
    int cpu;

    for_each_cpu(cpu, &brcm_cpu_absent_mask)
    {
        if (cpu_present(cpu))
            continue;

        cpumask_set_cpu(cpu, (struct cpumask *)cpu_present_mask);
        get_cpu_device(cpu)->offline = false;
        add_cpu(cpu);
    }

    bcm_biucfg_unmap_reg_addr();

    return 0;
}

int bcm_thermal_state_notify(int trip, int enable)
{
    switch (trip)
    {
    case 0: // Cold compensation
        if (enable)
        {
            printk("Cold compensation: go normal\n");
            RecloseAVS(0);
        }
        else
        {
            printk("Cold compensation: handling Cold\n");
            RecloseAVS(1);
        }
        break;
    case 1: // CPU cores
        if (enable)
        {
            printk("CPU: disable cores state 1\n");
            broadcom_cpu_set_state(1);
        }
        else
        {
            printk("CPU: enable all cores\n");
            broadcom_cpu_set_state(0);
        }
        break;
    case 2: // CPU cores
        if (enable)
        {
            printk("CPU: disable cores state 2\n");
            broadcom_cpu_set_state(2);
        }
        else
        {
            printk("CPU: disable cores state 1\n");
            broadcom_cpu_set_state(1);
        }
        break;
    case 3: // CPU frequency
        if (enable)
        {
            printk("CPU: Go to low frequency\n");
            bcm_cpufreq_set_freq_max(2);
        }
        else
        {
            printk("CPU: Go to max frequency\n");
            bcm_cpufreq_set_freq_max(1);
        }
        break;
    default:
        break;
    }

    return 0;
}
