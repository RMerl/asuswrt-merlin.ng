#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_fdt.h>
#include <linux/of.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <linux/kernel.h>
#include "bcm_bootstate.h"
#include "bcm_ioremap_shared.h"
#if defined(CONFIG_BRCM_SMC_BOOT)
#include "ba_rpc_svc.h"
#endif

#define BOOTSTATE_DIR "bootstate"
#define RESET_REASON_FILE "reset_reason"
#define OLD_RESET_REASON_FILE "old_reset_reason"
#define RESET_STATUS_FILE "reset_status"
#define BOOT_FAILED_COUNT_FILE "boot_failed_count"
#define ACTIVE_IMG_FILE "active_image"

#define CHECK_NULL(ptr) ptr != NULL

static inline void __iomem* DEVM_IOREMAP_RESOURCE(struct device *dev, const struct resource *res)
{
	void __iomem *base = devm_ioremap_shared_resource(dev, res);

	if(IS_ERR(base))
		base=NULL;

	return base;
}
typedef void (*set_boot_reason_p)(uint32_t value);
typedef void (*clear_boot_reason_p)(void);
typedef uint32_t (*get_boot_reason_p)(void);
typedef void (*set_boot_failed_count_p)(uint32_t value);
typedef uint32_t (*get_boot_failed_count_p)(void);
typedef void (*clear_boot_failed_count_p)(void);
typedef void (*set_sel_img_id_p)(uint32_t value);
typedef uint32_t (*get_sel_img_id_p)(void);
typedef void (*clear_sel_img_id_p)(void);

struct spi_reset_reason
{
	uint32_t *glb_cntrl;
	uint32_t *flash_cntrl;
	uint32_t *profile;
	uint32_t *old_profile;

};
typedef struct boot_state_data
{
	uint32_t version;
	uint32_t *reset_status;
	union
	{
		volatile uint32_t *reset_reason;
		struct spi_reset_reason srr;
	};
	set_boot_reason_p set_boot_reason;
	get_boot_reason_p get_boot_reason;
	get_boot_reason_p get_old_boot_reason;
	set_boot_reason_p set_old_boot_reason;
	clear_boot_reason_p clear_boot_reason;
	set_boot_failed_count_p set_boot_failed_count;
	get_boot_failed_count_p get_boot_failed_count;
	clear_boot_failed_count_p clear_boot_failed_count;
	set_sel_img_id_p set_sel_img_id;
	get_sel_img_id_p get_sel_img_id;
	clear_sel_img_id_p clear_sel_img_id;
}boot_state_data;

static boot_state_data b_state_data;

struct proc_dir_entry *bootstate_proc_dir;

uint32_t bcmbca_get_reset_status(void)
{
	uint32_t resetStatus = -1;

	if(CHECK_NULL((b_state_data.reset_status)))
	{
		resetStatus = *b_state_data.reset_status & RESET_STATUS_MASK;
	}
	return resetStatus;
}

static void bcmbca_set_boot_reason_v1(uint32_t value)
{
	value &= BCM_RESET_REASON_FULL_MASK;
	if(CHECK_NULL((b_state_data.srr.glb_cntrl)))
	{
		*b_state_data.srr.glb_cntrl |= SPI_DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.profile)))
		{
			*b_state_data.srr.profile &= ~(BCM_RESET_REASON_FULL_MASK);
			*b_state_data.srr.profile |= value;
		}
	}

}

static void bcmbca_set_boot_failed_count_v1(uint32_t value)
{
	if(CHECK_NULL((b_state_data.srr.glb_cntrl)))
	{
		*b_state_data.srr.glb_cntrl |= SPI_DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.profile)))
		{
			*b_state_data.srr.profile &= ~(BOOT_FAILED_COUNT_MASK);
			*b_state_data.srr.profile |= (value << BOOT_FAILED_COUNT_BIT_SHIFT) & BOOT_FAILED_COUNT_MASK;
		}
	}

}
static void bcmbca_set_sel_img_id_v1(uint32_t value)
{
	if(CHECK_NULL((b_state_data.srr.glb_cntrl)))
	{
		*b_state_data.srr.glb_cntrl |= SPI_DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.profile)))
		{
			*b_state_data.srr.profile &= ~(BOOT_IMAGE_ID_MASK);
			*b_state_data.srr.profile |= (value << BOOT_IMAGE_ID_SHIFT)&BOOT_IMAGE_ID_MASK;
		}
	}
}

