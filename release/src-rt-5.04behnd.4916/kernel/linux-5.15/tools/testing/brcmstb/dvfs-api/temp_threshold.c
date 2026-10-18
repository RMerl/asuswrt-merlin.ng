#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/brcmstb/brcmstb.h>
#include <linux/delay.h>

/*
 * USAGE:
 *   insmod temp_threshold.ko  [temp=<int>]
 *
 * EXAMPLES:
 *
 *   insmod temp_threshold.ko resettemp=100 parkhigh=100 parklow=110
 *
 * NOTE:
 *   The insmod will always fail to do the insmod operation but
 *   should execute the temp thresold setting this is by design so the
 *   user does not have to keep running an rmmod for every insmod.
 */


/*
 * Stand-alone build example:
 *
 * $ cat Makefile
 * obj-m += temp_threshold.o
 * $ make ARCH=arm64 CROSS_COMPILE=aarch64-linux- \
 *	-C /work3/kd923030/git/stblinux-49/linux SUBDIRS=$PWD modules
 */

static int resettemp = 110;
module_param(resettemp, int, 0660);

static int parkhigh = 110;
module_param(parkhigh, int, 0660);

static int parklow = 100;
module_param(parklow, int, 0660);

static int temp_threshold_init(void)
{
	int ret;

	if (resettemp > 120 || resettemp < 40 ||
	    parkhigh > 120 || parkhigh < 40 ||
	    parklow > 120 || parklow < 40) {
		pr_info("USAGE:\n");
		pr_info("insmod temp_threshold.ko [resettemp=<int> parkhigh=<int> parklow=<int>\n");
		pr_info("EXAMPLE:\n");
		pr_info("\tinsmod temp_threshold.ko resettemp=99 parkhigh=100 parklow=110\n");
		return -EIO;
	}

	ret = brcm_overtemp_reset_threshold(resettemp*1000, parkhigh*1000, parklow*1000);

	pr_info("%s:%d reset temp %d parkhigh %d parklow %d\n\n", ret ? "FAILED" : "Succeeded",
		ret, resettemp, parkhigh, parklow);

	return -EIO;
}

static void temp_threshold_exit(void)
{
}

module_init(temp_threshold_init);
module_exit(temp_threshold_exit);
MODULE_LICENSE("GPL");
