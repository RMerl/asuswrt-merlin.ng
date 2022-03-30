#include <stdlib.h>
#include <stdio.h>
#include <common.h>
#include <image.h>
#include <spl.h>
#include "tpl_params.h"
#include "bcm_secure.h"
#include "boot_blob.h"
#include "otp_map_cmn.h"
#ifdef CONFIG_ARMV7_NONSEC
#include <asm/armv7.h>
#include <asm/secure.h>
#endif


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
	spl_image->entry_point = (image_entry_t)CONFIG_SYS_TEXT_BASE;

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
	typedef void __noreturn(*image_entry_t) (void *);
	image_entry_t image_entry =
		(image_entry_t) CONFIG_TPL_TEXT_BASE;
	void *new_params = (void*)TPL_PARAMS_ADDR;

	memcpy(new_params, parms, sizeof(tpl_params));

	cleanup_before_linux();
	image_entry((void *)new_params);
}

void * load_spl_env(void *buffer)
{
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

int last_stage_init(void)
{
#ifdef CONFIG_ARM64

#else
	unsigned long machid = 0xffffffff;
#ifdef CONFIG_MACH_TYPE
	machid = CONFIG_MACH_TYPE;
#endif
	printf("Booting Linux...\n");

	armv7_init_nonsec();
	cleanup_before_linux();
	secure_ram_addr(_do_nonsec_entry)((void*)0x00108000,
					0, machid, 0x03000000);
#endif

    return 0;
}

void *board_fdt_blob_setup(void)
{
	/* DTB is backdoor loaded */
	return (void *)0x03000000;
}

uint32_t env_boot_magic_search_size(void)
{
	return 256*1024;
}

static int do_linux_ikos(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	run_command("env set -f fdt_high 0xFFFFFFFFFFFFFFFF", 0);
	/* back door load fit image at 0x3000000 in IKOS*/
	run_command("bootm 0x3000000#conf_linux", 0);

	return 0;
}

U_BOOT_CMD(
	boot_linux_ikos,	1,	1,	do_linux_ikos,
	"IKOS boot linux from backdoor loaded fit image ",
	""
);

#endif

otp_hw_cmn_err_t  otp_hw_read(otp_hw_cmn_t *dev, 
				u32 addr,
				otp_hw_cmn_row_conf_t* row_conf,
				u32* data,
				u32 size)
{
	*data = 0;
	return 0;
}

void bcm_sec_init(void)
{
}

int bcm_sec_do(bcm_sec_ctx_t ctx, bcm_sec_cb_arg_t arg[SEC_CTRL_ARG_MAX])
{
	return 0;
}

#if !defined(CONFIG_SYS_ARCH_TIMER)
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
#endif
