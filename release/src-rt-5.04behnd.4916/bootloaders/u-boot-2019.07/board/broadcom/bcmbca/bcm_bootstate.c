#include <common.h>
#include <asm/u-boot.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <asm/arch/cpu.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <dm.h>


#include "bcm_bootstate.h"

DECLARE_GLOBAL_DATA_PTR;

#define CHECK_NULL(ptr) ptr != NULL //|| init_regs()

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
	uint32_t *reset_status;
	union
	{
		volatile uint32_t *reset_reason;
		struct spi_reset_reason srr;
	};
	set_boot_reason_p set_boot_reason;
	get_boot_reason_p get_boot_reason;
	get_boot_reason_p get_old_boot_reason;
	clear_boot_reason_p clear_boot_reason;
	set_boot_failed_count_p set_boot_failed_count;
	get_boot_failed_count_p get_boot_failed_count;
	clear_boot_failed_count_p clear_boot_failed_count;
	set_sel_img_id_p set_sel_img_id;
	get_sel_img_id_p get_sel_img_id;
	clear_sel_img_id_p clear_sel_img_id;
}boot_state_data;

boot_state_data __attribute__((section(".data"))) b_state_data;

int bcmbca_get_reset_status(void)
{
	int resetStatus = -1;
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
		*b_state_data.srr.glb_cntrl |= DO_NOT_RESET_ON_WATCHDOG;
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
		*b_state_data.srr.glb_cntrl |= DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.profile)))
		{
			*b_state_data.srr.profile &= ~(BOOT_FAILED_COUNT_MASK);
			*b_state_data.srr.profile |= (value << BOOT_FAILED_COUNT_BIT_SHIFT);
		}
	}

}
static void bcmbca_set_sel_img_id_v1(uint32_t value)
{


	if(CHECK_NULL((b_state_data.srr.glb_cntrl)))
	{
		*b_state_data.srr.glb_cntrl |= DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.profile)))
		{
			*b_state_data.srr.profile &= ~(BOOT_IMAGE_ID_MASK);
			*b_state_data.srr.profile |= ((value << BOOT_IMAGE_ID_SHIFT)&BOOT_IMAGE_ID_MASK);
		}
	}

}
static void bcmbca_set_boot_reason_v2(uint32_t value)
{
	uint32_t tmp_val=-1;
	int retries=0;

	if(CHECK_NULL((b_state_data.reset_reason)))
	{
		value=(*b_state_data.reset_reason&(~BCM_RESET_REASON_FULL_MASK))|value;
		while(retries++ < 255) 
		{
			tmp_val=*b_state_data.reset_reason;
			dsb();
			*b_state_data.reset_reason=value;
			dsb();
			tmp_val=*b_state_data.reset_reason;
			if(tmp_val == value)
				break;
		}
		if(retries > 1)
			printf("current value %d value to write %d retried [%d] times to write the reset_reason %s\n", tmp_val, value, retries, (tmp_val == value) ? "success":"fail");
	}
}
static void bcmbca_set_boot_failed_count_v2(uint32_t value)
{
	uint32_t tmp_val=-1;
	int retries=0;

	if(CHECK_NULL((b_state_data.reset_reason)))
	{
		value = (*b_state_data.reset_reason & ~(BOOT_FAILED_COUNT_MASK)) | (value << BOOT_FAILED_COUNT_BIT_SHIFT);
		while(retries++ < 255)
		{
			tmp_val=*b_state_data.reset_reason;
			dsb();
			*b_state_data.reset_reason=value;
			dsb();
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
	uint32_t tmp_val=-1;
	int retries=0;

	if(CHECK_NULL((b_state_data.reset_reason)))
	{
		value = (*b_state_data.reset_reason & ~(BOOT_IMAGE_ID_MASK)) | ((value << BOOT_IMAGE_ID_SHIFT)&BOOT_IMAGE_ID_MASK);
		while(retries++ < 255)
		{
			tmp_val=*b_state_data.reset_reason;
			dsb();
			*b_state_data.reset_reason=value;
			dsb();
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

static void bcmbca_clear_boot_failed_count_v1(void)
{
	if(CHECK_NULL((b_state_data.srr.glb_cntrl)))
	{
		*b_state_data.srr.glb_cntrl &= ~DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.flash_cntrl)))
		{
			*b_state_data.srr.flash_cntrl = FLASH_CNTRL_RESET_VAL;
		}
		if(CHECK_NULL((b_state_data.srr.profile)))
		{
			*b_state_data.srr.profile &= ~(BOOT_FAILED_COUNT_MASK);
		}
	}
}
static void bcmbca_clear_sel_img_id_v1(void)
{
	if(CHECK_NULL((b_state_data.srr.glb_cntrl)))
	{
		*b_state_data.srr.glb_cntrl &= ~DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.flash_cntrl)))
		{
			*b_state_data.srr.flash_cntrl = FLASH_CNTRL_RESET_VAL;
		}
		if(CHECK_NULL((b_state_data.srr.profile)))
		{
			*b_state_data.srr.profile &= ~(BOOT_IMAGE_ID_MASK);
		}
	}
}

