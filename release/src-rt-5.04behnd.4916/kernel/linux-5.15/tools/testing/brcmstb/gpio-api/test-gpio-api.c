#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sizes.h>
#include <linux/brcmstb/brcmstb.h>
#include <linux/brcmstb/gpio_api.h>
#include <linux/of.h>
#include <linux/of_address.h>

#define GPIO_DT_COMPAT	"brcm,brcmstb-gpio"
#define GIO_BANK_SIZE	0x20
#define GPIO_PER_BANK	32

static int test_gpio_bank(uint32_t offset, unsigned int width)
{
	unsigned int shift, bank, banks;
	int ret;

	banks = width / GIO_BANK_SIZE;

	pr_info("Testing controller at 0x%08x, width: 0x%08x banks: %d\n",
		offset, width, banks);

	for (bank = 0; bank < banks; bank++) {
		for (shift = 0; shift < 32; shift++) {
			ret = brcmstb_gpio_irq(offset + bank * GIO_BANK_SIZE,
					       shift);
			if (ret < 0)
				continue;

			pr_info("%s: GPIO%d -> IRQ%d\n",
				__func__, shift + bank * GPIO_PER_BANK, ret);
		}
	}

	return 0;
}

static int __init test_init(void)
{
	struct device_node *dn;
	unsigned int times;
	struct resource res;
	int ret;

	for_each_compatible_node(dn, NULL, GPIO_DT_COMPAT) {
		ret = of_address_to_resource(dn, 0, &res);
		if (ret)
			continue;

		if (res.flags != IORESOURCE_MEM)
			continue;

		for (times = 0; times < 2; times++) {
			ret = test_gpio_bank(res.start, times);
			if (ret)
				pr_err("%s: test for shift %d failed!\n",
				       __func__, times);
		}
	}

	return 0;
}

static void __exit test_exit(void)
{
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Florian Fainelli (Broadcom Corporation)");
