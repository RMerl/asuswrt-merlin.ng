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
 * Stand-alone build example:
 *
 * $ cat Makefile
 * obj-m += num_pstates.o
 * $ make ARCH=arm64 CROSS_COMPILE=aarch64-linux- \
 *	-C /work3/jquinlan/git/49-arm64/linux SUBDIRS=$PWD modules
 */

int my_cores[] = {
	BCLK_SW_V3D,
	BCLK_SW_HVD0,
	BCLK_SW_M2MC0,
	BCLK_SW_RAAGA0,
	BCLK_SW_VICE0,
	BCLK_SW_XPT,
	BCLK_SW_MIPMAP0,
	BCLK_SW_TSX0,
	BCLK_SW_SMARTCARD0,
};

const char *my_core_strs[] = {
	"v3d",
	"hvd",
	"m2mc",
	"raaga",
	"vice",
	"xpt",
	"mipmap",
	"tsx",
	"sc",
};

static char *core = "invalid";
module_param(core, charp, 0660);

static int num_pstates_init(void)
{
	int i;
	const int N = ARRAY_SIZE(my_cores);
	int ret;
	int selected = -1;
	int num_pstates = -1;

	for (i = 0; i < N; i++)
		if (strcmp(my_core_strs[i], core) == 0)
			selected = i;
	if (selected < 0) {
		pr_info("USAGE:\n  insmod num_pstates.ko core=<core>\n\n");
		pr_info("NOTE:\n  By design, the insmod will fail.\n\n");
		pr_info("Possible cores and pstates:\n");
		for (i = 0; i < N; i++) {
			num_pstates = 0;
			ret = brcm_pmap_num_pstates(my_cores[i], &num_pstates);
			if (ret)
				pr_info("    [xxxx] %s\n", my_core_strs[i]);
			else
				pr_info("    [0..%d] %s\n", num_pstates - 1,
					my_core_strs[i]);
		}
		pr_info("\n\n");
	} else {
		ret = brcm_pmap_num_pstates(my_cores[selected], &num_pstates);
		pr_info("%s NUM_PSTATES: %d\n", ret ? "FAIL" : "PASS",
			ret ? -1 : num_pstates);
	}

	return -EIO;
}


static void num_pstates_exit(void)
{
}

module_init(num_pstates_init);
module_exit(num_pstates_exit);
MODULE_LICENSE("GPL");