static void bcmbca_set_old_boot_reason_v1(uint32_t value)
{
	value &= BCM_RESET_REASON_FULL_MASK;
	if(CHECK_NULL((b_state_data.srr.glb_cntrl)))
	{
		*b_state_data.srr.glb_cntrl |= SPI_DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.old_profile)))
		{
			*b_state_data.srr.old_profile = value;
		}
	}

}

static void bcmbca_set_boot_reason_v2(uint32_t value)
{
	uint32_t tmp_val;
	int retries=0;

	value &= BCM_RESET_REASON_FULL_MASK;
	if(CHECK_NULL((b_state_data.reset_reason)))
	{

		value=(*b_state_data.reset_reason&(~BCM_RESET_REASON_FULL_MASK))|value;
		while(retries++ < 255)
		{
			tmp_val=*b_state_data.reset_reason;
			dsb(sy);
			*b_state_data.reset_reason=value;
			dsb(sy);
			tmp_val=*b_state_data.reset_reason;
			if(tmp_val == value)
				break;
		}
		if(retries > 1)
			printk("current value %d value to write %d retried [%d] times to write the reset_reason %s\n", tmp_val, value, retries, (tmp_val == value) ? "success":"fail");
	}
}

static void bcmbca_set_boot_failed_count_v2(uint32_t value)
{
	uint32_t tmp_val;
	int retries=0;

	if(CHECK_NULL((b_state_data.reset_reason)))
	{
		value = (*b_state_data.reset_reason & ~(BOOT_FAILED_COUNT_MASK)) | (value << BOOT_FAILED_COUNT_BIT_SHIFT);
		while(retries++ < 255)
		{
			tmp_val=*b_state_data.reset_reason;
			dsb(sy);
			*b_state_data.reset_reason=value;
			dsb(sy);
			tmp_val=*b_state_data.reset_reason;
			if(tmp_val == value)
				break;
		}
		if(retries > 1)
			printk("current value %d value to write %d retried [%d] times to write the reset_reason %s\n", tmp_val, value, retries, (tmp_val == value) ? "success":"fail");
	}
}
static void bcmbca_set_sel_img_id_v2(uint32_t value)
{
	uint32_t tmp_val;
	int retries=0;

	if(CHECK_NULL((b_state_data.reset_reason)))
	{
		value = (*b_state_data.reset_reason & ~(BOOT_IMAGE_ID_MASK)) | (value << BOOT_IMAGE_ID_SHIFT);
		while(retries++ < 255)
		{
			tmp_val=*b_state_data.reset_reason;
			dsb(sy);
			*b_state_data.reset_reason=value;
			dsb(sy);
			tmp_val=*b_state_data.reset_reason;
			if(tmp_val == value)
				break;
		}
		if(retries > 1)
			printk("current value %d value to write %d retried [%d] times to write the reset_reason %s\n", tmp_val, value, retries, (tmp_val == value) ? "success":"fail");
	}
}

void bcmbca_set_boot_reason(uint32_t value)
{
	if(CHECK_NULL(b_state_data.set_boot_reason))
	{
		b_state_data.set_boot_reason(value);
	}
}

void bcmbca_set_boot_failed_count(uint32_t value)
{
	if(CHECK_NULL(b_state_data.set_boot_failed_count))
	{
		b_state_data.set_boot_failed_count(value);
	}
}
void bcmbca_set_sel_img_id(uint32_t value)
{
	if(CHECK_NULL(b_state_data.set_sel_img_id))
	{
		b_state_data.set_sel_img_id(value);
	}
}

