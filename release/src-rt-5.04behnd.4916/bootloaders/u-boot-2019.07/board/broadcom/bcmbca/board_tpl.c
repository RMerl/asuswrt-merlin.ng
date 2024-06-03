/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */
#define DEBUG
#include <common.h>
#include <mmc.h>
#include <spl.h>
#include <nand.h>
#include <dm.h>
#include <version.h>
#if defined(CONFIG_ARM64)
#include <asm/armv8/mmu.h>
#endif
#include <linux/io.h>
#include <linux/ioport.h>

#include "spl_ddrinit.h"
#include "bcmbca_nand_spl.h"
#include "bca_common.h"
#include "bca_sdk.h"
#include "bcm_bootstate.h"
#include <time.h>
#include <ubispl.h>
#include "early_abort.h"
#include "tpl_params.h"
#include "spl_env.h"
#include <environment.h>
#include <hexdump.h>
#include "bcmbca-dtsetup.h"
#include "bcm_secure.h"
#if defined(CONFIG_BCMBCA_OTP)
#include "bcm_otp.h"
#endif
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#endif
#if defined(CONFIG_TPL_BCMBCA_UBUS4_DCM)
#include "bcm_ubus4.h"
#endif
#include "tpl_common.h"
#if defined(CONFIG_BCMBCA_ITC_RPC)
#include "itc_rpc.h"
#endif
#if defined(CONFIG_BCMBCA_VFBIO)
#include "vfbio.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

tpl_params __attribute__((section(".data"))) * tplparams = NULL;

#define TPL_BLD_DATE_STRING  U_BOOT_DATE " - " U_BOOT_TIME  


#if defined(CONFIG_BCM_BOOTSTATE_FALLBACK_SUPPORT)
static int fallback_needed=0;
#endif

static struct bcasdk_ctx sdk_ctx = {};

int bcm_sec_fit(void * fit);
void save_boot_params_ret(void);
int get_fit_load_vol_id(bcaspl_part_info *info);

#if !defined(CONFIG_BCMBCA_IKOS)
static void update_uboot_fdt_sdk(void *fdt_addr)
{
	int offset;
	int node;

	offset=fdt_path_offset (fdt_addr, "/chosen");
	if(offset >= 0)
	{
		if (sdk_ctx.last_reset_reason >= 0) {
			if(fdt_setprop_u32(fdt_addr, offset, "reset_reason", sdk_ctx.last_reset_reason))
			{
				printf("Could not set reset reason node in the fdt, device may not boot properly\n");
			}
		}
		if(fdt_setprop_u32(fdt_addr, offset, "active_image", sdk_ctx.active_image))
		{
			printf("Could not set image node in the fdt, device may not boot properly\n");
		}
		/* Add Loader version info */
        	node = fdt_add_subnode(fdt_addr, offset, "loader_info");
		if( node < 0) {
			printf("ERROR: Could not create %s node!\n", "loader_info");
			return;
		}

		fdt_setprop_string(fdt_addr, node, "build_date", TPL_BLD_DATE_STRING);

#if defined(BUILD_TAG)
		fdt_setprop_string(fdt_addr, node, "build_tag", BUILD_TAG);
#endif
	}

}
#endif

__weak void boost_cpu_clock(void)
{

}

/* called from start.S. No stack use. */
void save_boot_params(void* params)
{
#if defined(CONFIG_SMC_BASED) && !defined(CONFIG_BCMBCA_NO_SMC_BOOT)
	tplparams = (tpl_params*)TPL_PARAMS_ADDR;
	tplparams->environment = (void *)TPL_ENV_ADDR;
	tplparams->early_flags = 0;
	tplparams->ddr_size = 0; // will be filled later from DT
	tplparams->boot_device = BOOT_DEVICE_NONE;
#else
	tplparams = (tpl_params*)params;
#endif
	save_boot_params_ret();
}

#if !defined(CONFIG_BCMBCA_IKOS)
void board_spl_fit_post_load(ulong load_addr, size_t length)
{
	bcm_sec_delg_on_post_load((u8*)load_addr);
}

void spl_perform_fixups(struct spl_image_info *spl_image)
{
#if defined(CONFIG_BCMBCA_ITC_RPC)
	rpc_exit();
#endif
	update_uboot_fdt(spl_image->fdt_addr, tplparams);
	update_uboot_fdt_sdk(spl_image->fdt_addr);
	bcm_sec_state_to_fdt(spl_image->fdt_addr);
	bcm_sec_delg_pre_launch_fixup((struct spl_image_info*)spl_image);
}
#endif

