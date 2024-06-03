/*
 * <:copyright-BRCM:2018:DUAL/GPL:standard
 * 
 *    Copyright (c) 2018 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

/* BCM UBUS3 supporting routines */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/bug.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

typedef struct
{
	uint32_t ubus_cfg;
	uint32_t ubus_cfg_window[8];
} ubus_t;

static ubus_t __iomem *ubus = NULL;

int ubus_decode_pcie_wnd_cfg(unsigned int base, unsigned int size, unsigned int core)
{
#if defined(CONFIG_BCM94908)
	uint32_t core2pid[3] = {0x00, 0x07, 0x0d};
	uint32_t reg_data0, reg_data1;
	int c = -1;
	static int cfg_index = 0;

	if (!ubus) {
		printk("%s ubus io resource is not set up yet\r\n", __FUNCTION__);
		return -1;
	}

	if (cfg_index >= 8) {
		/* Max only 8 registers */
		printk("%s(0x%x, 0x%x, 0x%x) exceeded max registers\r\n",
			__FUNCTION__, base, size, core);
		return -1;
	}

	if (!size) {
		/* size can not be zero */
		printk("%s(0x%x, 0x%x, 0x%x) invalid size\r\n",
			__FUNCTION__, base, size, core);
		return -1;
	}

	/* Get the size bits (assumed size is always in power of 2) */
	while (size) {
		c++;
		size >>= 1;
	}

	/* Is this extension of existing memory region or using a new region ? */

	reg_data0 = (base >> 8); //base addr[31:8] @ 23:00
	reg_data0 |= (((base >> 8) & 0xFF) << 24); //remap addr[15:8] @ 31:24
	reg_data1 = (base >> 16); //remap addr[31:16] @ 47:32
	reg_data1 |= (core2pid[core] << 16); //pid[7:0] @ 55:48
	reg_data1 |= (c << 24); //size[4:0] @ 60:56
	reg_data1 |= (0x01 << 30); //enable[1:0] @63:62

	printk("[%d] ubus_cfg_window[0] = 0x%x, ubus_cfg_window[1] = 0x%x\r\n",
		core, reg_data0, reg_data1);

	ubus->ubus_cfg_window[cfg_index++] = reg_data0;
	ubus->ubus_cfg_window[cfg_index++] = reg_data1;

	return 0;
#else  /* ! CONFIG_BCM94908 */
	return -1;
#endif /* ! CONFIG_BCM94908 */
}
EXPORT_SYMBOL(ubus_decode_pcie_wnd_cfg);

static int bcm_ubus_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *res;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ubus_systop");
	if (!res) {
		dev_err(dev, "Failed to find ubus_systop resource\n");
		return -EINVAL;
	}

	ubus = devm_ioremap_resource(dev, res);
	if (IS_ERR(ubus)) {
		dev_err(dev, "Failed to map the ubus_dcm_clk resource\n");
		return -ENXIO;
	}

	dev_info(dev, "Broadcom BCA Legacy UBUS Driver\n");
	dev_info(dev, "resource %pr\n", res);
	dev_info(dev, "map to virt addr 0x%px\n", ubus);
	return 0;
}


static const struct of_device_id bcm_ubus_of_match[] = {
	{ .compatible = "brcm,bca-ubus3"},
	{},
};
MODULE_DEVICE_TABLE(of, bcm_ubus_of_match);


static struct platform_driver bcm_ubus_driver = {
	.probe = bcm_ubus_probe,
	.driver = {
		.name = "bcm-ubus3",
		.of_match_table = bcm_ubus_of_match,
	},
};

static int __init bcm_ubus_module_init(void)
{
	return platform_driver_register(&bcm_ubus_driver);
}

static void __exit bcm_ubus_module_exit(void)
{
	platform_driver_unregister(&bcm_ubus_driver);
	return;
}

module_init(bcm_ubus_module_init);
module_exit(bcm_ubus_module_exit);

MODULE_DESCRIPTION("Broadcom BCA Legacy UBUS Driver");
MODULE_LICENSE("GPL v2");