void bcmbca_set_old_boot_reason(uint32_t value)
{
	if(CHECK_NULL(b_state_data.set_old_boot_reason))
	{
		b_state_data.set_old_boot_reason(value);
	}
}

static void bcmbca_clear_boot_failed_count_v1(void)
{
	if(CHECK_NULL((b_state_data.srr.glb_cntrl)))
	{
		*b_state_data.srr.glb_cntrl &= ~SPI_DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.flash_cntrl)))
		{
			*b_state_data.srr.flash_cntrl = SPI_FLASH_CNTRL_RESET_VAL;
		}
		if(CHECK_NULL((b_state_data.srr.profile)))
		{
			*b_state_data.srr.profile &= ~(BOOT_FAILED_COUNT_MASK);
		}
	}
}

static void bcmbca_clear_boot_reason_v1(void)
{
	if(CHECK_NULL((b_state_data.srr.glb_cntrl)))
	{
		*b_state_data.srr.glb_cntrl &= ~SPI_DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.flash_cntrl)))
		{
			*b_state_data.srr.flash_cntrl = SPI_FLASH_CNTRL_RESET_VAL;
		}
		if(CHECK_NULL((b_state_data.srr.profile)))
		{
			*b_state_data.srr.profile &= ~(BCM_RESET_REASON_FULL_MASK);
		}
	}
}
static void bcmbca_clear_sel_img_id_v1(void)
{
	if(CHECK_NULL((b_state_data.srr.glb_cntrl)))
	{
		*b_state_data.srr.glb_cntrl &= ~SPI_DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.flash_cntrl)))
		{
			*b_state_data.srr.flash_cntrl = SPI_FLASH_CNTRL_RESET_VAL;
		}
		if(CHECK_NULL((b_state_data.srr.profile)))
		{
			*b_state_data.srr.profile &= ~(BOOT_IMAGE_ID_MASK);
		}
	}
}
static void bcmbca_clear_boot_reason_v2(void)
{
	uint32_t tmp_val;
	int retries=0;
	if(CHECK_NULL((b_state_data.reset_reason)))
	{
		while(retries++ < 255)
		{
			tmp_val=*b_state_data.reset_reason;
			dsb(sy);
			*b_state_data.reset_reason &= ~(BCM_RESET_REASON_FULL_MASK);
			dsb(sy);
			tmp_val=*b_state_data.reset_reason;
			if((tmp_val & (BCM_RESET_REASON_FULL_MASK)) == 0)
				break;
		}
		if(retries > 1)
			printk("retried [%d] times to clear the reset_reason %s\n", retries, ((tmp_val & (BCM_RESET_REASON_FULL_MASK)) == 0) ? "success":"fail");
	}
}
static void bcmbca_clear_boot_failed_count_v2(void)
{
	uint32_t tmp_val;
	int retries=0;
	if(CHECK_NULL((b_state_data.reset_reason)))
	{
		while(retries++ < 255)
		{
			tmp_val=*b_state_data.reset_reason;
			dsb(sy);
			*b_state_data.reset_reason &= ~(BOOT_FAILED_COUNT_MASK);
			dsb(sy);
			tmp_val=*b_state_data.reset_reason;
			if((tmp_val & (BOOT_FAILED_COUNT_MASK)) == 0)
				break;
		}
		if(retries > 1)
			printk("retried [%d] times to clear the reset_reason %s\n", retries, ((tmp_val & (BOOT_FAILED_COUNT_MASK)) == 0) ? "success":"fail");
	}
}
static void bcmbca_clear_sel_img_id_v2(void)
{
	uint32_t tmp_val;
	int retries=0;
	if(CHECK_NULL((b_state_data.reset_reason)))
	{
		while(retries++ < 255)
		{
			tmp_val=*b_state_data.reset_reason;
			dsb(sy);
			*b_state_data.reset_reason &= ~(BOOT_IMAGE_ID_MASK);
			dsb(sy);
			tmp_val=*b_state_data.reset_reason;
			if((tmp_val & (BOOT_IMAGE_ID_MASK)) == 0)
				break;
		}
		if(retries > 1)
			printk("retried [%d] times to clear the reset_reason %s\n", retries, ((tmp_val & (BOOT_IMAGE_ID_MASK)) == 0) ? "success":"fail");
	}
}

