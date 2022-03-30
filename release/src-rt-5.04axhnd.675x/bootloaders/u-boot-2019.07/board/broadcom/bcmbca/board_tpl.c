/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */
#define DEBUG
#include <common.h>
#include <mmc.h>
#include <spl.h>
#include <nand.h>
#if defined(CONFIG_ARM64)
#include <asm/armv8/mmu.h>
#endif

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

DECLARE_GLOBAL_DATA_PTR;

tpl_params __attribute__((section(".data"))) * tplparams = NULL;

static struct bcasdk_ctx sdk_ctx = {};

static void update_uboot_fdt_sdk(void *fdt_addr);
void save_boot_params_ret(void);
static int get_fit_load_vol_id(void *info);
static void update_uboot_fdt_sdk(void *fdt_addr)
{
 	int offset;
	
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
	}
}

__weak void boost_cpu_clock(void)
{

}

/* called from start.S. No stack use. */
void save_boot_params(void* params)
{
	tplparams = (tpl_params*)params;
	save_boot_params_ret();
}

void board_spl_fit_post_load(ulong load_addr, size_t length) 
{
	bcm_sec_delg_cfg * delg_cfg = NULL;
	int ret = -1;

	/* Get delegation status */
	delg_cfg = bcm_sec_get_delg_cfg();

	/* Check if loaded images violate any sw restrictions in security policy */
	if( delg_cfg && delg_cfg->delg_id ) {
		ret = bcm_sec_delg_post_process_sw_restrictions( load_addr );
		if( ret ) {
			printf("ERROR: Post FIT load sw restrictions check failed!\n");
			bcm_sec_abort();
		}
	}
}


#if !defined(CONFIG_BCMBCA_IKOS)
void spl_perform_fixups(struct spl_image_info *spl_image)
{
	bcm_sec_delg_cfg * delg_cfg = NULL;

	update_uboot_fdt(spl_image->fdt_addr, tplparams);
	update_uboot_fdt_sdk(spl_image->fdt_addr);
	bcm_sec_ctrl_arg_t arg = {.ctrl = SEC_CTRL_KEY_CHAIN_RSA, 
				.ctrl_arg = spl_image->fdt_addr} ;
	bcm_sec_update_ctrl_arg(&arg, SEC_CTRL_ARG_KEY);

	/* Get delegation status */
	delg_cfg = bcm_sec_get_delg_cfg();

	/* Check if delegation is active inorder to pass encoded keys */
	if( delg_cfg && delg_cfg->delg_id ) {
		arg.ctrl = SEC_CTRL_KEY_CHAIN_ENCKEY;
	} else {
		arg.ctrl = SEC_CTRL_KEY_CHAIN_AES;
	}
	bcm_sec_update_ctrl_arg(&arg, SEC_CTRL_ARG_KEY);

	/* Check if delegation is active to pass export items */
	if( delg_cfg && delg_cfg->delg_id ) {
		arg.ctrl = SEC_CTRL_KEY_EXPORT_ITEM;
		bcm_sec_update_ctrl_arg(&arg, SEC_CTRL_ARG_KEY);
	}
	bcm_sec_do(SEC_SET, NULL);

	/* Compare entrypoint to security policy sw restrictions entrypoint */
	if( delg_cfg && delg_cfg->delg_id ) {
		if( delg_cfg->post_loader_entry_point ) {
			if( spl_image->entry_point != delg_cfg->post_loader_entry_point ) {
				printf("ERROR: Invalid entrypoint:0x%lx! reqd entrypoint from sec policy:0x%08x\n",
					spl_image->entry_point, delg_cfg->post_loader_entry_point);
				bcm_sec_abort();
			} else {
				printf("INFO: Post loader entry point 0x%08x is valid!\n", spl_image->entry_point);
			}
		}
	}
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
	disk_partition_t info;
	
	fit_part_id = get_fit_load_vol_id((void*)&info);

	return fit_part_id;
}
#endif
#endif /* CONFIG_MMC */