#if defined(CONFIG_MMC)
#define MMC_DEV_NUM 	0
#if defined(CONFIG_SUPPORT_EMMC_BOOT)
u32 spl_boot_mode(const u32 boot_device)
{
	return MMCSD_MODE_RAW;
}
#endif

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION
static int board_tpl_load_mmc_part(struct blk_desc *bd, disk_partition_t * info, void * buffer)
{
	int ret = 0;
	unsigned long count;
	unsigned long sector = info->start;
	unsigned long num_blocks = info->size;

	/* read image header to find the image size & load address */
	count = blk_dread(bd, sector, num_blocks, buffer);
	printf("%s: read sector %lx, count=%lu\n", __FUNCTION__, sector, count);
	if (count == 0) {
		ret = -EIO;
	}
	return ret;
}

int spl_boot_partition(const u32 boot_device)
{
	int fit_part_id = 0;
	bcaspl_part_info info;

	fit_part_id = get_fit_load_vol_id(&info);

	return fit_part_id;
}
#endif
#endif /* CONFIG_MMC */

static char *imgdev_name = NULL;

char * tpl_get_imgdev_name(void)
{
	return imgdev_name;
}

#if !defined(CONFIG_BCMBCA_IKOS)
void board_boot_order(u32 *spl_boot_list)
{
	int i=0;

#if defined(CONFIG_BCMBCA_VFBIO)
	spl_boot_list[i++] = BOOT_DEVICE_BOARD;
#else
	unsigned long iargs[4];
	char units[4];
	int n;
	n = parse_env_string_plus_nums(find_spl_env_val
				       (tplparams->environment, "IMAGE"), &imgdev_name,
				       4, iargs, units);
	if (n > 0) {
		printf("IMAGE is %s\n", imgdev_name);

#if defined(CONFIG_NAND_BRCMNAND) || defined(CONFIG_MTD_SPI_NAND)
		if (strcmp(imgdev_name,"NAND") == 0) {
			spl_boot_list[i++] = BOOT_DEVICE_BOARD;
		}
#endif

#if defined(CONFIG_MMC)
		if (strcmp(imgdev_name,"EMMC") == 0) {
			spl_boot_list[i++] = BOOT_DEVICE_MMC1;
		}
#endif

#if defined(CONFIG_TPL_SPI_FLASH_SUPPORT)
		if (strcmp(imgdev_name,"SPINOR") == 0) {
			spl_boot_list[i++] = BOOT_DEVICE_NOR;
		}
#endif

	} else {
		printf("IMAGE environment variable not found in env at %p!\n", tplparams->environment);
	}
#endif

	spl_boot_list[i++] = BOOT_DEVICE_NONE;
}
#endif

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
int reserve_mmu(void)
{
	gd->arch.tlb_addr = CONFIG_SYS_PAGETBL_BASE;
	gd->arch.tlb_size = CONFIG_SYS_PAGETBL_SIZE;

	return 0;
}

/*
 * flush and invalid cache before launch u-boot.
 * this function is called by board_init_r after u-boot image loaded.
 */
void spl_board_prepare_for_boot(void)
{
	cleanup_before_linux();
}
#endif

#if defined(CONFIG_SMC_BASED) && !defined(CONFIG_BCMBCA_NO_SMC_BOOT)
uint32_t __attribute__((section(".data"))) *misc_periph_spare = NULL;

static int misc_io_probe(struct udevice *dev)
{
	const fdt32_t *val;

	val = dev_read_prop(dev, "misc-periph-spare", NULL);
        if (val) {
		misc_periph_spare = devm_ioremap(dev, fdt32_to_cpu(val[0]), fdt32_to_cpu(val[1]));
	}

	return misc_periph_spare ? 0 : -1;
}

static const struct udevice_id misc_io_ids[] = {
        { .compatible = "brcm,misc-io" },
        { }
};
U_BOOT_DRIVER(misc_io_drv) = {
        .name = "misc_io",
        .id = UCLASS_NOP,
        .of_match = misc_io_ids,
        .probe = misc_io_probe,
};
#endif

void board_init_f(ulong dummy)
{
#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init();
#endif

#if defined(CONFIG_SYS_ARCH_TIMER)
	timer_init();
#endif

#if !defined(CONFIG_ARM64)
	spl_set_bd();
#endif

	if (spl_early_init())
		hang();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

#if defined(CONFIG_SMC_BASED) && !defined(CONFIG_BCMBCA_NO_SMC_BOOT)
	struct udevice *dev;

	uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(misc_io_drv), &dev);
	tplparams->ddr_size = misc_periph_spare[0];
#endif

	dram_init();

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	reserve_mmu();
	enable_caches();
#endif

	if (tplparams->environment != (void*)(TPL_ENV_ADDR)) {
		printf("Invalid environment address 0x%p! Should be 0x%x.\n",
			tplparams->environment, TPL_ENV_ADDR);
		hang();
	}

