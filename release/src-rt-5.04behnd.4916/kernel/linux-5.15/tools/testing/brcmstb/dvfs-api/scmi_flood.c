#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/brcmstb/brcmstb.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/fs.h>      /* Needed by filp */
#include <linux/uaccess.h>   /* Needed by segment descriptors */

/*
 * Stand-alone build example:
 *
 * $ cat Makefile
 * obj-m += scmi_flood.o
 * $ make ARCH=arm64 CROSS_COMPILE=aarch64-linux- \
 *	-C /work3/jquinlan/git/49-arm64/linux SUBDIRS=$PWD modules
 *
 * USAGE:
 *     insmod scmi_flood.ko [m_sleep=<int>] [nthreads=<int>] \
 *			    [run_for_secs=<int>] [bind=<int>]
 *
 * PARAMS:
 *    m_sleep		-- pause amount in 1 iter of an SCMI-generating thread.
 *    nthreads		-- number of SCMI-generating threads.
 *    run_for_secs	-- duration of the SCMI flood.
 *    bind              -- bind each thread to a successive cpu.
 */

static bool done;
static int bind;
static int nthreads = 5;
static int m_sleep = 1;
static int run_for_secs = 2;
module_param(bind, int, 0660);
module_param(m_sleep, int, 0660);
module_param(nthreads, int, 0660);
module_param(run_for_secs, int, 0660);

#define SYSFS_CPU_FREQ_FILE \
	"/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_cur_freq"
#define SYSFS_HWMON_FILE \
	"/sys/devices/platform/brcm_scmi@0/scmi_dev.3/hwmon/hwmon0/in1_input"

static void pause(void)
{
	mb();
	if (m_sleep)
		msleep(m_sleep);
	else
		yield();
}

/* Read a sysfs file so as to generate an SCMI request */
static void read_sysfs_file(const char *file)
{
	struct file *f;
	char buf[128];

	f = filp_open(file, O_RDONLY, 0);
	if (IS_ERR(f))
		return;
	/* Read the file */
	kernel_read(f, buf, 128, &f->f_pos);
	/* Restore segment descriptor */
	filp_close(f, NULL);
}

static int thread_brcm(void *data)
{

	while (!done) {
		brcm_reset_assert(BRST_SW_VICE0);
		brcm_reset_deassert(BRST_SW_VICE0);
		brcm_reset_assert(BRST_SW_VICE0);
		brcm_reset_deassert(BRST_SW_VICE0);
		pause();
	}
	return 0;
}

static int thread_pmic(void *data)
{
	const u8 pmic_num = 0;
	uint32_t die_temp, ext_therm_temp, overall_power;

	while (!done) {
		(void) brcmstb_stb_avs_get_pmic_status(
			pmic_num,
			&die_temp,
			&ext_therm_temp,
			&overall_power);
		(void) brcmstb_stb_avs_get_pmic_status(
			pmic_num,
			&die_temp,
			&ext_therm_temp,
			&overall_power);
		(void) brcmstb_stb_avs_get_pmic_status(
			pmic_num,
			&die_temp,
			&ext_therm_temp,
			&overall_power);
		pause();
	}
	return 0;
}

static int thread_clk(void *data)
{
	struct clk *clk0, *clk1;

	clk0 = clk_get(NULL, "sw_v3d");
	clk1 = clk_get(NULL, "sw_v3d_cpu");
	if (IS_ERR(clk0) || IS_ERR(clk1)) {
		pr_info("Error: could not open a clock!\n");
		return -EIO;
	}

	while (!done) {
		clk_prepare_enable(clk0);
		clk_prepare_enable(clk1);
		clk_disable_unprepare(clk1);
		clk_disable_unprepare(clk0);
		pause();
	}
	clk_put(clk0);
	clk_put(clk1);
	return 0;
}

static int thread_hwmon(void *data)
{
	while (!done) {
		read_sysfs_file((const char *)data);
		read_sysfs_file((const char *)data);
		read_sysfs_file((const char *)data);
		pause();
	}
	return 0;
}

static int thread_perf(void *data)
{
	while (!done) {
		read_sysfs_file((const char *)data);
		read_sysfs_file((const char *)data);
		read_sysfs_file((const char *)data);
		pause();
	}
	return 0;
}

struct scmi_thread_info {
	int (*f)(void *data);
	const char *str;
	void *data;
};

struct scmi_thread_info threads_arr[] = {
	{ thread_pmic, "pmic", NULL, },
	{ thread_brcm, "brcm", NULL, },
	{ thread_clk, "clock", NULL, },
	{ thread_perf, "perf", SYSFS_CPU_FREQ_FILE, },
	{ thread_hwmon, "sensor", SYSFS_HWMON_FILE, },
};

static int scmi_flood_init(void)
{
	int i;
	struct scmi_thread_info *t;
	const int ncpus = num_online_cpus();

	for (i = 0; i < nthreads; i++) {
		struct task_struct *th;

		t = &threads_arr[i % ARRAY_SIZE(threads_arr)];
		th = kthread_create(t->f, t->data, "t%d-%s", i, t->str);
		if (IS_ERR(th))
			continue;

		if (bind) {
			kthread_bind(th, (i + bind) % ncpus);
			pr_info("Started t%d (proto %s) on cpu%d\n",
				i, t->str, (i + bind) % ncpus);
		} else {
			pr_info("Started t%d (proto %s)\n", i, t->str);
		}

		wake_up_process(th);
	}

	msleep(1000 * run_for_secs);
	done = true;
	mb();
	msleep(2000);

	return -EIO;
}


static void scmi_flood_exit(void)
{
}

module_init(scmi_flood_init);
module_exit(scmi_flood_exit);
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