void bcmbca_clear_boot_reason(void)
{
	if(CHECK_NULL((b_state_data.clear_boot_reason)))
	{
		b_state_data.clear_boot_reason();
	}
}

void bcmbca_clear_boot_failed_count(void)
{
	if(CHECK_NULL((b_state_data.clear_boot_failed_count)))
	{
		b_state_data.clear_boot_failed_count();
	}
}
void bcmbca_clear_sel_img_id(void)
{
	if(CHECK_NULL((b_state_data.clear_sel_img_id)))
	{
		b_state_data.clear_sel_img_id();
	}
}
static uint32_t bcmbca_get_boot_reason_v1(void)
{
	uint32_t rc=-1;

	if(CHECK_NULL((b_state_data.srr.profile)))
	{
		rc=(*b_state_data.reset_status & SW_RESET_STATUS) != 0 ? *b_state_data.srr.profile & BCM_RESET_REASON_FULL_MASK:-1;
	}
	return rc;
}
static uint32_t bcmbca_get_boot_failed_count_v1(void)
{
	uint32_t rc=-1;

	if(CHECK_NULL((b_state_data.srr.profile)))
	{
		rc=(*b_state_data.reset_status & SW_RESET_STATUS) != 0 ? (*b_state_data.srr.profile & BOOT_FAILED_COUNT_MASK)>>BOOT_FAILED_COUNT_BIT_SHIFT:0;
	}
	return rc;
}
static uint32_t bcmbca_get_sel_img_id_v1(void)
{
	uint32_t rc=-1;

	if(CHECK_NULL((b_state_data.srr.profile)))
	{
		rc=(*b_state_data.reset_status & SW_RESET_STATUS) != 0 ? (*b_state_data.srr.profile & BOOT_IMAGE_ID_MASK)>>BOOT_IMAGE_ID_SHIFT:0;
	}
	return rc;
}
static uint32_t bcmbca_get_old_boot_reason_v1(void)
{
	uint32_t rc=-1;

	if(CHECK_NULL((b_state_data.srr.old_profile)))
	{
		rc=(*b_state_data.reset_status & SW_RESET_STATUS) != 0 ? *b_state_data.srr.old_profile & BCM_RESET_REASON_FULL_MASK:-1;
	}
	return rc;
}
static uint32_t bcmbca_get_boot_reason_v2(void)
{
	uint32_t rc=-1;

	//boot reason is only good in case of sw reset
	if((b_state_data.reset_reason != NULL && b_state_data.reset_status != NULL))
	{
#if defined(CONFIG_BRCM_SMC_BOOT)
		rc=*b_state_data.reset_reason & BCM_RESET_REASON_FULL_MASK;
#else
		rc=(*b_state_data.reset_status & SW_RESET_STATUS) != 0 ? *b_state_data.reset_reason & BCM_RESET_REASON_FULL_MASK:-1;
#endif
	}
	return rc;
}
static uint32_t bcmbca_get_boot_failed_count_v2 (void)
{
	uint32_t rc=-1;

	//boot reason is only good in case of sw reset
	if((b_state_data.reset_reason != NULL && b_state_data.reset_status != NULL))
	{
		rc=(*b_state_data.reset_status & SW_RESET_STATUS) != 0 ? ((*b_state_data.reset_reason & BOOT_FAILED_COUNT_MASK)>>BOOT_FAILED_COUNT_BIT_SHIFT):0;
	}
	return rc;
}

#if defined(CONFIG_BRCM_SMC_BOOT)
static uint32_t bcmbca_get_old_boot_reason_v2(void)
{
	uint32_t rc=-1;

	//boot reason is only good in case of sw reset
	if((b_state_data.reset_reason != NULL && b_state_data.reset_status != NULL))
	{
		rc=(*b_state_data.reset_status & SW_RESET_STATUS) != 0 ? (*b_state_data.reset_reason & 0xffff0000) >> 16:-1;
	}
	return rc;
}
#endif