char *imgdev_name = NULL;
#if !defined(CONFIG_BCMBCA_IKOS)
void board_boot_order(u32 *spl_boot_list)
{
	int i=0;

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

	if (bcm_otp_init()) {
		hang();
	}
	bcm_sec_init();
	bcm_sec_cb_arg_t cb_args[SEC_CTRL_ARG_MAX] = {0};
	cb_args[SEC_CTRL_ARG_KEY].arg[0].ctrl = SEC_CTRL_KEY_GET;
	bcm_sec_do(SEC_SET, cb_args);
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
	gd->ram_size = min(PHYS_SDRAM_1_SIZE, ((u64) tplparams->ddr_size << 20));
	gd->bd->bi_dram[0].start = 0x00;
	gd->bd->bi_dram[0].size = gd->ram_size;

#if defined(CONFIG_ARM64)
	/* update memory size in mmu table*/
	mem_map[0].virt = mem_map[0].phys = gd->ram_base;
	mem_map[0].size = gd->ram_size;
#endif
        return 0;
}

void spl_board_init(void)
{
#if defined(CONFIG_BCMBCA_PMC)
	pmc_initmode();
#endif
#if !defined(CONFIG_BCMBCA_IKOS)
	boost_cpu_clock();
#endif
}

int board_fit_config_name_match(const char *name)
{
#ifdef CONFIG_BCMBCA_BOARD_SPECIFIC_DT
	static char *boardid = NULL;
	
	if(!boardid && !(tplparams->early_flags & SPL_EA_IGNORE_BOARDID))
		boardid = find_spl_env_val(tplparams->environment, "boardid");
	
	return strcasecmp(boardid, name);
#else
	return 0;
#endif //#ifdef CONFIG_BCMBCA_BOARD_SPECIFIC_DT
}

struct image_header *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return (struct image_header *)CONFIG_SPL_LOAD_FIT_ADDRESS;
}

#ifdef CONFIG_NAND
static ulong tpl_load_read(struct spl_load_info *load, ulong sector,
			       ulong count, void *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n", __func__, sector, count,
	      buf);
	if(buf != (void *)sector)
		memcpy(buf, (void *)sector, count);

	return count;
}

// TBD
struct metadata_s {
};

__weak int get_raw_metadata(void *buffer, struct ubispl_info *info, int n)
{
	int r = 0;
	int v;
#if defined(CONFIG_MMC)
	struct mmc *mmc; 
	struct blk_desc *bd;
#endif	
	struct ubispl_load volume;
	int volmap[] = { METADATA_VOL_ID_1, METADATA_VOL_ID_2, -1 };
	v = volmap[n];
	if (v >= 0) {
		if (strcmp(imgdev_name,"NAND") == 0) {  
			volume.vol_id = v;
			volume.load_addr = buffer;
			if (0 > ubispl_load_volumes((struct ubispl_info *)info, &volume, 1)) {
				return (-1);
			}
#if defined(CONFIG_MMC)
		} else if (strcmp(imgdev_name,"EMMC") == 0) {
			mmc = find_mmc_device(0);
			bd = mmc_get_blk_desc(mmc);
			memset(info, 0, sizeof(disk_partition_t));
			if( part_get_info(bd, v, (disk_partition_t *)info) ) {
				printf("Error: Invalid gpt partition %d\n", v);
				return (-1);
			} else {
#if defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION)
				if( board_tpl_load_mmc_part(bd, info, buffer) < 0) {
					printf("Error: Read from gpt partition %d failed!\n", v); 
					return (-1);
				}
#endif
			}
#endif			
		}
		r = 1;
	}
	return (r);
}

/* FIXME -- break into 
 *   get_raw_metatdata()  just fetch metadata copy N into a buffer 
 *   validate_metadata()  given buffer, return committed and valid
 *   get_reboot_volatile_flags()
 *   set_reboot_volatile_flags()
 *   get_fit_load_id() -- combine above to return volid or partid
 */