#if !defined(CONFIG_BCMBCA_IKOS)
#if defined(CONFIG_BCMBCA_OTP)
	if (bcm_otp_init()) {
		hang();
	}
#endif
#if !defined(CONFIG_SMC_BASED)
	bcm_sec_init();
	bcm_sec_cb_arg_t cb_args[SEC_CTRL_ARG_MAX] = {0};
	cb_args[SEC_CTRL_ARG_KEY].arg[0].ctrl = SEC_CTRL_KEY_GET;
	bcm_sec_do(SEC_SET, cb_args);
#endif
#endif
#if defined(BUILD_TAG)
	printf("$TPL: "BUILD_TAG" $\n");
#endif

#if defined(CONFIG_BCM_BOOTSTATE)
	bca_bootstate_probe();
#endif
}

int dram_init(void)
{
	gd->ram_base = 0x0;
	gd->ram_size = min((u64)PHYS_SDRAM_1_SIZE, ((u64) tplparams->ddr_size << 20));
	gd->bd->bi_dram[0].start = 0x00;
	gd->bd->bi_dram[0].size = gd->ram_size;

#if defined(CONFIG_ARM64)
	/* update memory size in mmu table*/
	mem_map[0].virt = mem_map[0].phys = gd->ram_base;
	mem_map[0].size = gd->ram_size;
#endif
        return 0;
}

__weak void reset_plls(void)
{
}

void spl_board_init(void)
{
#if defined(CONFIG_BCMBCA_ITC_RPC) && !defined(CONFIG_BCMBCA_IKOS)
	rpc_tunnel_init(RPC_TUNNEL_ARM_SMC_NS, true);
	rpc_tunnel_init(RPC_TUNNEL_VFLASH_SMC_NS, true);
	rpc_tunnel_init(RPC_TUNNEL_AVS_SMC_NS, true);
#endif
#if defined(CONFIG_SMC_BASED)
	bcm_sec_init();
	bcm_sec_cb_arg_t cb_args[SEC_CTRL_ARG_MAX] = {0};
	cb_args[SEC_CTRL_ARG_KEY].arg[0].ctrl = SEC_CTRL_KEY_GET;
	bcm_sec_do(SEC_SET, cb_args);
#endif
#ifdef CONFIG_BCMBCA_VFBIO
	vfbio_init();
#endif

#if defined(CONFIG_BCMBCA_PMC)
	bcm_pmc_drv_reg();
	pmc_initmode();
	reset_plls();
#endif
#if defined(CONFIG_TPL_BCMBCA_UBUS4_DCM)
	bcm_ubus_drv_init();
#endif
	boost_cpu_clock();
}

int board_fit_config_name_match(const char *name)
{
#ifdef CONFIG_BCMBCA_BOARD_SPECIFIC_DT
	static char *boardid = NULL;

	if(!boardid && !(tplparams->early_flags & SPL_EA_IGNORE_BOARDID))
		boardid = find_spl_env_val(tplparams->environment, "boardid");

	if(boardid != NULL)
		return strcasecmp(boardid, name);
#else
	return 0;
#endif //#ifdef CONFIG_BCMBCA_BOARD_SPECIFIC_DT
	return 0;
}

struct image_header *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return (struct image_header *)CONFIG_SPL_LOAD_FIT_ADDRESS;
}

ulong tpl_load_read(struct spl_load_info *load, ulong sector,
			       ulong count, void *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n", __func__, sector, count,
	      buf);
	if(buf != (void *)sector)
		memcpy(buf, (void *)sector, count);

	return count;
}

#if !defined(CONFIG_BCMBCA_VFBIO)
__weak int get_raw_metadata(void *buffer, bcaspl_part_info *info, int n)
{
	int r = 0;
	int v;
	int volmap[] = { METADATA_VOL_ID_1, METADATA_VOL_ID_2, -1 };
	v = volmap[n];

	if (v >= 0) {
#if defined(CONFIG_NAND)
		if (strcmp(imgdev_name,"NAND") == 0) {
			struct ubispl_load volume;
			volume.vol_id = v;
			volume.load_addr = buffer;
			if (0 > ubispl_load_volumes(&(info->ubi_info), &volume, 1)) {
				return (-1);
			}
		}
#endif
#if defined(CONFIG_MMC)
		if (strcmp(imgdev_name,"EMMC") == 0) {
			struct blk_desc *bd;
			struct mmc *mmc = find_mmc_device(0);
			if( !mmc ) {
				printf("Error: cannot get mmc device!\n");
				return (-1);
			}
			bd = mmc_get_blk_desc(mmc);
			memset(&(info->gpt_info), 0, sizeof(disk_partition_t));
			if( part_get_info(bd, v, &(info->gpt_info)) ) {
				printf("Error: Invalid gpt partition %d\n", v);
				return (-1);
			} else {
#if defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION)
				if( board_tpl_load_mmc_part(bd, &(info->gpt_info), buffer) < 0) {
					printf("Error: Read from gpt partition %d failed!\n", v);
					return (-1);
				}
#endif
			}
		}
#endif
		r = 1;
	}
	return (r);
}
#endif

