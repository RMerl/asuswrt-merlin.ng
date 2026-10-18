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
 * obj-m += set_pstate.o
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

static int target = -1;
module_param(target, int, 0660);

static char *core = "invalid";
module_param(core, charp, 0660);

static int set_pstate_init(void)
{
	int i;
	const int N = ARRAY_SIZE(my_cores);
	int ret;
	int selected = -1;

	for (i = 0; i < N; i++)
		if (strcmp(my_core_strs[i], core) == 0)
			selected = i;
	if (selected < 0 || target < 0) {
		pr_info("USAGE:\n");
		pr_info("  insmod set_pstate.ko core=<core> target=<int>\n\n");
		pr_info("Possible cores and pstates:\n");
		for (i = 0; i < N; i++) {
			int num_pstates = 0;

			ret = brcm_pmap_num_pstates(my_cores[i], &num_pstates);
			if (ret)
				pr_info("    [xxxx] %s\n", my_core_strs[i]);
			else
				pr_info("    [0..%d] %s\n", num_pstates - 1,
					my_core_strs[i]);
		}
		pr_info("\n\n");
	} else {
		int actual = -1;

		ret = brcm_pmap_set_pstate(my_cores[selected], target);
		if (!ret)
			ret = brcm_pmap_get_pstate(my_cores[selected], &actual);
		pr_info("Set core %s to P%d => %s\n", core, target,
			ret || actual != target ? "FAIL" : "PASS");
	}

	return -EIO;
}


static void set_pstate_exit(void)
{
}

module_init(set_pstate_init);
module_exit(set_pstate_exit);
MODULE_LICENSE("GPL");