static uint32_t bcmbca_get_sel_img_id_v2 (void)
{
	uint32_t rc=-1;

	//boot reason is only good in case of sw reset
	if((b_state_data.reset_reason != NULL && b_state_data.reset_status != NULL))
	{
		rc=(*b_state_data.reset_status & SW_RESET_STATUS) != 0 ? ((*b_state_data.reset_reason & BOOT_IMAGE_ID_MASK)>>BOOT_IMAGE_ID_SHIFT):0;
	}
	return rc;
}

uint32_t bcmbca_get_boot_reason(void)
{
	int rc=-1;
	if(CHECK_NULL((b_state_data.get_boot_reason)))
	{
		return b_state_data.get_boot_reason();
	}
	return rc;
}

uint32_t bcmbca_get_boot_failed_count(void)
{
	int rc=-1;
	if(CHECK_NULL((b_state_data.get_boot_failed_count)))
	{
		return b_state_data.get_boot_failed_count();
	}
	return rc;
}
uint32_t bcmbca_get_sel_img_id(void)
{
	int rc=-1;
	if(CHECK_NULL((b_state_data.get_sel_img_id)))
	{
		return b_state_data.get_sel_img_id();
	}
	return rc;
}

uint32_t bcmbca_get_old_boot_reason(void)
{
	int rc=0x22ff1;
	if(CHECK_NULL((b_state_data.get_old_boot_reason)))
	{
		return b_state_data.get_old_boot_reason();
	}
	return rc;
}


EXPORT_SYMBOL(bcmbca_set_boot_reason);
EXPORT_SYMBOL(bcmbca_clear_boot_reason);
EXPORT_SYMBOL(bcmbca_get_boot_reason);
EXPORT_SYMBOL(bcmbca_get_old_boot_reason);
EXPORT_SYMBOL(bcmbca_get_reset_status);
EXPORT_SYMBOL(bcmbca_set_old_boot_reason);
EXPORT_SYMBOL(bcmbca_get_boot_failed_count);
EXPORT_SYMBOL(bcmbca_set_boot_failed_count);
EXPORT_SYMBOL(bcmbca_clear_boot_failed_count);
EXPORT_SYMBOL(bcmbca_get_sel_img_id);
EXPORT_SYMBOL(bcmbca_set_sel_img_id);
EXPORT_SYMBOL(bcmbca_clear_sel_img_id);

static int old_reset_reason_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%x", bcmbca_get_old_boot_reason());
	return 0;
}

static int boot_failed_count_proc_show(struct seq_file *m, void *v)
{
#if defined(CONFIG_BRCM_SMC_BOOT)
	seq_printf(m, "%x", bcm_rpc_ba_get_boot_fail_cnt());
#else
	seq_printf(m, "%x", bcmbca_get_boot_failed_count());
#endif
	return 0;
}
static int sel_img_id_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%x", bcmbca_get_sel_img_id());
	return 0;
}
static int reset_reason_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%x", bcmbca_get_boot_reason());
	return 0;
}
static int reset_status_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%x", bcmbca_get_reset_status());
	return 0;
}

#if !defined(CONFIG_BRCM_SMC_BOOT)
static ssize_t boot_failed_count_proc_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *pos)
{
	char input[32];
	uint32_t reset_reason;
	memset(input, '\0', sizeof(input));

	if ((count > 32) || (copy_from_user(input, buffer, count) != 0))
		return -EFAULT;

	reset_reason=simple_strtoul(input, NULL, 16);
	bcmbca_set_boot_failed_count(reset_reason);
	return count;
}
#endif

