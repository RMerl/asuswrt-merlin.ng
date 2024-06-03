/*
<:copyright-BRCM:2021:GPL/GPL:standard

   Copyright (c) 2021 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
/***************************************************************************
 * Description: This file contains kernel level support for the 
 *              battery management unit
 ***************************************************************************/

#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include "bmu.h"
#include "pmc/pmc_ssb_access.h"
#include "pmc/pmc_apm.h"

typedef struct apm_control_registers
{
	uint32_t reserved0;
	uint32_t irq_pend;
#define DEV_BMU_IRQ                 0x00000400
	uint32_t irq_mask;
#ifdef CONFIG_BCM963148
	uint32_t reserved1[72];
	uint32_t apm_analog_bg;
#define APM_ANALOG_BG_BOOST         (1<<16)
	uint32_t codec_config_4;
#define APM_LDO_VREGCNTL_7          (1<<(7+8))
#endif
} apm_control_registers;

static volatile apm_control_registers *bmu_apmctrl;

typedef struct cb_bmu_list
{
	struct list_head list;
	char name[DEVNAMSIZ];
	cb_bmu_t cb_bmu_fn;
	void *context;
} cb_bmu_list;

static cb_bmu_list *g_cb_bmu_list_head;

static int bmu_enabled;
static DEFINE_MUTEX(bmu_mutex);

static struct tasklet_struct bcm_bmu_tasklet;
static unsigned long bmu_tasklet_active_cnt;
static spinlock_t bmu_irq_lock;
static unsigned int bcm_bmu_irq_id;

int bcm_bmu_is_battery_enabled(void)
{
	return bmu_enabled;
}
EXPORT_SYMBOL(bcm_bmu_is_battery_enabled);

void bcm_bmu_register_handler(char *devname, void *cbfn, void *context)
{
	cb_bmu_list *new_node;

	// do all the stuff that can be done without the lock first
	if (devname == NULL || cbfn == NULL) {
		pr_err("%s: Error -- invalid devname or cbfn (%p, %p)\n",
			__func__, devname, cbfn);
		return;
	}

	if (strlen(devname) > (DEVNAMSIZ - 1))
		pr_warn("%s: Warning -- devname too long, will be truncated\n",
			__func__);

	new_node= (cb_bmu_list *)kmalloc(sizeof(cb_bmu_list), GFP_KERNEL);
	memset(new_node, 0x00, sizeof(cb_bmu_list));
	INIT_LIST_HEAD(&new_node->list);
	strncpy(new_node->name, devname, DEVNAMSIZ-1);
	new_node->cb_bmu_fn = (cb_bmu_t)cbfn;
	new_node->context = context;

	// OK, now acquire the lock and insert into list
	mutex_lock(&bmu_mutex);
	if (g_cb_bmu_list_head == NULL) {
		pr_err("%s: Error -- list head is null\n", __func__);
		kfree(new_node);
	} else {
		list_add(&new_node->list, &g_cb_bmu_list_head->list);
		pr_info("%s: %s registered\n", __func__, devname);
	}
	mutex_unlock(&bmu_mutex);
}
EXPORT_SYMBOL(bcm_bmu_register_handler);

void bcm_bmu_deregister_handler(char *devname)
{
	struct list_head *pos;
	cb_bmu_list *tmp;
	int found=0;

	if (devname == NULL) {
		pr_err("%s: Error -- devname is null\n", __func__);
		return;
	}

	pr_info("%s: %s is deregistering\n", __func__, devname);

	mutex_lock(&bmu_mutex);
	if (g_cb_bmu_list_head == NULL) {
		pr_err("%s: Error -- list head is null\n", __func__);
	} else {
		list_for_each(pos, &g_cb_bmu_list_head->list) {
			tmp = list_entry(pos, cb_bmu_list, list);
			if (!strcmp(tmp->name, devname)) {
				list_del(pos);
				kfree(tmp);
				found = 1;
				pr_info("%s: %s is deregistered\n", __func__, devname);
				break;
			}
		}
		
		if (!found)
			pr_info("%s: %s not (de)registered\n", __func__, devname);
	}
	mutex_unlock(&bmu_mutex);
}
EXPORT_SYMBOL(bcm_bmu_deregister_handler);

/***************************************************************************
* Battery Management Unit ISR and functions.
* Triggers when switching from Utility power to Battery mode.
* Note: BMUD application reports when switching from Battery back to Utility.
***************************************************************************/
static void bcm_bmu_tasklet_fn(unsigned long data)
{
	struct list_head *pos;
	cb_bmu_list *tmp = NULL;
	unsigned long flags;

	/* Invoke each registered function */
	list_for_each(pos, &g_cb_bmu_list_head->list) {
		tmp = list_entry(pos, cb_bmu_list, list);
		(tmp->cb_bmu_fn)(tmp->context);
	}

	spin_lock_irqsave(&bmu_irq_lock, flags);
	bmu_tasklet_active_cnt = 0;
	spin_unlock_irqrestore(&bmu_irq_lock, flags);
}