#if defined(CONFIG_SMC_BASED) && !defined(CONFIG_BCMBCA_NO_SMC_BOOT)
#define BOOT_PARMS_ACTIVE_IMG_MASK     0x00000010
#define BOOT_PARMS_BOOT_BY_NAME_MASK   0x00000200
#define BOOT_PARMS_IMG_INVERT_MASK     0x00000400
int get_fit_load_vol_id(bcaspl_part_info *dummy)
{
	int fit_vol_id, image;
	
	sdk_ctx.last_reset_reason = 0;
#if defined(CONFIG_BCM_BOOTSTATE)
	if (bcmbca_get_reset_status() & SW_RESET_STATUS) {
		sdk_ctx.last_reset_reason = bcmbca_get_boot_reason();
	}
	bcmbca_set_boot_reason(sdk_ctx.last_reset_reason | BCM_BOOT_PHASE_UBOOT);
	sdk_ctx.last_reset_reason >>= 16;
#endif
	image = (misc_periph_spare[1] & BOOT_PARMS_ACTIVE_IMG_MASK) ? 2:1;
	fit_vol_id = (image == 1) ? IMAGE_VOL_ID_1:IMAGE_VOL_ID_2;
	
	if(misc_periph_spare[1] & BOOT_PARMS_BOOT_BY_NAME_MASK)
	{
		if(misc_periph_spare[1] & BOOT_PARMS_IMG_INVERT_MASK)
			sdk_ctx.active_image =  2;
		else
			sdk_ctx.active_image =  1;
		printf("SELECTED %s image, LUN ID %d\n", (sdk_ctx.active_image == 1) ? "PRIMARY":"UPDATE/BACKUP", fit_vol_id);
	}
	else
	{
		sdk_ctx.active_image = image;
		printf("SELECTED image %d, LUN ID %d\n", sdk_ctx.active_image, fit_vol_id);
	}
	
	return fit_vol_id;
}
#else
#if defined(CONFIG_BCM_BOOTSTATE_FALLBACK_SUPPORT)
/*
Check if the last bootup didn't reach a steady state
It is expected that the Linux userspac sets the reset_reason to BCM_BOOT_REASON_WATCHDOG|BCM_BOOT_PHASE_LINUX_RUN
On manual reboot, the reset reason is set to 0
During upgrade, it is set to BCM_BOOT_REASON_ACTIVATE
*/
static int check_image_fallback_needed(void)
{
	int rc=0;
	int reset_reason=bcmbca_get_boot_reason();

	printf("check_image_fallback_needed? reset_reason [%x] EARLY_FB %x\n", reset_reason, tplparams->early_flags & SPL_EA_IMAGE_FB);
	if(!(tplparams->early_flags & SPL_EA_IMAGE_FB) && bcmbca_get_reset_status() & SW_RESET_STATUS) {
		if (( reset_reason & 0xFFFF) != 0x22ff) //for 63138 and 63148
			if(reset_reason & BCM_BOOT_REASON_WATCHDOG && ((reset_reason & BCM_BOOT_PHASE_MASK) == BCM_BOOT_PHASE_TPL ||
				(reset_reason & BCM_BOOT_PHASE_MASK) == BCM_BOOT_PHASE_UBOOT ||
				(reset_reason & BCM_BOOT_PHASE_MASK) == BCM_BOOT_PHASE_LINUX_START))
				rc=1;
	}
	return rc;
}
#endif


/* FIXME -- break into
 *   get_raw_metatdata()  just fetch metadata copy N into a buffer
 *   validate_metadata()  given buffer, return committed and valid
 *   get_reboot_volatile_flags()
 *   set_reboot_volatile_flags()
 *   get_fit_load_id() -- combine above to return volid or partid
 */