static ssize_t reset_reason_proc_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *pos)
{
	char input[32];
	uint32_t reset_reason;
	memset(input, '\0', sizeof(input));

	if ((count > 32) || (copy_from_user(input, buffer, count) != 0))
		return -EFAULT;

	if(count >= strlen("steadystate") &&
		strncmp(input, "steadystate", strlen("steadystate")) == 0)
	{
#if defined(CONFIG_BRCM_SMC_BOOT)
		bcm_rpc_ba_report_boot_success(BA_SVC_RESET_BOOT_WDOG | BA_SVC_RESET_BOOT_COUNT);
#endif
		bcmbca_set_boot_reason((bcmbca_get_boot_reason() & 0xff00) | BCM_BOOT_PHASE_LINUX_RUN | BCM_BOOT_REASON_WATCHDOG);
	}
#if defined(CONFIG_BRCM_SMC_BOOT)
	else if(count >= strlen("reset_wd") && !strncmp(input, "reset_wd", strlen("reset_wd")))
	{
		reset_reason = (bcmbca_get_boot_reason() & 0xffffff00) | BCM_BOOT_PHASE_LINUX_RUN | BCM_BOOT_REASON_WATCHDOG;
		bcm_rpc_ba_report_boot_success(BA_SVC_RESET_BOOT_WDOG);
	}
	else if(count >= strlen("reset_cnt") && !strncmp(input, "reset_cnt", strlen("reset_cnt")))
	{
		reset_reason = (bcmbca_get_boot_reason() & 0xffffff00) | BCM_BOOT_PHASE_LINUX_RUN | BCM_BOOT_REASON_WATCHDOG;
		bcm_rpc_ba_report_boot_success(BA_SVC_RESET_BOOT_COUNT);
	}
#endif
	else
	{
		reset_reason=simple_strtoul(input, NULL, 16);
		bcmbca_set_boot_reason((bcmbca_get_boot_reason() & 0xff00) | (reset_reason & 0xff));

	}
	return count;
}

static ssize_t old_reset_reason_proc_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *pos)
{
	char input[32];
	uint32_t reset_reason;
	memset(input, '\0', sizeof(input));

	if ((count > 32) || (copy_from_user(input, buffer, count) != 0))
		return -EFAULT;

	reset_reason=simple_strtoul(input, NULL, 16);
	bcmbca_set_old_boot_reason(reset_reason);
	return count;
}

static int boot_failed_count_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, boot_failed_count_proc_show, PDE_DATA(inode));
}
static int reset_reason_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, reset_reason_proc_show, PDE_DATA(inode));
}
static int sel_img_id_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, sel_img_id_proc_show, PDE_DATA(inode));
}

static int old_reset_reason_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, old_reset_reason_proc_show, PDE_DATA(inode));
}

static int bootstate_v1_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret=0;
	b_state_data.clear_boot_reason = bcmbca_clear_boot_reason_v1;
	b_state_data.set_boot_reason = bcmbca_set_boot_reason_v1;
	b_state_data.get_boot_reason = bcmbca_get_boot_reason_v1;
	b_state_data.get_old_boot_reason = bcmbca_get_old_boot_reason_v1;
	b_state_data.set_old_boot_reason = bcmbca_set_old_boot_reason_v1;

	b_state_data.clear_boot_failed_count = bcmbca_clear_boot_failed_count_v1;
	b_state_data.set_boot_failed_count = bcmbca_set_boot_failed_count_v1;
	b_state_data.get_boot_failed_count = bcmbca_get_boot_failed_count_v1;

	b_state_data.clear_sel_img_id = bcmbca_clear_sel_img_id_v1;
	b_state_data.set_sel_img_id = bcmbca_set_sel_img_id_v1;
	b_state_data.get_sel_img_id = bcmbca_get_sel_img_id_v1;


	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "reset_status");
	if (res)
	{
		b_state_data.reset_status=DEVM_IOREMAP_RESOURCE(&pdev->dev, res);
	}
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "global_control");
	if (res)
	{
		b_state_data.srr.glb_cntrl=DEVM_IOREMAP_RESOURCE(&pdev->dev, res);
	}
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "flash_control");
	if (res)
	{
		b_state_data.srr.flash_cntrl=DEVM_IOREMAP_RESOURCE(&pdev->dev, res);
	}
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "mode_control");
	if (res)
	{
		b_state_data.srr.profile=DEVM_IOREMAP_RESOURCE(&pdev->dev, res);
	}
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "old_mode_control");
	if (res)
	{
		b_state_data.srr.old_profile=DEVM_IOREMAP_RESOURCE(&pdev->dev, res);
	}

	return ret;
}
static int bootstate_v2_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret=0;
	b_state_data.clear_boot_reason = bcmbca_clear_boot_reason_v2;
	b_state_data.set_boot_reason = bcmbca_set_boot_reason_v2;
	b_state_data.get_boot_reason = bcmbca_get_boot_reason_v2;
