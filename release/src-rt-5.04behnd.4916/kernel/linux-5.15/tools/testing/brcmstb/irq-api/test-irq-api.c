#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sizes.h>
#include <linux/brcmstb/irq_api.h>

static int __init test_init(void)
{
	brcmstb_l2_irq irq;
	int ret;

	for (irq = 0; irq < brcmstb_l2_irq_max; irq++) {
		ret = brcmstb_get_l2_irq_id(irq);
		if (ret < 0) {
			pr_err("IRQ %d fails to be obtained, ret=%d\n",
				irq, ret);
			continue;
		}

		pr_info("%s: Found VIRQ%d for IRQ%d\n",
			__func__, ret, irq);
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