int get_fit_load_vol_id(bcaspl_part_info *info)
{
	int fit_vol_id = IMAGE_VOL_ID_1;
	/* Default image to load is '1' */
	int selected_img_idx = 1;
	int valid[2] = { 0, 0 };
	int seq[2] = { -1, -1 };
	int committed = 1;
	int r;
	int boot_strap_detected = 0;
	volatile int imgmap[ACTIVE_IMGIDX_MAX+1] = { -1, IMAGE_VOL_ID_1, IMAGE_VOL_ID_2 };

	char *cp;
	int v = 0;
	
	while (1 == 1) {
		cp = (char *)spl_get_load_buffer(0, 0);
		r = get_raw_metadata(cp, info, v);
		if (0 == r) {
			/* no more copies */
			printf("no metadata -- using defaults\n");
			boot_strap_detected = 1;
			break;
		} else if (r < 0) {
			/* read error on copy */
			v++;
			continue;
		}
		if (0 == validate_metadata(cp, valid, &committed, seq)) {
			break;
		}
		printf("metadata copy %d crc bad\n", v);
		v++;
	}

	if ((valid[0] != 1 ) && (valid[1] == 2)) {
		/* If only img2 is valid */
		selected_img_idx = 2;
	} else if ((valid[0] == 1) && (valid[1] != 2)) {
		/* If only img1 is valid */
		selected_img_idx = 1;
	} else {
		/* If 2 valid images, check commit flag/sequence numbers */
		if ((valid[0] == 1) && (valid[1] == 2)) {
			if( committed ) {
				/* If there is a committed image */
				selected_img_idx = committed;
#if defined(CONFIG_BCM_BOOTSTATE_FALLBACK_SUPPORT)
				// Did the last commited image failed to reach a steady state??
				if(check_image_fallback_needed())
				{
					fallback_needed=1;
					printf("Last image found to be corrupt, trying fallback to uncommited image\n");
					selected_img_idx = committed == 1 ? 2:1;
				}
#endif
			} else {
				/* If none comitted, pick latest */
				if( seq[0] > -1 || seq[1] > -1 )
					selected_img_idx = (seq[0] > seq[1])? 1:2;
			}
		}
	}

	sdk_ctx.last_reset_reason = 0;
#if defined(CONFIG_BCM_BOOTSTATE)
	r = bcmbca_get_reset_status();
	printf("RESET STATUS is 0x%x\n",r);
	if (r & SW_RESET_STATUS) {
#if defined(CONFIG_BCM63138) || defined(CONFIG_BCM63148)
		if ((bcmbca_get_boot_reason() & 0xFFFF) == 0x22ff)
		{
			printf("old dbcmbca_get_boot_reason(profile 7) %x  \n", bcmbca_get_old_boot_reason());
			if ((bcmbca_get_old_boot_reason() & 0xFFFF) != 0x22ff)
			{
				printf("setting boot reason %x\n", bcmbca_get_old_boot_reason());
				bcmbca_set_boot_reason(bcmbca_get_old_boot_reason());
			}
		}


		/* there's a hardware bug in the 63138 where a power-up boot reports a software reset,
			check for power-up value in register and if so set to zero */
		if ((bcmbca_get_boot_reason() & 0xFFFF) == 0x22ff)
		{
			bcmbca_set_boot_reason(0);
		}
#endif
		sdk_ctx.last_reset_reason = bcmbca_get_boot_reason();
		printf("BOOT REASON is 0x%x\n",sdk_ctx.last_reset_reason);
	}
#if defined(CONFIG_BCM_BOOTSTATE_FALLBACK_SUPPORT)
	if(fallback_needed)
	{
		printf("fallback needed, setting reset reason to BCM_BOOT_REASON_WATCHDOG | BCM_BOOT_PHASE_FB_UBOOT\n"); 
		bcmbca_set_boot_reason((sdk_ctx.last_reset_reason << BCM_RESET_REASON_BITS) | BCM_BOOT_REASON_WATCHDOG | BCM_BOOT_PHASE_FB_UBOOT);
	}
	else
	{

		printf("fallback not needed, setting reset reason to BCM_BOOT_REASON_WATCHDOG | BCM_BOOT_PHASE_UBOOT\n"); 
		bcmbca_set_boot_reason((sdk_ctx.last_reset_reason << BCM_RESET_REASON_BITS) | BCM_BOOT_REASON_WATCHDOG | BCM_BOOT_PHASE_UBOOT);
	}
	bcmbca_set_sel_img_id(selected_img_idx);
#else
	/* reset the boot reason to 0, since this is not used by image that doesn't support FALLBACK
	 *  and for tapioca build , since it doesn't call bcmbca_bootstate_reached_uboot SWBCACPE-54619
         *  which resets the boot reason to 0 when stopped in uboot
	 */
	bcmbca_set_boot_reason((sdk_ctx.last_reset_reason << 16));
#endif
#endif

	/* TODO -- if boot_once is set AND nonselected image is valid, switch selected image AND clear boot once */
	if ( committed && valid[0] && valid[1] && ((sdk_ctx.last_reset_reason & BCM_BOOT_REASON_ACTIVATE) ||  (tplparams->early_flags & SPL_EA_IMAGE_FB)))
	{
		selected_img_idx = 3 - committed;
	}

	/* now look up volume ID for the selected image */
	fit_vol_id = imgmap[selected_img_idx];
	if( boot_strap_detected ) {
		printf("SELECTED BOOTSTRAP Image FIT_VOL_ID is %d\n", fit_vol_id);
		sdk_ctx.active_image = ACTIVE_IMGIDX_BOOTSTRAP;
	} else {
		printf("SELECTED Image %d FIT_VOL_ID is %d\n",selected_img_idx, fit_vol_id);
		sdk_ctx.active_image = selected_img_idx;
	}
	return fit_vol_id;
}
#endif