static int get_fit_load_vol_id(void *info)
//static int get_fit_load_vol_id(struct ubispl_info *info)
{
	int fit_vol_id = IMAGE_VOL_ID_1;
	/* Default image to load is '1' */
	int selected_img_idx = 1; 
	char *cp;
	int valid[2] = { 0, 0 };
	int seq[2] = { -1, -1 };
	int committed = 1;
	int r;
	int v = 0;
	int boot_strap_detected = 0;
	volatile int imgmap[ACTIVE_IMGIDX_MAX+1] = { -1, IMAGE_VOL_ID_1, IMAGE_VOL_ID_2 };
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
#if defined(CONFIG_BCM63138)
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
	bcmbca_set_boot_reason((sdk_ctx.last_reset_reason << 16) | BCM_BOOT_REASON_WATCHDOG | BCM_BOOT_PHASE_UBOOT);
#endif

	/* TODO -- if boot_once is set AND nonselected image is valid, switch selected image AND clear boot once */
	if (valid[0] && valid[1] && ((sdk_ctx.last_reset_reason & BCM_BOOT_REASON_ACTIVATE) ||  (tplparams->early_flags & SPL_EA_IMAGE_FB)))
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

static int log2exact(uint32_t num)
{
	int r = 0;
	while (num > 1) {
		r++;
		num = num >> 1;
	}
	return(r);
}

#if !defined(CONFIG_BCMBCA_IKOS)
/* This function is called after entire FIT header has been read into memory */
void board_spl_fit_pre_load(struct spl_image_info * image_info, 
	struct spl_load_info *load_info, void * fit, ulong start_sector, ulong sector_count)
{
 	uint64_t loaded_size = sector_count * load_info->bl_len;
	uint64_t required_size = bcm_sec_get_reqd_load_size(fit);

	/* In the case where entire FIT image may not have been loaded to memory
	 * at once, we may have a case where we havent loaded enough blocks to get 
	 * the FIT signature */
	if( loaded_size  < required_size ) {
		/* Read required size */
		sector_count = load_info->read(load_info, start_sector, 
			(required_size/load_info->bl_len) + (required_size%load_info->bl_len?1:0), fit);
		debug("fit read sector %lx, sectors=%d, dst=%p, count=%lu, size=0x%lx\n",
	      		start_sector, 
			(required_size/load_info->bl_len) + (required_size%load_info->bl_len?1:0),
			fit, sector_count, required_size);
		loaded_size = sector_count * load_info->bl_len;
	}

	/* Authenticate FIT header and set security settings */
	bcm_sec_validate_fit(fit, loaded_size);
	bcm_sec_fit(fit);
}
#endif

__weak int tpl_load_image(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev)
{
	int ret = -1;
	struct ubispl_info info;
	struct ubispl_load volume;
	int page_size; 
	void *fit = spl_get_load_buffer(0, 0);
	uint64_t image_start = 1024 * 1024;
	uint64_t image_end = (uint64_t)(-1);
	uint64_t totalsize;
	int log2_pebsize;
	char *name;
	unsigned long iargs[4];
	char units[4];
	int n;

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

#ifdef CONFIG_SPL_NAND_SUPPORT
	nand_init();
#endif

	info.read = nand_spl_read_block;
	info.peb_size = nand_spl_get_blk_size();
	log2_pebsize = log2exact(info.peb_size);
	page_size = nand_spl_get_page_size();

	info.ubi = (struct ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_offset = image_start >> log2_pebsize;
	/* vid_offset and leb_start of page_size and 2*page_size respectively are dictated by ubinize with no subpage writes required */
	info.vid_offset = page_size;
	info.leb_start = 2 * page_size ;
	totalsize = nand_spl_get_total_size();
	if (image_end == (uint64_t) (-1)) {
		image_end = totalsize;
	} else if (image_end > totalsize) {
		image_end = totalsize - 8 * info.peb_size;
		printf("INFO: IMAGE specified beyond flash size... using flash up to the end for image\n");
	}
	info.peb_count = (image_end >> log2_pebsize) - info.peb_offset;
	debug("image from %lld to %lld\n",image_start, image_end); 
	
	volume.vol_id = get_fit_load_vol_id(&info);
	volume.load_addr = fit;

	ret = ubispl_load_volumes(&info, &volume, 1);
	  
	if (!ret && (image_get_magic(fit) == FDT_MAGIC)) {
		struct spl_load_info load;
		debug("Found FIT format U-Boot\n");

		load.bl_len = 1;
		load.read = tpl_load_read;
		ret = spl_load_simple_fit(spl_image, &load, (long)fit, fit);
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
	unsigned long image_start;
	int use_spinor = 0;
	char units[4];
	struct udevice *dev;
	struct mtd_info *mtd;
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
		debug("spl_load_simple_fit from %d\n",image_start);
		ret = spl_load_simple_fit(spl_image, &load, image_start, fit);

		//spi nor, hard code active_image  to 1		
		sdk_ctx.active_image = 1;
	}
done:
	put_mtd_device(mtd);
	return ret;
}

SPL_LOAD_IMAGE_METHOD("SPI", 0, BOOT_DEVICE_NOR, tpl_spinor_load_image);
#endif

