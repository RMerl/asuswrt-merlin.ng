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
/****************************************************************************
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/

#include <linux/gfp.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include "pmc_drv.h"
#include "pmc_rpc.h"
#include "avs_svc.h"
#include "bcm_ubus4.h"
#include <linux/delay.h>
#include <linux/irqflags.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>

#ifdef CONFIG_PM
extern struct kobject *power_kobj; 
#else
struct kobject *power_kobj = NULL;
#endif

struct bpcm_attribute {
        struct attribute attr;
        ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf);
        ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count);
        int param;
};

static uint8_t avs_svc_msg_get_retcode(rpc_msg *msg)
{
    return (uint8_t)msg->data[2];
}

static ssize_t convert_result(char *buf, int sel, int value)
{
	static const char *name[] = {
        [kDIE_TEMP] = "DieTemp",
        [kCORE_VIN] = "VIN Core",
        [kCPU_VIN]  = "VIN CPU",
        [kCPU_FREQ] = "CPU Freq",
        [k_VIN_1p8] = "1p8V",
        [k_VIN_3p3] = "3p3V",
        [k_VIN_1p2] = "1p2V",
	};
	static const char *suffix[] = {
        [kDIE_TEMP] = "°C",
        [kCORE_VIN] = "V",
        [kCPU_VIN]  = "V",
        [kCPU_FREQ] = "GHz",
        [kCPU_FREQ_MHz] = "MHz",
        [k_VIN_1p8] = "V",
        [k_VIN_3p3] = "V",
        [k_VIN_1p2] = "V",
    };

	int i, f, len = 0;

	while (*buf) {
		len++;
		buf++;
	}

	if (sel < kDIE_TEMP || sel > k_VIN_1p2)
		return len;

    i = value/1000;
    f = (value > 0 ? value : -value) % 1000; 

    if ((i == 0) && (sel == kCPU_FREQ))
    {
        return len + sprintf(buf, "%s: %03d %s\n", name[sel], f, suffix[sel+1]);
    }
    return len + sprintf(buf, "%s: %d.%03d %s\n", name[sel], i, f, suffix[sel]);
}

static ssize_t get_sensor(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    struct bpcm_attribute *prm = (struct bpcm_attribute *) kattr;
    int ret = 0;
    uint32_t val;
    uint32_t sensor;
    uint32_t mask = 0xffff;

    switch (prm->param)
    {
    case kDIE_TEMP:
        sensor = 0;
        mask = 0xffffffff;
        break;
    case kCORE_VIN:
        sensor = 2;
        break;
    case kCPU_VIN:
        sensor = 1;
        break;
    case k_VIN_1p8:
        sensor = 3;
        break;
    case k_VIN_3p3:
        sensor = 4;
        break;
    case k_VIN_1p2:
        sensor = 5;
        break;
    default:
        sensor = prm->param;
    }

    ret = bcm68xx_get_sensor(sensor, &val, NULL);
    if (ret)
        return sprintf(buf, "ERROR: avs_svc: failure (%d)\n", ret);

    return convert_result(buf, prm->param, (val & mask));
}

static ssize_t get_vin(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    uint32_t val;
    uint32_t mask = 0xffff;
    int ret = 0;

    ret = bcm68xx_get_sensor(1, &val, NULL);
    if (ret)
        return sprintf(buf, "ERROR: avs_svc: failure (%d)\n", ret);

    convert_result(buf, kCPU_VIN, (val & mask));

    ret = bcm68xx_get_sensor(2, &val, NULL);
    if (ret)
        return sprintf(buf, "ERROR: avs_svc: failure (%d)\n", ret);

    return convert_result(buf, kCORE_VIN, (val & mask));
}

static ssize_t avs_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    struct bpcm_attribute *prm = (struct bpcm_attribute *) kattr;
    rpc_msg msg;
    int ret = 0;

    rpc_msg_init(&msg, RPC_SERVICE_AVS, AVS_SVC_GET_DATA ,RPC_SERVICE_VER_AVS_GET_DATA, 0, 0, 0);
    msg.data[0] = prm->param;

    ret = pmc_svc_request(&msg, avs_svc_msg_get_retcode);
    if (ret)
        return sprintf(buf, "ERROR: avs_svc: failure (%d)\n", ret);

    return convert_result(buf, prm->param, msg.data[1]);
}

static ssize_t avs_info(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    struct bpcm_attribute *prm = (struct bpcm_attribute *) kattr;
    int i;
    ssize_t len = 0;
    int val, h, f;
    int ret = 0;

    for (i = 1; i <= prm->param; i++)
    {
        rpc_msg msg;
        rpc_msg_init(&msg, RPC_SERVICE_AVS, AVS_SVC_GET_DATA ,RPC_SERVICE_VER_AVS_GET_DATA, kCPU_FREQ, 0, 0);
        msg.data[0] += i;

        ret = pmc_svc_request(&msg, avs_svc_msg_get_retcode);
        if (ret)
        {
            return sprintf(buf, "ERROR: avs_svc: failure (%d)\n", ret);
        }

        val = msg.data[1];

        h = val/1000;
        f = (val > 0 ? val : -val) % 1000;

        len += sprintf(&buf[len], "%d.%03d ", h, f);
    }
    len += sprintf(&buf[len], "\n");

    return len;
}

static char* avs_mode2str(uint32_t mode)
{
    switch (mode) 
    {
    case AVS_MODE:
        return "AVS";
    case DFS_MODE:
        return "DFS";
    case DVS_MODE:
        return "DVS";
    case DVFS_MODE:
        return "DVFS";
    default:
        return "Unknown";
    }
}

static ssize_t dvfs_mode_get(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    int ret = 0;
    size_t len = 0;
    uint32_t mode, pmap, cur_state;

    ret = bcm68xx_get_pmap(&mode, &pmap, &cur_state, NULL, NULL);
    if (ret)
    {
        len = sprintf(buf, "ERROR: get_pmap failed (%d)\n", ret);
    }
    else
    {
        len = sprintf(buf, "Running mode %s PMAP %d State %d\n",
            avs_mode2str(mode), pmap, cur_state); 
    }
    
    return len;
}

static ssize_t dvfs_mode_set(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int ret = 0;
    ssize_t res = count;
    uint32_t mode;
    char s_mode[16];

    sscanf(buf, "%s", s_mode);

    if (strcmp(s_mode, "avs") == 0)
        mode = AVS_MODE;
    else if (strcmp(s_mode, "dfs") == 0)
        mode = DFS_MODE;
    else if (strcmp(s_mode, "dvfs") == 0)
        mode = DVFS_MODE;
    else 
    {
        printk("The %s mode is not supported. (avs|dfs|dvfs)\n", s_mode);
        res = -EINVAL;
        goto exit;
    }

    ret = bcm68xx_set_pmap(mode, 0, NULL);
    if (ret)
    {
        printk("ERROR: set_pmap: failed (%d)\n", ret);
        res = ret;
    }

exit:
    return res;
}

static ssize_t dvfs_pstate_get(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    int ret = 0;
    size_t len = 0;
    uint32_t cur, total;

    ret = bcm68xx_get_pstate(&cur, &total, NULL);
    if (ret)
    {
        len = sprintf(buf, "ERROR: get_pstate: failed (%d)\n", ret);
    }
    else
    {
        len = sprintf(buf, "Current P_STATE: %d (0 ... %d)\n", cur, total - 1 ); 
    }
    
    return len;
}

static ssize_t dvfs_pstate_set(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    unsigned value;
    int ret = 0;
    char s_value[16];
    ssize_t res = count;

    sscanf(buf, "%s", s_value);
    if (kstrtouint(s_value, 0, &value)) {
        printk("%s: not a number\n", buf);
        res = -EINVAL;
        goto exit;
    }

    ret = bcm68xx_set_pstate(value, NULL);
    if (ret)
    {
        printk("ERROR: set_pstate: failed (%d)\n", ret);
        res = ret;
    }

exit:
    return res;
}

static ssize_t dvfs_pstates_get(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    int ret = 0;
    size_t len = 0;
    uint32_t cur, total;
    int i;

    ret = bcm68xx_get_pstate(&cur, &total, NULL);
    if (ret)
    {
        len = sprintf(buf, "ERROR: get_pstate: failed (%d)\n", ret);
        goto exit;
    }
    for (i = 0; i < total; i++)
    {
        uint32_t freq;

        freq = 0;
        ret = bcm68xx_get_cpu_freq(i, &freq, NULL);
        if (ret)
        {
            len = sprintf(buf, "ERROR: get_cpu_freq for %d state: failed (%d)\n", i, ret);
            goto exit;
        }

        len += sprintf(buf + len, " %c %d: %d MHz\n", i == cur ? '*' : ' ', i, freq); 
    }
    
exit:
    return len;
}

static const struct bpcm_attribute root_attrs[] = {
        { { "DieTemp",     0444, }, get_sensor,      NULL, kDIE_TEMP },
        { { "VIN",         0444, }, get_vin,         NULL, 0 },
        { { "VIN_core",    0444, }, get_sensor,      NULL, kCORE_VIN },
        { { "VIN_cpu",     0444, }, get_sensor,      NULL, kCPU_VIN },
        { { "CPU_freq",    0444, }, avs_show,        NULL, kCPU_FREQ },
        { { "V_1p8",       0444, }, get_sensor,      NULL, k_VIN_1p8 },
        { { "V_3p3",       0444, }, get_sensor,      NULL, k_VIN_3p3 },
        { { "V_1p2",       0444, }, get_sensor,      NULL, k_VIN_1p2 },
        { { "info",        0444, }, avs_info,        NULL, 4 },
};

static const struct bpcm_attribute dvfs_attrs[] = {
        { { "mode",     0644, }, dvfs_mode_get,      dvfs_mode_set, 0 },
        { { "P_State",     0644, }, dvfs_pstate_get,      dvfs_pstate_set, 0 },
        { { "state_table", 0444, }, dvfs_pstates_get,      NULL, 0 },
};

static int __init pmc_sysfs_init(void)
{
    struct kobject *bpcmkobj;
    struct kobject *dvfskobj;
    struct bpcm_attribute *pmcattr;
    unsigned d;
    int error;

    /* create power object ourselves? */
    if (!power_kobj) {
        power_kobj = kobject_create_and_add("power", NULL);
        if (!power_kobj)
            return -ENOENT;
    }

    /* create bpcm subdirectory */
    bpcmkobj = kobject_create_and_add("bpcm", power_kobj);
    if (bpcmkobj == 0)
        return -ENOMEM;

    dvfskobj = kobject_create_and_add("dvfs", power_kobj);
    if (dvfskobj == 0)
        return -ENOMEM;


    /* create root nodes */
    d = ARRAY_SIZE(root_attrs);
    pmcattr = kmalloc(d * sizeof *pmcattr, GFP_KERNEL);
    while (d--) {
        pmcattr[d] = root_attrs[d];
        sysfs_attr_init(&pmcattr[d].attr);
        error = sysfs_create_file(bpcmkobj, (struct attribute *)&pmcattr[d]);
    }

    d = ARRAY_SIZE(dvfs_attrs);
    pmcattr = kmalloc(d * sizeof *pmcattr, GFP_KERNEL);
    while (d--) {
        pmcattr[d] = dvfs_attrs[d];
        sysfs_attr_init(&pmcattr[d].attr);
        error = sysfs_create_file(dvfskobj, (struct attribute *)&pmcattr[d]);
    }

    return 0;
}

late_initcall(pmc_sysfs_init);