#if !defined(CONFIG_BCMBCA_IKOS) 
/* This function is called after entire FIT header has been read into memory */
void board_spl_fit_pre_load(struct spl_image_info * image_info,
	struct spl_load_info *load_info, void * fit, ulong start_sector, ulong sector_count)
{
 	uint64_t loaded_size = (uint64_t)sector_count * load_info->bl_len;
	uint64_t required_size = bcm_sec_get_reqd_load_size(fit);

	/* In the case where entire FIT image may not have been loaded to memory
	 * at once, we may have a case where we havent loaded enough blocks to get
	 * the FIT signature */
	if( loaded_size  < required_size ) {
		/* Read required size */
		sector_count = load_info->read(load_info, start_sector,
			(required_size/load_info->bl_len) + (required_size%load_info->bl_len?1:0), fit);
		debug("fit read sector %lx, sectors=%lld, dst=%p, count=%lu, size=0x%llx\n",
	      		start_sector,
			(required_size/load_info->bl_len) + (required_size%load_info->bl_len?1:0),
			fit, sector_count, required_size);
		loaded_size = (uint64_t)sector_count * load_info->bl_len;
	}

	/* Authenticate FIT header and set security settings */
	bcm_sec_validate_fit(fit, loaded_size);
	bcm_sec_fit(fit);
}
#endif

#if defined(CONFIG_BCMBCA_VFBIO)
__weak int tpl_vflash_load_image(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev)
{
	int ret, id;
	void *fit = spl_get_load_buffer(0, 0);
	char *value = find_spl_env_val(tplparams->environment, "vf_1_boot");

	if (value) {
		const char *conf;
		int nodeoffset;
		uint blk_size;
		ulong size=UINT_MAX;
		char *name;
		void *bootfs;

		printf("Initial boot, try to boot uboot from initial boot LUN...\n");

		ret = vfbio_lun_get_id("image-pack", &id);
		if (ret)
		{
			printf("Could not find initial boot package\n");
			return -1;
		}

		if (value) {
			size = strtoul(value, NULL, 10);
			if (vfbio_lun_get_blk_size(id, &blk_size))
				size = UINT_MAX;
			else
				size = (size + blk_size -1)/blk_size;
		}

		if (size == UINT_MAX) {
			printf("Initial bundle LUN size was not supplied in 'vf_1_boot' env var, will try reading entire LUN...\n");
		} else {
			printf("Reading initial bundle LUN %lu blocks...\n", size);
		}
		ret = vfbio_lun_read(id, 0, size, fit);
		if (ret)
		{
			printf("Could not read initial boot package\n");
			return -1;
		}

		nodeoffset = fdt_path_offset(fit, "/configurations");
		if (nodeoffset < 0) {
			printf("/configurations node not found in bundle!\n");
			return -1;
		}

		conf = fdt_getprop(fit, nodeoffset, "default", (int *)&size);
		if (!conf) {
			printf("default configuration not found in bundle!\n");
			return -1;
		}

		ret = get_binary_from_bundle((ulong)fit, conf, "bootfs", &name, (ulong *)&bootfs, &size);
		if (ret) {
			return -1;
		}
		memcpy(fit, bootfs, size);
	}
	else {
		id = get_fit_load_vol_id(NULL);
		ret = vfbio_lun_read(id, 0, UINT_MAX, fit);
	}

	if ((ret == 0) && (image_get_magic(fit) == FDT_MAGIC)) {
		struct spl_load_info load;
		debug("Found FIT format U-Boot\n");

		memset(&load, '\0', sizeof(struct spl_load_info));
		load.bl_len = 1;
		load.read = tpl_load_read;
		ret = spl_load_simple_fit(spl_image, &load, (long)fit, fit);
	}

	return ret;
}

SPL_LOAD_IMAGE_METHOD("VFLASH", 0, BOOT_DEVICE_BOARD, tpl_vflash_load_image);
#elif defined(CONFIG_NAND)
static int log2exact(uint32_t num)
{
	int r = 0;
	while (num > 1) {
		r++;
		num = num >> 1;
	}
	return(r);
}

