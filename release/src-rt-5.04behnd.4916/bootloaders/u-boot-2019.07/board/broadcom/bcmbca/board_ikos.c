#include <stdlib.h>
#include <stdio.h>
#include <common.h>
#include <image.h>
#include <spl.h>
#include <ubispl.h>
#include "tpl_params.h"
#include "bcm_secure.h"
#include "boot_blob.h"
#include "boot_flash.h"
#include "otp_map_cmn.h"

DECLARE_GLOBAL_DATA_PTR;

typedef void __noreturn(*image_entry_t) (void *);

/* Override function for ikos implementation */

#if defined(CONFIG_SPL_BUILD)

#if defined(CONFIG_TPL_BUILD)
int tpl_ram_load_image(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev)
{
	printf("ikos set tpl image entry to u-boot directly\n");

	spl_image->os = IH_OS_U_BOOT;
	spl_image->entry_point = (uintptr_t)((image_entry_t)CONFIG_SYS_TEXT_BASE);

	return 0;
}

int get_raw_metadata(void *buffer, struct ubispl_info *info, int n)
{
	return 0;
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_RAM;
}

SPL_LOAD_IMAGE_METHOD("RAM", 0, BOOT_DEVICE_RAM, tpl_ram_load_image);
#else
void start_tpl(tpl_params *parms)
{ 
	image_entry_t image_entry =
		(image_entry_t) CONFIG_TPL_TEXT_BASE;
	void *new_params = (void*)TPL_PARAMS_ADDR;

	memcpy(new_params, parms, sizeof(tpl_params));

	cleanup_before_linux();
	image_entry((void *)new_params);
}

void * load_spl_env(void *buffer)
{
	return buffer;
}
#endif

int nand_is_bad_block(unsigned int blk)
{
	/* simulatioin/ikos enironment does not support spare area */
	return 0;
}

void early_abort(void)
{

}
#else

char * get_loader_media(void)
{
#if defined(CONFIG_NAND) || defined(CONFIG_MTD_SPI_NAND)
	return FLASH_DEV_STR_NAND;
#elif defined(CONFIG_SPI_FLASH)
	return FLASH_DEV_STR_SPINOR;
#elif defined(CONFIG_MMC)
	return FLASH_DEV_STR_EMMC;
#else
	return NULL;
#endif
}

uint32_t env_boot_magic_search_size(void)
{
	return 128*1024;
}
#endif

#if !defined(CONFIG_SYS_ARCH_TIMER)
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
#endif

#ifdef CONFIG_ARM64
/* the u-boot version calculates the exact size recursively and currently the page
   table size is 100KB. Use fixed 256KB size to speed up the IKOS run */
u64 get_page_table_size(void)
{
	return 256*1024;
}
#endif

void jump_to_image(uintptr_t entry, void* param, int clean_cache)
{
	image_entry_t image_entry = (image_entry_t)entry;

	printf("ikos jump to image entry 0x%p directly\n", (void*)entry);
	if (clean_cache)
		cleanup_before_linux();

	image_entry(param);
}
