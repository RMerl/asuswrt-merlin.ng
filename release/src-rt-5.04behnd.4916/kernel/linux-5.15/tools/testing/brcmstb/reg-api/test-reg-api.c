#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sizes.h>
#include <linux/gpio.h>
#include <linux/brcmstb/brcmstb.h>
#include <linux/brcmstb/gpio_api.h>
#include <linux/of.h>
#include <linux/of_address.h>

#define GPIO_DT_COMPAT	"brcm,brcmstb-gpio"
#define GIO_BANK_SIZE	0x20

static int test_gpio_bank(uint32_t offset, unsigned int width)
{
	unsigned int bank, banks;
	int ret;

	banks = width / GIO_BANK_SIZE;

	pr_info("Testing controller at 0x%08x, width: 0x%08x banks: %d\n",
		offset, width, banks);

	for (bank = 0; bank < banks; bank++) {
		ret = brcmstb_update32(offset + bank * GIO_BANK_SIZE, 0xf, 1);
		if (ret)
			pr_err("%s: failed to request bank %d\n", __func__, bank);

		ret = brcmstb_update32(offset + bank * GIO_BANK_SIZE, 0xf << 8, 1);
		if (ret)
			pr_err("%s: failed to request bank %d\n", __func__, bank);

		ret = brcmstb_update32(offset + bank * GIO_BANK_SIZE, 0xf << 16, 1);
		if (ret)
			pr_err("%s: failed to request bank %d\n", __func__, bank);

		ret = brcmstb_update32(offset + bank * GIO_BANK_SIZE, 0xf << 24, 1);
		if (ret)
			pr_err("%s: failed to request bank %d\n", __func__, bank);
	}

	return 0;
}

static unsigned int request_linux_gpios(unsigned int width, unsigned int base)
{
	unsigned int bank, banks;
	unsigned int num;

	banks = width / GIO_BANK_SIZE;

	for (bank = 0; bank < banks; bank++) {
		num = base + bank * 32;
	 	gpio_request(num, "Linux test");
		gpio_request(num + 8, "Linux test");
		gpio_request(num + 16, "Linux test");
		pr_info("%s: bank %d request GPIOs %d, %d, %d\n",
			__func__, bank, num, num + 8, num + 16);
	}

	return banks * 32;
}

static int brcmstb_aon_gpio(struct device_node *dn)
{
	return of_property_read_bool(dn, "always-on");
}

static int get_resource(const char *compat, struct resource *res,
			int (*cmp_func)(struct device_node *dn))
{
	struct device_node *dn;
	int ret = -ENODEV;

	for_each_compatible_node(dn, NULL, compat) {
		ret = of_address_to_resource(dn, 0, res);
		if (ret)
			continue;

		if (res->flags != IORESOURCE_MEM)
			continue;

		if (cmp_func) {
			ret = cmp_func(dn);
			if (ret)
				continue;
		}

		break;
	}

	return ret;
}

static int __init test_init(void)
{
	unsigned int times, base;
	struct resource res1, res2;
	int ret;

	ret = get_resource("ns16550a", &res1, NULL);
	if (ret) {
		pr_err("%s: failed to get UARTA resource\n", __func__);
		return ret;
	}

	ret = brcmstb_update32(res1.start, 0xf, 1);
	if (ret >= 0)
		pr_err("%s: this should fail!\n", __func__);

	ret = get_resource("brcm,brcmstb-gpio", &res1, brcmstb_aon_gpio);
	if (ret) {
		pr_err("%s: failed to obtain AON_GPIO controller\n", __func__);
		return ret;
	}

	base = request_linux_gpios(res1.end - res1.start + 4, 0);
	request_linux_gpios(res1.end - res1.start + 4, base);

	for (times = 0; times < 2; times++) {
		ret = get_resource("brcm,brcmstb-gpio", &res2, NULL);
		if (ret)
			pr_err("%s: failed to obtain GIO controller resource\n",
				__func__);

		ret = test_gpio_bank(res2.start, res2.end - res2.start + 4);
		if (ret)
			pr_err("%s: GIO_REG_START failed!\n", __func__);

		ret = test_gpio_bank(res1.start, res1.end - res1.start + 4);
		if (ret)
			pr_err("%s: GIO_AON_REG_START failed!\n", __func__);
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