// str to int
static int is_digit(char c)
{
	if(c >= '0' && c <= '9')
		return 1;
	return 0;
}
static uint64_t atol(const char *s)
{
	uint64_t i = 0;

	while(is_digit(*s)){
	i = i * 10 + *(s++) - '0';
	}

	return(i);
}
/*
	brcmnand.0:2097152(loader),102760448@2097152(image)
	returns image_start=2097152 image_end=104857600
*/

static int parse_mtdparts(char *ptr, uint64_t *image_start, uint64_t *image_end)
{
char *tmp_str, *start, *end;
int rc=0;

	tmp_str=ptr;
	if(tmp_str != NULL)
	{
		if((tmp_str=strstr(ptr, "(image)")) != NULL)
		{
			end=tmp_str;
			while(tmp_str[0] != '@' && tmp_str >= ptr)
			{
				tmp_str--;
			}
			if(tmp_str[0] == '@')
			{
				start=tmp_str;
				while(tmp_str[0] != ',' && tmp_str >= ptr)
				{
					tmp_str--;
				}
				if(tmp_str[0] == ',')
				{
					tmp_str++;
					start[0]='\0';
					*image_end=atol(tmp_str);
					end[0]='\0';
					*image_start=atol(start+1);
					*image_end+=*image_start;
					rc=1;
				}

			}
		}
	}
return rc;

}