#if defined(CONFIG_BRCM_SMC_BOOT)
	b_state_data.get_old_boot_reason = bcmbca_get_old_boot_reason_v2;
#endif


	b_state_data.clear_boot_failed_count = bcmbca_clear_boot_failed_count_v2;
	b_state_data.set_boot_failed_count = bcmbca_set_boot_failed_count_v2;
	b_state_data.get_boot_failed_count = bcmbca_get_boot_failed_count_v2;

	b_state_data.clear_sel_img_id = bcmbca_clear_sel_img_id_v2;
	b_state_data.set_sel_img_id = bcmbca_set_sel_img_id_v2;
	b_state_data.get_sel_img_id = bcmbca_get_sel_img_id_v2;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "reset_status");
	if (res)
	{
		b_state_data.reset_status=DEVM_IOREMAP_RESOURCE(&pdev->dev, res);
	}
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "reset_reason");
	if (res)
	{
		b_state_data.reset_reason=DEVM_IOREMAP_RESOURCE(&pdev->dev, res);
	}
	return ret;
}


int bcm_bootstate_probe(struct platform_device *pdev)
{
	int rc=-1;
	char *compat;

	struct proc_dir_entry *p1;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
	static struct proc_ops sel_img_id_ops = {
		.proc_open    = sel_img_id_proc_open,
		.proc_read    = seq_read,
		.proc_release = single_release,
	};
	static struct proc_ops boot_failed_count_ops = {
		.proc_open    = boot_failed_count_proc_open,
		.proc_read    = seq_read,
		.proc_release = single_release,
#if !defined(CONFIG_BRCM_SMC_BOOT)
		.proc_write   = boot_failed_count_proc_write,
#endif
	};
	static struct proc_ops reset_reason_ops = {
		.proc_open    = reset_reason_proc_open,
		.proc_read    = seq_read,
		.proc_release = single_release,
		.proc_write   = reset_reason_proc_write,
	};
	static struct proc_ops old_reset_reason_ops = {
		.proc_open    = old_reset_reason_proc_open,
		.proc_read    = seq_read,
		.proc_release = single_release,
		.proc_write   = old_reset_reason_proc_write,
	};
#else
	static struct file_operations sel_img_id_ops = {
		.owner   = THIS_MODULE,
		.open    = sel_img_id_proc_open,
		.read    = seq_read,
		.release = single_release,
	};
	static struct file_operations boot_failed_count_ops = {
		.owner   = THIS_MODULE,
		.open    = boot_failed_count_proc_open,
		.read    = seq_read,
		.release = single_release,
#if !defined(CONFIG_BRCM_SMC_BOOT)
		.write   = boot_failed_count_proc_write,
#endif
	};
	static struct file_operations reset_reason_ops = {
		.owner   = THIS_MODULE,
		.open    = reset_reason_proc_open,
		.read    = seq_read,
		.release = single_release,
		.write   = reset_reason_proc_write,
	};
	static struct file_operations old_reset_reason_ops = {
		.owner   = THIS_MODULE,
		.open    = old_reset_reason_proc_open,
		.read    = seq_read,
		.release = single_release,
		.write   = old_reset_reason_proc_write,
	};
#endif
	//create proc entries
	bootstate_proc_dir = proc_mkdir(BOOTSTATE_DIR, NULL);
	if (bootstate_proc_dir == NULL) {
		printk("bootstate: failed to create proc dir [%s]!\n", BOOTSTATE_DIR);
		return -1;
	}

	p1 = proc_create_data(RESET_REASON_FILE, S_IRUSR, bootstate_proc_dir, &reset_reason_ops, NULL);

	if (p1 == NULL)
	{
		printk("bootstate: failed to create proc file! [%s]\n", RESET_REASON_FILE);
		return -1;
	}

	p1 = proc_create_data(OLD_RESET_REASON_FILE, S_IRUSR, bootstate_proc_dir, &old_reset_reason_ops, NULL);

	if (p1 == NULL)
	{
		printk("bootstate: failed to create proc file! [%s]\n", OLD_RESET_REASON_FILE);
		return -1;
	}

	p1 = proc_create_single(RESET_STATUS_FILE, S_IRUSR, bootstate_proc_dir, reset_status_proc_show);

	if (p1 == NULL)
	{
		printk("bootstate: failed to create proc file! [%s]\n", RESET_STATUS_FILE);
		return -1;
	}

	p1 = proc_create_data(BOOT_FAILED_COUNT_FILE, S_IRUSR, bootstate_proc_dir, &boot_failed_count_ops, NULL);

	if (p1 == NULL)
	{
		printk("bootstate: failed to create proc file! [%s]\n", BOOT_FAILED_COUNT_FILE);
		return -1;
	}
	p1 = proc_create_data(ACTIVE_IMG_FILE, S_IRUSR, bootstate_proc_dir, &sel_img_id_ops, NULL);

	if (p1 == NULL)
	{
		printk("bootstate: failed to create proc file! [%s]\n", ACTIVE_IMG_FILE);
		return -1;
	}

	memset(&b_state_data, '\0', sizeof(b_state_data));


	compat=(char*)of_get_property(pdev->dev.of_node, "compatible", NULL);
	if(compat != NULL)
	{
		if(strcmp(compat, "brcm,bcmbca-bootstate-v1") == 0)
		{
			rc=bootstate_v1_probe(pdev);
		}
		else if(strcmp(compat, "brcm,bcmbca-bootstate-v2") == 0)
		{
			rc=bootstate_v2_probe(pdev);
		}
	}

#if !defined(CONFIG_BRCM_SMC_BOOT)
	printk("BOOT REASON 0x%x\n", bcmbca_get_boot_reason());
#endif
	printk("RESET STATUS 0x%x\n", bcmbca_get_reset_status());
	return rc;
}