static void bcmbca_clear_boot_reason_v1(void)
{
	if(CHECK_NULL((b_state_data.srr.glb_cntrl)))
	{
		*b_state_data.srr.glb_cntrl &= ~DO_NOT_RESET_ON_WATCHDOG;
		if(CHECK_NULL((b_state_data.srr.flash_cntrl)))
		{
			*b_state_data.srr.flash_cntrl = FLASH_CNTRL_RESET_VAL;
		}
		if(CHECK_NULL((b_state_data.srr.profile)))
		{
			*b_state_data.srr.profile &= ~(BCM_RESET_REASON_FULL_MASK);
		}
	}
}
static void bcmbca_clear_boot_reason_v2(void)
{
	uint32_t tmp_val=-1;
	int retries=0;
	if(CHECK_NULL((b_state_data.reset_reason)))
	{
		while(retries++ < 255)
		{
			tmp_val=*b_state_data.reset_reason;
			dsb();
			*b_state_data.reset_reason &= ~(BCM_RESET_REASON_FULL_MASK);
			dsb();
			tmp_val=*b_state_data.reset_reason;
			if((tmp_val & (BCM_RESET_REASON_FULL_MASK)) == 0)
				break;
		}
		if(retries > 1)
			printf("retried [%d] times to clear the reset_reason %s\n", retries, ((tmp_val & (BCM_RESET_REASON_FULL_MASK)) == 0) ? "success":"fail");
	}
}
static void bcmbca_clear_boot_failed_count_v2(void)
{
	uint32_t tmp_val=-1;
	int retries=0;
	if(CHECK_NULL((b_state_data.reset_reason)))
	{
		while(retries++ < 255)
		{
			tmp_val=*b_state_data.reset_reason;
			dsb();
			*b_state_data.reset_reason &= ~(BOOT_FAILED_COUNT_MASK);
			dsb();
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
	uint32_t tmp_val=-1;
	int retries=0;
	if(CHECK_NULL((b_state_data.reset_reason)))
	{
		while(retries++ < 255)
		{
			tmp_val=*b_state_data.reset_reason;
			dsb();
			*b_state_data.reset_reason &= ~(BOOT_IMAGE_ID_MASK);
			dsb();
			tmp_val=*b_state_data.reset_reason;
			if((tmp_val & (BCM_RESET_REASON_FULL_MASK)) == 0)
				break;
		}
		if(retries > 1)
			printf("retried [%d] times to clear the reset_reason %s\n", retries, ((tmp_val & (BOOT_IMAGE_ID_MASK)) == 0) ? "success":"fail");
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

static uint32_t bcmbca_get_boot_reason_v1(void)
{
	uint32_t rc=-1;

	if(CHECK_NULL((b_state_data.srr.profile)))
	{
		rc=(*b_state_data.reset_status & SW_RESET_STATUS) != 0 ? *b_state_data.srr.profile & BCM_RESET_REASON_FULL_MASK:-1;
	}
	return rc;
}
static uint32_t bcmbca_get_boot_reason_v2(void)
{
	uint32_t rc=-1;

	//boot reason is only good in case of sw reset
	if((b_state_data.reset_reason != NULL && b_state_data.reset_status != NULL))
	{
		rc=(*b_state_data.reset_status & SW_RESET_STATUS) != 0 ? *b_state_data.reset_reason & BCM_RESET_REASON_FULL_MASK:-1;
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
static uint32_t bcmbca_get_sel_img_id_v2(void)
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
		rc = b_state_data.get_boot_reason();
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
	int rc=0x22ff; // default value needs to be 0x22ff
	if(CHECK_NULL((b_state_data.get_old_boot_reason)))
	{
		rc = b_state_data.get_old_boot_reason();
	}
	return rc;
}

static int bootstate_v1_probe(struct udevice *dev)
{
	struct resource res;
	int ret=0;
	b_state_data.clear_boot_reason = bcmbca_clear_boot_reason_v1;
	b_state_data.set_boot_reason = bcmbca_set_boot_reason_v1;
	b_state_data.get_boot_reason = bcmbca_get_boot_reason_v1;
	b_state_data.get_old_boot_reason = bcmbca_get_old_boot_reason_v1;
	b_state_data.clear_boot_failed_count = bcmbca_clear_boot_failed_count_v1;
	b_state_data.set_boot_failed_count = bcmbca_set_boot_failed_count_v1;
	b_state_data.get_boot_failed_count = bcmbca_get_boot_failed_count_v1;
	b_state_data.clear_sel_img_id = bcmbca_clear_sel_img_id_v1;
	b_state_data.set_sel_img_id = bcmbca_set_sel_img_id_v1;
	b_state_data.get_sel_img_id = bcmbca_get_sel_img_id_v1;
	ret = dev_read_resource_byname(dev, "reset_status", &res);
        if (!ret) {
		b_state_data.reset_status=devm_ioremap(dev, res.start, resource_size(&res));
	}
	ret = dev_read_resource_byname(dev, "global_control", &res);
        if (!ret) {
		b_state_data.srr.glb_cntrl=devm_ioremap(dev, res.start, resource_size(&res));
	}
	ret = dev_read_resource_byname(dev, "flash_control", &res);
        if (!ret) {
		b_state_data.srr.flash_cntrl=devm_ioremap(dev, res.start, resource_size(&res));
	}
	ret = dev_read_resource_byname(dev, "mode_control", &res);
        if (!ret) {
		b_state_data.srr.profile=devm_ioremap(dev, res.start, resource_size(&res));
	}
	ret = dev_read_resource_byname(dev, "old_mode_control", &res);
        if (!ret) {
		b_state_data.srr.old_profile=devm_ioremap(dev, res.start, resource_size(&res));
	}
	return 0;
}
static int bootstate_v2_probe(struct udevice *dev)
{
	struct resource res;
	int ret=0;
	b_state_data.clear_boot_reason = bcmbca_clear_boot_reason_v2;
	b_state_data.set_boot_reason = bcmbca_set_boot_reason_v2;
	b_state_data.get_boot_reason = bcmbca_get_boot_reason_v2;
	b_state_data.get_old_boot_reason = NULL;
	b_state_data.clear_boot_failed_count = bcmbca_clear_boot_failed_count_v2;
	b_state_data.set_boot_failed_count = bcmbca_set_boot_failed_count_v2;
	b_state_data.get_boot_failed_count = bcmbca_get_boot_failed_count_v2;
	b_state_data.clear_sel_img_id = bcmbca_clear_sel_img_id_v2;
	b_state_data.set_sel_img_id = bcmbca_set_sel_img_id_v2;
	b_state_data.get_sel_img_id = bcmbca_get_sel_img_id_v2;
	ret = dev_read_resource_byname(dev, "reset_status", &res);
        if (!ret) {
		b_state_data.reset_status=devm_ioremap(dev, res.start, resource_size(&res));
	}
	ret = dev_read_resource_byname(dev, "reset_reason", &res);
        if (!ret) {
		b_state_data.reset_reason=devm_ioremap(dev, res.start, resource_size(&res));
	}
	return 0;
}

static const struct udevice_id bootstate_v1_ids[] = {
        { .compatible = "brcm,bcmbca-bootstate-v1" },
        { }
};
static const struct udevice_id bootstate_v2_ids[] = {
        { .compatible = "brcm,bcmbca-bootstate-v2" },
        { }
};

U_BOOT_DRIVER(bootstate_v1_drv) = {
        .name = "bcm_bootsate_v1",
        .id = UCLASS_NOP,
        .of_match = bootstate_v1_ids,
        .probe = bootstate_v1_probe,
};
U_BOOT_DRIVER(bootstate_v2_drv) = {
        .name = "bcm_bootsate_v2",
        .id = UCLASS_NOP,
        .of_match = bootstate_v2_ids,
        .probe = bootstate_v2_probe,
};

void bca_bootstate_probe(void)
{
	struct udevice *dev;

	memset(&b_state_data, '\0', sizeof(b_state_data));

	//for (uclass_first_device_check(UCLASS_MISC, &dev); dev; uclass_next_device_check(&dev));

	uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(bootstate_v2_drv), &dev);
	uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(bootstate_v1_drv), &dev);
}