int tpl_get_ubi_info( bcaspl_part_info * part_info)
{
	int ret = -1;
	int page_size;
	uint64_t image_start = 1024 * 1024;
	uint64_t image_end = (uint64_t)(-1);
	uint64_t totalsize;
	int log2_pebsize;
	char *name;
	unsigned long iargs[4];
	char units[4];
	int n;

	ret=parse_mtdparts(find_spl_env_val(tplparams->environment, "mtdparts"), &image_start, &image_end);

	if(!ret)
	{
		image_start = 1024 * 1024;
		image_end = (uint64_t)(-1);
		/* For NAND devices, determine image_start and image_end */
		n = parse_env_string_plus_nums(find_spl_env_val
					       (tplparams->environment, "IMAGE"), &name,
					       4, iargs, units);
		if (n > 0) {
			if (n > 1) {
				image_start =
				    ((uint64_t) iargs[0]) << suffix2shift(units[0]);

			}
			if (n > 2) {
				image_end =
				    (uint64_t) iargs[1] << suffix2shift(units[1]);
			}
		}
		free(name);
	}

	part_info->ubi_info.read = nand_spl_read_block;
	part_info->ubi_info.peb_size = nand_spl_get_blk_size();
	log2_pebsize = log2exact(part_info->ubi_info.peb_size);
	page_size = nand_spl_get_page_size();

	part_info->ubi_info.ubi = (struct ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	part_info->ubi_info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	part_info->ubi_info.peb_offset = image_start >> log2_pebsize;
	/* vid_offset and leb_start of page_size and 2*page_size respectively are dictated by ubinize with no subpage writes required */
	part_info->ubi_info.vid_offset = page_size;
	part_info->ubi_info.leb_start = 2 * page_size ;
	totalsize = nand_spl_get_total_size();
	if (image_end == (uint64_t) (-1)) {
		image_end = totalsize;
	} else if (image_end > totalsize) {
		image_end = totalsize - 8 * part_info->ubi_info.peb_size;
		printf("INFO: IMAGE specified beyond flash size... using flash up to the end for image\n");
	}
	part_info->ubi_info.peb_count = (image_end >> log2_pebsize) - part_info->ubi_info.peb_offset;
	debug("INFO: image from %lld to %lld\n",image_start, image_end); 

	return 0;
}

__weak int tpl_load_image(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev)
{
	int ret = -1;
	bcaspl_part_info info;
	struct ubispl_load volume;
	void *fit = spl_get_load_buffer(0, 0);

#if defined(CONFIG_BCM_BOOTSTATE_FALLBACK_SUPPORT)
	const char *fallback_ptr=NULL;
#endif

#ifdef CONFIG_SPL_NAND_SUPPORT
	nand_init();
#endif
	
	/* Get current ubi info */
	tpl_get_ubi_info(&info);
	volume.vol_id = get_fit_load_vol_id(&info);
	volume.load_addr = fit;

	ret = ubispl_load_volumes(&(info.ubi_info), &volume, 1);

	if (!ret && (image_get_magic(fit) == FDT_MAGIC)) {
		struct spl_load_info load;
		debug("Found FIT format U-Boot\n");

		memset(&load, '\0', sizeof(struct spl_load_info));
		load.bl_len = 1;
		load.read = tpl_load_read;
		ret = spl_load_simple_fit(spl_image, &load, (long)fit, fit);
#if defined(CONFIG_BCM_BOOTSTATE_FALLBACK_SUPPORT)
		if(ret != 0)
		{
			if(!fallback_needed)
			{
				printf("Loading U-boot failed, setting reset reason to BCM_BOOT_REASON_WATCHDOG | BCM_BOOT_PHASE_TPL\n");  
				bcmbca_set_boot_reason((sdk_ctx.last_reset_reason << BCM_RESET_REASON_BITS) | BCM_BOOT_REASON_WATCHDOG | BCM_BOOT_PHASE_TPL);
			}
			else
			{
				printf("Loading fallback u-boot failed, current reset reason %x\n", bcmbca_get_boot_reason());
			}
		}
		else
		{

				fallback_ptr=fdt_getprop(fit, 0, "support_fallback", NULL);
				if(fallback_ptr == NULL)
				{
					if(fallback_needed)
					{
						printf("Image doesn't support fallback but we are attempting to un-comitted image\n");
						printf("Not resetting the boot reason to 0\n");
					}
					else
					{
						printf("active image doesn't support fallback, resetting reset_reason\n");
						bcmbca_set_boot_reason((sdk_ctx.last_reset_reason << BCM_RESET_REASON_BITS));
					}
				}
		}
#endif
	}

#ifdef CONFIG_SPL_NAND_SUPPORT
	nand_deselect();
#endif

	return ret;
}

SPL_LOAD_IMAGE_METHOD("NAND", 0, BOOT_DEVICE_BOARD, tpl_load_image);
#endif /* CONFIG_NAND */
#if defined(CONFIG_TPL_SPI_FLASH_SUPPORT)
static ulong spl_spi_fit_read(struct spl_load_info *load, ulong sector,
			      ulong count, void *buf)
{
	ulong ret;
	size_t retlen;
	struct mtd_info *mtd = load->dev;

	ret = mtd_read(mtd, sector, count, &retlen, buf);
	if (!ret)
		return retlen;
	else
		return 0;
}

static int tpl_spinor_load_image(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev)
{
	int ret = -1;
	char *name;
	int n;
	unsigned long iargs[4];
	unsigned long image_start = 0;
	int use_spinor = 0;
	char units[4];
	struct udevice *dev;
	struct mtd_info *mtd = NULL;
	size_t retlen;
	void *fit = spl_get_load_buffer(0, 0);

	n = parse_env_string_plus_nums(find_spl_env_val
				       (tplparams->environment, "IMAGE"), &name,
				       4, iargs, units);
	if (n > 0) {
		debug("IMAGE is %s\n",name);
		if (strcmp(name,"SPINOR") == 0) {
			use_spinor = 1;
			if (n > 1) {
				image_start = iargs[0]<< suffix2shift(units[0]);

			}
		}
	}
	free(name);
	if (!use_spinor){
		debug("Lack of SPINOR image env setting.\n");
		goto done;
	}

	ret = uclass_get_device_by_driver(UCLASS_SPI_FLASH, DM_GET_DRIVER(spi_flash_std), &dev);
	if (ret){
		debug("SPI NOR failed to initialize. (error %d)\n", ret);
		goto done;
	}
	mtd = get_mtd_device_nm(SPIFLASH_MTDNAME);
	if (IS_ERR_OR_NULL(mtd)){
		debug("%s:MTD device %s not found, ret %ld\n",__func__, SPIFLASH_MTDNAME,
		   PTR_ERR(mtd));
		ret = -1;
		goto done;
	}

	/* Load u-boot, mkimage header is 64 bytes. */
	ret = mtd_read(mtd, image_start, sizeof(struct image_header), &retlen,fit);
	if (ret) {
		debug("%s: Failed to read from SPI flash (err=%d)\n", __func__, ret);
		goto done;
	}
	if (image_get_magic(fit) == FDT_MAGIC) {
		struct spl_load_info load;

		debug("Found FIT format U-Boot\n");
		load.dev = mtd;
		load.priv = NULL;
		load.filename = NULL;
		load.bl_len = 1;
		load.read = spl_spi_fit_read;
		debug("spl_load_simple_fit from %lu\n",image_start);
		ret = spl_load_simple_fit(spl_image, &load, image_start, fit);

		//spi nor, hard code active_image  to 1
		sdk_ctx.active_image = 1;
	}
done:
	if (!IS_ERR_OR_NULL(mtd))
		put_mtd_device(mtd);
	return ret;
}

SPL_LOAD_IMAGE_METHOD("SPI", 0, BOOT_DEVICE_NOR, tpl_spinor_load_image);
#endif

