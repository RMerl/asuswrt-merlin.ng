#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_fdt.h>
#include <linux/of.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <linux/kernel.h>
#include "bcm_bootstate.h"
#include "bcm_ioremap_shared.h"

#define BOOTSTATE_DIR "bootstate"
#define RESET_REASON_FILE "reset_reason"
#define RESET_STATUS_FILE "reset_status"


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

struct spi_reset_reason
{
	uint32_t *glb_cntrl;
	uint32_t *flash_cntrl;
	uint32_t *profile;

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
	clear_boot_reason_p clear_boot_reason;
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
	
	if(CHECK_NULL((b_state_data.srr.glb_cntrl)))
	{
		*b_state_data.srr.glb_cntrl |= SPI_DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.profile)))
		{
			*b_state_data.srr.profile = value;
		}
	}

}
static void bcmbca_set_boot_reason_v2(uint32_t value)
{
	uint32_t tmp_val;
	int retries=0;

	if(CHECK_NULL((b_state_data.reset_reason)))
	{
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
			*b_state_data.srr.profile = 0;
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
			*b_state_data.reset_reason &= ~(0x1ffff);
			dsb(sy);
			tmp_val=*b_state_data.reset_reason;
			if((tmp_val & (0x1ffff)) == 0)
				break;
		}
		if(retries > 1)
			printk("retried [%d] times to clear the reset_reason %s\n", retries, ((tmp_val & (0x1ffff)) == 0) ? "success":"fail");
	}
}

void bcmbca_clear_boot_reason(void)
{
	if(CHECK_NULL((b_state_data.clear_boot_reason)))
	{
		b_state_data.clear_boot_reason();
	}
}

static uint32_t bcmbca_get_boot_reason_v1(void)
{
	uint32_t rc=-1;

	if(CHECK_NULL((b_state_data.srr.profile)))
	{
		rc=(*b_state_data.reset_status & SW_RESET_STATUS) != 0 ? *b_state_data.srr.profile & 0x1ffff:-1;
	}
	return rc;
}
static uint32_t bcmbca_get_boot_reason_v2(void)
{
	uint32_t rc=-1;

	//boot reason is only good in case of sw reset
	if((b_state_data.reset_reason != NULL && b_state_data.reset_status != NULL))
	{
		rc=(*b_state_data.reset_status & SW_RESET_STATUS) != 0 ? *b_state_data.reset_reason & 0x1ffff:-1;
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


EXPORT_SYMBOL(bcmbca_set_boot_reason);
EXPORT_SYMBOL(bcmbca_clear_boot_reason);
EXPORT_SYMBOL(bcmbca_get_boot_reason);

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

static ssize_t reset_reason_proc_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *pos)
{
	char input[32];
	uint32_t reset_reason;
	memset(input, '\0', sizeof(input));

	if ((count > 32) || (copy_from_user(input, buffer, count) != 0))
		return -EFAULT;

	reset_reason=simple_strtoul(input, NULL, 16);
	bcmbca_set_boot_reason(reset_reason);
	return count;
}

static int reset_reason_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, reset_reason_proc_show, PDE_DATA(inode));
}

static int bootstate_v1_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret=0;
	b_state_data.clear_boot_reason = bcmbca_clear_boot_reason_v1;
	b_state_data.set_boot_reason = bcmbca_set_boot_reason_v1;
	b_state_data.get_boot_reason = bcmbca_get_boot_reason_v1;
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

	return ret;
}
static int bootstate_v2_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret=0;
	b_state_data.clear_boot_reason = bcmbca_clear_boot_reason_v2;
	b_state_data.set_boot_reason = bcmbca_set_boot_reason_v2;
	b_state_data.get_boot_reason = bcmbca_get_boot_reason_v2;

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
	static struct file_operations reset_reason_ops = {
		.owner   = THIS_MODULE,
		.open    = reset_reason_proc_open,
		.read    = seq_read,
		.write   = reset_reason_proc_write,
	};
	
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
	p1 = proc_create_single(RESET_STATUS_FILE, S_IRUSR, bootstate_proc_dir, reset_status_proc_show);

	if (p1 == NULL)
	{
		printk("bootstate: failed to create proc file! [%s]\n", RESET_STATUS_FILE);
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
	
	return rc;
}

int bcm_bootstate_remove(struct platform_device *pdev)
{
	if (bootstate_proc_dir != NULL)
	{
		remove_proc_entry(RESET_REASON_FILE, bootstate_proc_dir);
		remove_proc_entry(RESET_STATUS_FILE, bootstate_proc_dir);
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