int bcm_bootstate_remove(struct platform_device *pdev)
{
	if (bootstate_proc_dir != NULL)
	{
		remove_proc_entry(RESET_REASON_FILE, bootstate_proc_dir);
		remove_proc_entry(RESET_STATUS_FILE, bootstate_proc_dir);
		remove_proc_entry(BOOT_FAILED_COUNT_FILE, bootstate_proc_dir);
		remove_proc_entry(BOOTSTATE_DIR, NULL);
	}
	return 0;
}

static const struct of_device_id bcm_bootstate_dt_ids[] = {
        { .compatible = "brcm,bcmbca-bootstate-v1"},
        { .compatible = "brcm,bcmbca-bootstate-v2"},
        { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, bcm_bootstate_dt_ids);

static struct platform_driver bcm_bootstate_driver = {
        .driver = {
                   .name = "bcm_bootstate",
                   .owner = THIS_MODULE,
                   .of_match_table = of_match_ptr(bcm_bootstate_dt_ids),
                   },
        .probe = bcm_bootstate_probe,
        .remove = bcm_bootstate_remove,
};



static int __init
bootstate_module_init(void)
{
	int rc;

        rc = platform_driver_register(&bcm_bootstate_driver);

	if (rc < 0)
	{
		pr_err("%s: Driver registration failed, error %d\n",
			__func__, rc);
		return rc;
	}
return 0;
}
static void __exit bootstate_module_exit(void)
{
	platform_driver_unregister(&bcm_bootstate_driver);
}

module_init(bootstate_module_init);
module_exit(bootstate_module_exit);


MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom bootstate driver");
MODULE_LICENSE("GPL v2");