static irqreturn_t bcm_bmu_isr(int irq, void * dev_id)
{
	unsigned long flags;

	if (bmu_apmctrl->irq_pend & DEV_BMU_IRQ) {
		/* Clear the BMU specific interrupt */
		bmu_apmctrl->irq_pend |= DEV_BMU_IRQ;

		spin_lock_irqsave(&bmu_irq_lock, flags);
		bmu_tasklet_active_cnt++;
		spin_unlock_irqrestore(&bmu_irq_lock, flags);

		if (bmu_tasklet_active_cnt == 1)
			tasklet_hi_schedule(&bcm_bmu_tasklet);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

static struct of_device_id const bcm_bmu_of_match[] = {
	{ .compatible = "brcm,bmu-1-x" },
	{ /* end of list */ }
};

MODULE_DEVICE_TABLE(of, bcm_bmu_of_match);

static int bcm_bmu_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct resource *res_reg, *res_irq;
	cb_bmu_list *new_node;
	int ret;
	struct device *dev = &pdev->dev;

	match = of_match_device(bcm_bmu_of_match, dev);
	if (!match) {
		dev_err(dev, "failed to match the battery management unit\n");
		return -ENODEV;
	}

	res_reg = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res_reg) {
		dev_err(dev, "failed to get reg resource\n");
		return -ENODEV;
	}

	res_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res_irq) {
		dev_err(dev, "failed to get irq resource\n");
		return -ENODEV;
	}

	dev_info(dev, "bmu matched %s reg=<0x%zx 0x%zx> irq_id=%zu",
		match->compatible, res_reg->start, resource_size(res_reg),
		res_irq->start);

	bmu_apmctrl = ioremap(res_reg->start, resource_size(res_reg));
	if (!bmu_apmctrl) {
		dev_err(dev, "ioremap reg failed\n");
		return -ENXIO;
	}

	printk("%s: enabling battery management unit\n", __func__);

	/* set vr drive strength for bmu board */
	write_ssbm_reg(0x00+7, 0x12d9, 1);
	write_ssbm_reg(0x40+7, 0x12d9, 1);
	write_ssbm_reg(0x20+7, 0x12d9, 1);

	pmc_apm_power_up();
#ifdef CONFIG_BCM963148 
	// APM_ANALOG_BG_BOOST and APM_LDO_VREGCNTL_7 default to 0 in 63148
	// and need to be set
	bmu_apmctrl->apm_analog_bg |= APM_ANALOG_BG_BOOST;
	bmu_apmctrl->codec_config_4 |= APM_LDO_VREGCNTL_7;
#endif

	if (g_cb_bmu_list_head) {
		printk("%s: cb_bmu_list head is not null\n", __func__);
		return -EINVAL;
	}
	new_node = (cb_bmu_list *)kmalloc(sizeof(cb_bmu_list), GFP_KERNEL);
	memset(new_node, 0x00, sizeof(cb_bmu_list));
	INIT_LIST_HEAD(&new_node->list);
	g_cb_bmu_list_head = new_node;

	spin_lock_init(&bmu_irq_lock);
	tasklet_init(&bcm_bmu_tasklet, bcm_bmu_tasklet_fn, 0);

	ret = request_irq(res_irq->start, bcm_bmu_isr, 0, dev_name(dev), pdev);
	if (ret) {
		dev_err(dev, "request_irq failed\n");
		return ret;
	}
	bcm_bmu_irq_id = res_irq->start;

	/* Enable the BMU specific interrupt */
	bmu_apmctrl->irq_mask |= DEV_BMU_IRQ;
	bmu_enabled = 1;
	return 0;
}

static int bcm_bmu_remove(struct platform_device *pdev)
{
	struct list_head *pos;
	cb_bmu_list *tmp;

	if (!bmu_apmctrl)
		return 0;

	bmu_enabled = 0;
	bmu_apmctrl->irq_mask &= ~DEV_BMU_IRQ;
	free_irq(bcm_bmu_irq_id, pdev);
	tasklet_kill(&bcm_bmu_tasklet);
	iounmap(bmu_apmctrl);
	bmu_apmctrl = NULL;

	if (!g_cb_bmu_list_head)
		return 0;

	list_for_each(pos, &g_cb_bmu_list_head->list) {
		tmp = list_entry(pos, cb_bmu_list, list);
		list_del(pos);
		kfree(tmp);
	}

	kfree(g_cb_bmu_list_head);
	g_cb_bmu_list_head = NULL;
	return 0;
}

static struct platform_driver bcm_bmu_platform_driver = {
	.driver = {
		.name = "bcm_bmu",
		.of_match_table = bcm_bmu_of_match,
	},
	.probe = bcm_bmu_probe,
	.remove = bcm_bmu_remove,
};

static int __init bcm_bmu_init(void)
{
	return platform_driver_register(&bcm_bmu_platform_driver);
}

static void __exit bcm_bmu_exit(void)
{
	platform_driver_unregister(&bcm_bmu_platform_driver);
}

module_init(bcm_bmu_init);
module_exit(bcm_bmu_exit);

MODULE_DESCRIPTION("Broadcom battery management unit driver");
MODULE_LICENSE("GPL");
