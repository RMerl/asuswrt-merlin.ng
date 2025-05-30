// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019
 * Broadcom Corp
 */

#include <stdlib.h>
#include <command.h>
#include <common.h>
#include <environment.h>
#include <hexdump.h>
#include <ubi_uboot.h>
#include <cli.h>
#include <console.h>
#if defined(CONFIG_DM_RNG)	
#include <rng.h>
#endif
#include "bca_common.h"
#include "bcm_bootstate.h"
#include "bcm_thermal.h"
#include "bca_sdk.h"
#if defined(CONFIG_BCMBCA_OTP)
#include "bcm_otp.h"
#endif
#include "bcm_secure.h"
#include "httpd/bcmbca_net.h"
#include "clk_rst.h"
#include <nand.h>
#include <mmc.h>
#if defined(CONFIG_WDT)
#include <wdt.h>
#endif
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#endif
#ifdef CONFIG_BCMBCA_VFBIO
#include "vfbio.h"
#include "ba_svc.h"
#else
#include "spl_env.h"
#endif

#define SIZE_OVERHEAD		0x100000UL	//1M
#define BOOTFS_SIZE_BYTES	0x00A00000UL	//10M
#define ROOTFS_SIZE_BYTES	0x02800000UL	//40M
#define SQUASHFS_MAGIC		0x73717368
#define UBIFS_MAGIC		0x06101831
#define MIN_IMG_INDEX		1
#define MAX_IMG_INDEX		2

#define IMG_STAT_INVALID	0	//Mark image as invalid
#define IMG_STAT_VALID		1	//Mark image as valid
#define IMG_STAT_UNCOMMIT	-1	//Uncommit image
#define IMG_STAT_NOCOMMIT	0	//Leave commit status unchanged
#define IMG_STAT_COMMIT		1	//Commit image

#define atoi(s) simple_strtol(s, NULL, 0)

DECLARE_GLOBAL_DATA_PTR;

/* Flag indicating whether we want to force an update and disregard any compatibility checks */
static int forced_updates = 0;
/* Flag to prevent synching of runtime env to loader bin when flashing a new loader */
static int disable_runtime_env_sync = 0;

typedef int (*flash_fn)( ulong addr, ulong size , int img_index);

void mtd_probe_devices(void);

typedef struct {
	char name[20]; /* Name of upgrade bundle component */
	flash_fn func; /* Function to flash this upgrade bundle component */
} flashfn_table_entry;

/* Enable CLI commands based on included CMD modules */
#if defined(CONFIG_CMD_GPT) && defined(CONFIG_CMD_PART) && defined(CONFIG_CMD_MMC) && defined(CONFIG_MMC)
#define BCA_SDK_EMMC_CMD	1
#endif

#if defined(CONFIG_CMD_UBI) && defined(CONFIG_CMD_MTD) && defined(CONFIG_NAND)
#define BCA_SDK_NAND_CMD	1
#endif

#if defined(CONFIG_CMD_SF) && defined(CONFIG_SPI_FLASH) && defined(CONFIG_DM_SPI_FLASH) && defined(CONFIG_CMD_MTD)
#define BCA_SDK_SPINOR_CMD	1
#endif

#if defined(BCA_SDK_SPINOR_CMD)
static int spinor_load_bootfs(uint32_t bootfs_load_addr );
static int spinor_restoredefault(void);
static int write_spinor_partition( char *const partitionname, ulong addr, ulong size );
static int do_flash_spinor_binary(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
static int do_flash_spinor_bootfs_rootfs(cmd_tbl_t * cmdtp, int flag, int argc,	char *const argv[]);
static int flash_loader_spinor( ulong addr, ulong size);
#endif
#ifdef BCA_SDK_EMMC_CMD
/* EMMC specific routines */
static int flash_rootfs_emmc( ulong addr, ulong size, int img_index );
static int flash_bootfs_emmc( ulong addr, ulong size, int img_index );
static int flash_loader_emmc( ulong addr, ulong size, int img_index );
static int set_emmc_metadata( char* metadata, int size );
static int get_emmc_metadata( char* metadata, int size );
static int emmc_load_bootfs( int img_index, uint32_t bootfs_load_addr );
static int emmc_rdwr_userdata_part( char * part_name, ulong addr, ulong size, int write );
static int emmc_rdwr_boot_part( ulong addr, ulong size, int img_index, int write );
static int emmc_restoredefault(void);
static int emmc_do_gpt_fixup(void);
#endif /* BCA_SDK_EMMC_CMD */

#ifdef BCA_SDK_NAND_CMD
/* NAND Specific routines */
static int flash_rootfs_nand( ulong addr, ulong size, int img_index );
static int flash_bootfs_nand( ulong addr, ulong size, int img_index );
static int flash_loader_nand( ulong addr, ulong size, int img_index );
static int set_nand_metadata( char* metadata, int size );
static int get_nand_metadata( char* metadata, int size );
static int nand_load_fit( int img_index, uint32_t fit_load_addr );
static int nand_load_bootfs( int img_index, uint32_t bootfs_load_addr );
static int nand_restoredefault(void);
#endif /* BCA_SDK_NAND_CMD */

/* Generic routines */
#if defined(BCA_SDK_EMMC_CMD) || defined(BCA_SDK_NAND_CMD) || defined(BCA_SDK_SPINOR_CMD) || defined(CONFIG_BCMBCA_VFBIO)
static unsigned int bcm_handle_mapper(void* fit, char *flash_device, char *flash_opts);
#endif
#if defined(BCA_SDK_EMMC_CMD) || defined(BCA_SDK_NAND_CMD)
static int do_flash_bins(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
static int do_metadata(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
static int do_flash_bootfs_rootfs( char * bootfs_filename, char * rootfs_filename, int image_index );
#endif
static int set_img_status( int img_index, int valid_status, int commit_status );
static int verify_compat_string( const char * compat_str );
static int do_boot(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
static int do_load(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
static int load_linux_img(int flag, int argc, char *const argv[]);
static int do_restoredefault(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
#if defined(CONFIG_BCMBCA_VFBIO)
static int flash_smcbl( ulong addr, ulong size, int img_index );
static int flash_smcos( ulong addr, ulong size, int img_index );
static int flash_armbl( ulong addr, ulong size, int img_index );
static int flash_meminit( ulong addr, ulong size, int img_index );
static int flash_ubootenv( ulong addr, ulong size, int img_index );
#else
static char forced_image_media[256] = {0};
static char forced_boot_media[256] = {0};
static int sync_update_loader_bin_env(ulong loader_addr);
static int update_flash_parts_from_loader_bin( ulong loader_addr, ulong loader_size );
static int do_force(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
static int do_flash_loader(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
static int flash_loader( ulong addr, ulong size , int img_index);
char * get_loader_media(void);
static char * get_image_media(void);
static int strcasecmp_image_media(const char *s);
#endif
static int flash_rootfs( ulong addr, ulong size , int img_index);
static int flash_bootfs( ulong addr, ulong size , int img_index);
static int do_flash_upgrade_img (cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
static int set_metadata_val( int * committed, int * valid, int * seq );
static int get_metadata_val( int * committed, int * valid, int * seq );
#if defined(CONFIG_BCM_BOOTSTATE)
static int do_activate(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
#if defined(CONFIG_BCM_BOOTSTATE_FALLBACK_SUPPORT)
static int do_check_fallback(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
#endif
#endif
static int get_active_img_idx(void);
static int set_active_img_idx( int img_idx);
#ifdef CONFIG_BCMBCA_HTTPD
static int do_httpd_start(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
#endif
#ifdef CONFIG_BCMBCA_XRDP_ETH
static int do_eth_active_port(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
#endif

int board_init_flash_parts(int erase_img_part);
uint32_t env_boot_magic_search_size(void);

/* Local Variables */
static flashfn_table_entry fn_table[] =  {
	{ "bootfs", flash_bootfs },
	{ "rootfs", flash_rootfs },
#ifdef CONFIG_BCMBCA_VFBIO
	{ "smcos", flash_smcos },
	{ "armbl", flash_armbl },
	{ "meminit", flash_meminit },
	{ "ubootenv", flash_ubootenv },
	{ "smcbl", flash_smcbl },
#else
	{ "loader", flash_loader },
#endif
};

#ifdef CONFIG_BCMBCA_VFBIO
static int flash_ubootenv( ulong addr, ulong size, int img_index )
{
	if (!addr)
		return CMD_RET_USAGE;

	if(!(gd->flags & GD_FLG_ENV_DEFAULT) && (env_get_yesno("update_env") != 1))
	{
		printf("Skipped updating uboot env. Set update_env=y to force update if needed\n");
		return -1;
	}

	if(size != CONFIG_ENV_SIZE)
	{
		printf("Environment size in package %lx is different from max env size %x\n", size, CONFIG_ENV_SIZE);
		return -1;
	}

	if(env_import((char *)addr, 1))
		return -1;

	env_save();

	return 0;
}

static int flash_smcbl( ulong addr, ulong size, int img_index )
{
	// SMCBL should always be flashed into the 1st bank. SMC takes care of copying into the 2nd bank in a safe manner
	char *name = "smcbl1";
	uint blk_size = 0;
	uint64_t lun_size = 0;
	int id, ret;

	if (!addr || !size)
		return CMD_RET_USAGE;

	ret = vfbio_lun_get_id(name, &id);
	if(ret < 0)
	{
		printf("%s lun was not found on flash\n", name);
		return -1;
	}

	vfbio_lun_get_size(id, &lun_size);
	if(size > lun_size)
	{
		printf("Not enough space to burn %s image - available %llu, needed %lu\n", name, lun_size, size);
		return -1;
	}
	vfbio_lun_get_blk_size(id, &blk_size);
	// TBD: add marking invalid mechanism before write and valid after
	ret = vfbio_lun_write(id, 0, (size + blk_size - 1)/blk_size, (void *)addr);
	#if defined(CONFIG_WDT)
		/* Force reset the watchdog after flash write */
		if (gd->watchdog_dev)
			wdt_reset(gd->watchdog_dev);
	#endif

	return ret;
}

static int flash_lun_vfbio( ulong addr, ulong size, int img_index, int id, char *name_base, uint32_t flags)
{
	int ret;
	uint blk_size = 0;
	uint64_t lun_size = 0;
	uint64_t total_size = 0, free_size = 0;
	char name[128];
	int prev_id;

	if (!addr || !size || (img_index < MIN_IMG_INDEX) || (img_index > MAX_IMG_INDEX))
		return CMD_RET_USAGE;

	sprintf(name, "%s%d", name_base, img_index);

	vfbio_device_get_info(&total_size, &free_size);

	ret = vfbio_lun_get_id(name, &prev_id);
	if ((id != -1) && (ret >= 0))
		vfbio_lun_get_size(prev_id, &lun_size);

	if ((free_size + lun_size) < size)
	{
		printf("Not enough space on flash to burn %lu byte %s (free %llu)\n", size, name_base, free_size + lun_size);
		return -1;
	}

	if (id != -1) {
		if (lun_size)
			vfbio_lun_delete(prev_id);
		ret = vfbio_lun_create(name, size, flags, &id);
	}
	else
	{
		id = prev_id;
	}
	
	if(!ret)
	{
		vfbio_lun_get_blk_size(id, &blk_size);
		ret = vfbio_lun_write(id, 0, (size + blk_size - 1)/blk_size, (void *)addr);
	}

	return ret;
}

static int flash_meminit( ulong addr, ulong size, int img_index )
{
	return flash_lun_vfbio(addr, size, img_index, -1, "meminit", 0);
}

static int flash_smcos( ulong addr, ulong size, int img_index )
{
	return flash_lun_vfbio(addr, size, img_index, -1, "smcos", 0);
}

static int flash_armbl( ulong addr, ulong size, int img_index )
{
	return flash_lun_vfbio(addr, size, img_index, -1, "armbl", 0);
}

static int flash_bootfs_vfbio( ulong addr, ulong size, int img_index )
{
	int id = (img_index == 1? IMAGE_VOL_ID_1:IMAGE_VOL_ID_2);
	/* TODO: add support for transparent encryption */
	return flash_lun_vfbio(addr, size, img_index, id, "bootfs", 0);
}

static int flash_rootfs_vfbio( ulong addr, ulong size, int img_index )
{
	int id = (img_index == 1? IMAGE_VOL_ID_1:IMAGE_VOL_ID_2) + 1;
	/* TODO: add support for transparent encryption */
	return flash_lun_vfbio(addr, size, img_index, id, "rootfs", 0);
}

static int vfbio_load_bootfs( int img_index, uint32_t bootfs_load_addr )
{
	int ret;
	char cmd[128];
	char name[32];
	int fit_auth;
	int id;

	sprintf(name, "bootfs%d", img_index);
	ret = vfbio_lun_get_id(name, &id);
	if(!ret)
		ret = vfbio_lun_read(id, 0, UINT_MAX, (void *)(ulong)bootfs_load_addr);

	if (!ret) {
		fit_auth = bcm_sec_validate_fit((void *)(ulong)bootfs_load_addr, 0x10000);

		sprintf(name, "/dev/flash-rootfs%d", img_index);
		/* If device mapper not being used, set rootfs_opts manually */
		if ((bcm_handle_mapper((void *)(ulong)bootfs_load_addr, name, "")) != 0)
		{
			/* Set default bootargs */
			sprintf(cmd, "env set -f rootfs_opts root=%s", name);
			run_command(cmd, 0);
		}

		if (!fit_auth) {
			bcm_board_boot_fdt_fixup_from_fit((void *)(ulong)bootfs_load_addr);
		}
	}

	return ret;
}
#endif

#ifdef CONFIG_BCMBCA_BOARD_SPECIFIC_DT
#define BCMBCA_BOARDID_MAXSIZE	64
#define BCMBCA_VOICEBOARDID_MAXSIZE	BCMBCA_BOARDID_MAXSIZE
#include "spl_ddrinit.h"

static int check_board_compat_chipid(const void* fdt, long offset)
{
	int ret = -1, len = 0;
	u32* compat_id;
	u32 chipid = bcmbca_get_chipid();

	compat_id = (u32*)fdt_getprop(fdt, offset, "compat_chip_id", &len);
	if (compat_id && len) {
		while (len) {
			if (be32_to_cpu(*compat_id) == chipid) {
				ret = 0;
				break;
			}
			compat_id++;
			len -= sizeof(u32);
		}
	}
	else
		ret = 0;

	return ret;
}

static int on_boardid(const char *name, const char *value, enum env_op op, int flags)
{
	char boardid[BCMBCA_BOARDID_MAXSIZE+4];
	int ret = 0;
	int img_index = 0;
	long conf, node, offset;
	uint32_t env_mcb;
	const void* fdt;
	size_t size;

	if (forced_updates || ((flags & H_INTERACTIVE) == 0))
		return 0;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:
		img_index = get_active_img_idx();
		if((img_index < MIN_IMG_INDEX) || (img_index > MAX_IMG_INDEX))
			img_index = MIN_IMG_INDEX;
		/* Load bootfs to load address */
#ifdef CONFIG_BCMBCA_VFBIO
		if (vfbio_load_bootfs(img_index, load_addr))
		{
			printf("Failed to load bootfs\n");
			return 0;
		}
#else
		if(  strcasecmp_image_media(FLASH_DEV_STR_NAND) == 0 )  {
#ifdef BCA_SDK_NAND_CMD
			if (nand_load_fit(img_index, load_addr))
			{
				printf("Failed to load bootfs\n");
				return 0;
			}
#endif
		}

		if( ( strcasecmp_image_media(FLASH_DEV_STR_EMMC) == 0 ) ) {
#ifdef BCA_SDK_EMMC_CMD
			ret = emmc_load_bootfs(img_index, load_addr);
			if (ret)
			{
				printf("Failed to load bootfs\n");
				return 0;
			}
#endif
		}
		if( ( strcasecmp_image_media(FLASH_DEV_STR_SPINOR) == 0 ) ) {
#ifdef BCA_SDK_SPINOR_CMD
			ret = spinor_load_bootfs(load_addr);
			if (ret)
			{
				printf("Failed to load bootfs\n");
				return 0;
			}
#endif
		}
#endif

		conf = fdt_path_offset((void*)load_addr, FIT_IMAGES_PATH);
		if (conf < 0) {
			printf("Cannot find /images node: %ld\n", conf);
			return 0;
		}

		if (strlen(value) > BCMBCA_BOARDID_MAXSIZE)
		{
			printf("boardid is too long, max size is %d\n", BCMBCA_BOARDID_MAXSIZE);
			return 1;
		}
		sprintf(boardid,"fdt_%s", value);
		for (node = fdt_first_subnode((const void*)load_addr, conf); node >= 0; node = fdt_next_subnode((const void*)load_addr, node))
		{
			const char* image_name = fit_get_name((const char*)load_addr, node, NULL);

			if (strcmp(image_name, boardid) == 0)
			{
				const uint32_t* memcfg;
				uint32_t fdt_mcb;

				ret = fit_image_get_data_and_size((const void*)load_addr, node, &fdt, &size);
				if (ret)
				{
					printf("Failed to get FDT\n");
					return 0;
				}

				/* check compat chip id first to make sure this board id support the chip */
				offset = fdt_path_offset(fdt, "/");
				if (check_board_compat_chipid(fdt, offset))
					goto list_board_id;

				offset = fdt_path_offset(fdt, "/memory_controller");
				if (offset < 0) {
					printf("Not found memory_controller node in FDT\n");
					return 0;
				}

				memcfg = fdt_getprop(fdt, offset, "memcfg", NULL);
				if (memcfg == NULL)
				{
					printf("Can't find memcfg parameter in DTB\n");
					return 0;
				}
				fdt_mcb = be32_to_cpu(*memcfg);
				env_mcb = env_get_hex("MCB", 0);
				if (env_mcb & BP_DDR_CONFIG_OVERRIDE)
				{
					printf("MCB sticky bit is set, MCB is not updated\n Current MCB 0x%x, FDT MCB 0x%x\n", env_mcb, fdt_mcb);
					return 0;
				}
				printf("Updating MCB environment from 0x%x to 0x%x\n", env_mcb, fdt_mcb);
				env_set_hex("MCB", fdt_mcb);
				env_set("boardid", value);
				printf("Memory Configuration Changed -- SAVING ENV AND REBOOT NEEDED\n");
				return 0;
			}
		}
list_board_id:
		printf("Error: boardid %s not supported.\nList of supported boards:\n", value);
		for (node = fdt_first_subnode((const void*)load_addr, conf); node >= 0; node = fdt_next_subnode((const void*)load_addr, node))
		{
			const char* name = fit_get_name((const char*)load_addr, node, NULL);
			if ((strncmp("fdt_", name, 4) == 0) && (strcmp("fdt_uboot", name))) {
				if (fit_image_get_data_and_size((const void*)load_addr, node, &fdt, &size) == 0 ) {
					/* only list the board id if its compat chip id match this chip */
					offset = fdt_path_offset(fdt, "/");
					if (check_board_compat_chipid(fdt, offset))
						continue;

					printf("%s\n", (name+4));
				}
			}
		}
		return 1;
	case env_op_delete:
		if (flags & H_FORCE)
			return 0;

		printf("## boardid may not be deleted\n");
		return 1;
	default:
		return 0;
	}
}
U_BOOT_ENV_CALLBACK(boardid, on_boardid);

static int on_voiceBoardid(const char *name, const char *value, enum env_op op, int flags)
{
	char boardid[BCMBCA_BOARDID_MAXSIZE+4];
	int ret = 0;
	int img_index = 0;
	long conf, node, offset;
	char *pboardid;

	if (forced_updates || ((flags & H_INTERACTIVE) == 0))
		return 0;

	pboardid = env_get("boardid");
	if (!pboardid)
	{
		printf("boardId must be set first.\n");
		return 1;
	}

	switch (op) {
	case env_op_create:
	case env_op_overwrite:
		img_index = get_active_img_idx();
		if((img_index < MIN_IMG_INDEX) || (img_index > MAX_IMG_INDEX))
			img_index = MIN_IMG_INDEX;
		/* Load bootfs to load address */
#ifdef CONFIG_BCMBCA_VFBIO
		if (vfbio_load_bootfs(img_index, load_addr))
		{
			printf("Failed to load bootfs\n");
			return 0;
		}
#else
		if(  strcasecmp_image_media(FLASH_DEV_STR_NAND) == 0 )  {
#ifdef BCA_SDK_NAND_CMD
			if (nand_load_fit(img_index, load_addr))
			{
				printf("Failed to load bootfs\n");
				return 0;
			}
#endif
		}

		if( ( strcasecmp_image_media(FLASH_DEV_STR_EMMC) == 0 ) ) {
#ifdef BCA_SDK_EMMC_CMD
			ret = emmc_load_bootfs(img_index, load_addr);
			if (ret)
			{
				printf("Failed to load bootfs\n");
				return 0;
			}
#endif
		}
#endif

		conf = fdt_path_offset((void*)load_addr, FIT_IMAGES_PATH);
		if (conf < 0) {
			printf("Cannot find /images node: %ld\n", conf);
			return 0;
		}

		if (strlen(value) > BCMBCA_VOICEBOARDID_MAXSIZE)
		{
			printf("voiceboardid is too long, max size is %d\n", BCMBCA_VOICEBOARDID_MAXSIZE);
			return 1;
		}
		sprintf(boardid,"fdt_%s", pboardid);
		for (node = fdt_first_subnode((const void*)load_addr, conf); node >= 0; node = fdt_next_subnode((const void*)load_addr, node))
		{
			const char* image_name = fit_get_name((const char*)load_addr, node, NULL);

			if (strcmp(image_name, boardid) == 0)
			{
				size_t size;
				const void* fdt;
				int i, len, idx, slicCount;
				const char *sliclist;

				ret = fit_image_get_data_and_size((const void*)load_addr, node, &fdt, &size);
				if (ret)
				{
					printf("Failed to get FDT\n");
					return 0;
				}

				offset = fdt_path_offset(fdt, "/bcm_voice");
				if (offset < 0) {
					printf("Voice node is not found in FDT. Please add the valid sliclist to the dts file.\n");
					return offset;
				}
				idx = fdt_stringlist_search(fdt, offset, "sliclist", value);
				/* is the string is found, save it and exit */
				if (0 <= idx)
				{
					printf("-- saving env and reboot needed\n");
					return 0;
				}
				printf("%d: %s is not found. idx=%d\n", __LINE__, value, idx);
				slicCount = fdt_stringlist_count(fdt, offset, "sliclist");
				printf("slicCount=%d. List of supported daughter cards:\n", slicCount);
				for (i=0; i < slicCount; i++)
				{
					sliclist = fdt_stringlist_get(fdt, offset, "sliclist", i, &len);
					if (sliclist)
					   printf("%s\n", sliclist);
					else
					   printf("%d: fdt_stringlist_get failed. len=%d\n", __LINE__, len);
				}
				return 1;
			}
		}
		return 1;
	case env_op_delete:
		printf("-- Saving env and reboot needed\n");
		return 0;
	default:
		return 0;
	}
}
U_BOOT_ENV_CALLBACK( voiceboardid, on_voiceBoardid);
#endif

static int get_active_img_idx( void )
{
	int node,len;
	unsigned int active_img_idx = 0;
	fdt32_t * nodep = NULL;
	node = fdt_path_offset(gd->fdt_blob, "/chosen");
	if (node < 0) {
		printf("Can't find /chosen node in uboot DTB\n");
		return node;
	}
	nodep = (fdt32_t *)fdt_getprop(gd->fdt_blob, node, "active_image", &len);
	if(nodep != NULL)
		active_img_idx = fdt32_to_cpu(*(fdt32_t *)nodep);

	return active_img_idx;
}

static int set_active_img_idx( int img_idx )
{
	int node;
	node = fdt_path_offset(gd->fdt_blob, "/chosen");
	int ret = -1;
	if (node < 0) {
		printf("Can't find /chosen node in uboot DTB, device may not boot properly!\n");
	} else {
		ret = fdt_setprop_u32((char *)gd->fdt_blob, node, "active_image", img_idx);
		if(ret)
		{
			printf("Could not set active image node in the fdt, device may not boot properly!\n");
		}
	}
	return ret;
}

/************************************************************
 *                Flash Specific Functions                  *
 ************************************************************/
#ifdef BCA_SDK_EMMC_CMD
static int emmc_restoredefault(void)
{
	ulong part_size_blk, part_start_blk;
	char cmd[128];
	int ret = -1;

	/* Switch to user data partition */
	if ( run_command("mmc dev 0 0", 0) == 0 ) {
		env_set("part_start_blk", NULL);
		env_set("part_size_blk", NULL);
		run_command("part start mmc 0 data part_start_blk", 0);
		run_command("part  size mmc 0 data part_size_blk", 0);
		part_start_blk = env_get_hex("part_start_blk", 0);
		part_size_blk  = env_get_hex("part_size_blk", 0);

		/* If data exists, delete it */
		if( part_size_blk ) {
			sprintf(cmd, "mmc erase 0x%lx 0x%lx", part_start_blk, part_size_blk);
			ret = run_command(cmd, 0);
		} else {
			ret = 0;
		}
	} else {
		printf("Error: Cannot switch to userdata partition!\n");
	}

	return ret;
}

static int emmc_rdwr_boot_part( ulong addr, ulong size, int img_index, int write )
{
	char cmd[128];
	ulong num_blocks = 0;
	ulong block_addr = 0;
	ulong part_size_blk = 0;
	int mmc_boot_part = 0;
	int ret = -1;
	struct mmc *mmc = NULL;

	mmc = find_mmc_device(0);
	if( !mmc ) {
		printf("Error: Cannot access mmc device!\n");
		goto emmc_rdwr_boot_part_exit;
	}

	/* Get active boot partition */
	mmc_boot_part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);

	/* Default to first boot partition if nothing is set */
	if( !mmc_boot_part )
		mmc_boot_part = 1;

	/* Calculate number of blocks */
        num_blocks = size/mmc->read_bl_len + (size%mmc->read_bl_len?1:0);

	/* Determine size of raw boot partition */
	part_size_blk = mmc->capacity_boot/mmc->read_bl_len;
	if( num_blocks > part_size_blk ) {
		printf("Error: Insufficient space in boot partition %d for 0x%lx blocks. Partition size is 0x%lx\n", mmc_boot_part, num_blocks, part_size_blk);
		goto emmc_rdwr_boot_part_exit;
	}

	/* Switch to boot partition */
	sprintf(cmd, "mmc dev 0 %d", mmc_boot_part);
	if ( run_command(cmd, 0) ) {
		printf("Error: Cannot switch to bootpartition %d!\n", mmc_boot_part);
		goto emmc_rdwr_boot_part_exit;
	}

	/* Write to raw boot partition */
	sprintf(cmd, "mmc %s 0x%lx 0x%lx 0x%lx", (write?"write":"read"), addr, block_addr, num_blocks);
	ret = run_command(cmd, 0);
	printf("%s: %s (bootp%d)\n", __FUNCTION__, cmd, mmc_boot_part);

	/* Switch back to userdata partition */
	run_command("mmc dev 0 0", 0);

emmc_rdwr_boot_part_exit:
	if( ret )
		printf("Error: Failed %s 0x%lx blocks from/to boot partition %d!\n", (write?"writing":"reading"), num_blocks, mmc_boot_part);

	return ret;
}

static int emmc_rdwr_userdata_part( char * part_name, ulong addr, ulong size, int write )
{
	char cmd[128];
	ulong num_blocks = 0;
	ulong block_addr = 0;
	ulong part_size_blk = 0;
	int ret = -1;
	struct mmc *mmc = NULL;

	mmc = find_mmc_device(0);
	if( !mmc ) {
		printf("Error: Cannot access mmc device!\n");
		goto emmc_rdwr_usrdata_part_exit;
	}

	/* Calculate number of blocks */
        num_blocks = size/mmc->read_bl_len + (size%mmc->read_bl_len?1:0);

	/* Switch to user data partition */
	if ( run_command("mmc dev 0 0", 0) ) {
		printf("Error: Cannot switch to userdata partition!\n");
		goto emmc_rdwr_usrdata_part_exit;
	}

	/* Get GPT partition size in blocks */
	env_set("part_size_blk", NULL);
	sprintf(cmd, "part size mmc 0 %s part_size_blk", part_name);
	if(run_command(cmd, 0) == 0) {
		part_size_blk = env_get_hex("part_size_blk", 0);
		if( num_blocks > part_size_blk ) {
			printf("Error: Insufficient space in partition %s for 0x%lx blocks. Partition size is 0x%lx\n", part_name, num_blocks, part_size_blk);
			goto emmc_rdwr_usrdata_part_exit;
		}
	} else {
		printf("Error: Cannot determine size of GPT partition %s\n", part_name);
		goto emmc_rdwr_usrdata_part_exit;
	}

	/* Get start block address of partition */
	sprintf(cmd, "part start mmc 0 %s part_start_blk", part_name);
	env_set("part_start_blk", NULL);
	if(run_command(cmd, 0) == 0) {
		block_addr = env_get_hex("part_start_blk", 0);
		sprintf(cmd, "mmc %s 0x%lx 0x%lx 0x%lx", (write?"write":"read"), addr, block_addr, num_blocks);
		ret = run_command(cmd, 0);
		printf("%s: %s (%s)\n", __FUNCTION__, cmd, part_name);
	} else {
		printf("Error: Cannot determine start block of GPT partition %s\n", part_name);
		goto emmc_rdwr_usrdata_part_exit;
	}

emmc_rdwr_usrdata_part_exit:
	if( ret )
		printf("Error: Failed %s 0x%lx blocks from/to GPT partition %s!\n", (write?"writing":"reading"), num_blocks, part_name);
	return ret;
}

static int flash_rootfs_emmc( ulong addr, ulong size, int img_index )
{
	char part_name[128];
	int ret = CMD_RET_FAILURE;

	if( !addr || !size || img_index < MIN_IMG_INDEX || img_index > MAX_IMG_INDEX )
		return CMD_RET_USAGE;

	sprintf(part_name, "rootfs%d", img_index);
	ret = emmc_rdwr_userdata_part( part_name, (ulong)addr, size, 1 );

	return ret;
}

static int flash_bootfs_emmc( ulong addr, ulong size, int img_index )
{
	char part_name[128];
	int ret = CMD_RET_FAILURE;

	if( !addr || !size || img_index < MIN_IMG_INDEX || img_index > MAX_IMG_INDEX )
		return CMD_RET_USAGE;

	sprintf(part_name, "bootfs%d", img_index);
	ret = emmc_rdwr_userdata_part( part_name, (ulong)addr, size, 1 );
	return ret;
}

static int flash_loader_emmc( ulong addr, ulong size, int img_index )
{
	int ret = -1;

	if( !addr || !size )
		return CMD_RET_USAGE;

	ret = emmc_rdwr_boot_part( addr, size, img_index, 1);

	return ret;
}

static int set_emmc_metadata( char* metadata, int size )
{
	char part_name[128];
	int ret;
	int i;

	for (i = 1; i < 3; i++) {
		sprintf(part_name, "metadata%d", i);
		ret = emmc_rdwr_userdata_part( part_name, (ulong)metadata, size, 1 );
	}
	return ret;
}

static int get_emmc_metadata( char* metadata, int size )
{
	int ret;
	int committed = 0;
	int valid[2] = {0,0};
	int seq[2] = {-1,-1};
	int i;
	char part_name[128];

	for (i = 1; i < 3; i++) {
		sprintf(part_name, "metadata%d", i);
		ret = emmc_rdwr_userdata_part( part_name, (ulong)metadata, size, 0 );
		if (0 == validate_metadata(metadata, valid, &committed, seq)) {
			break;
		} else {
			printf("metadata parse error\n");
			ret = CMD_RET_FAILURE;
		}
	}
	return ret;
}

static ulong emmc_get_bootfs_size( char * fit )
{
	int images_noffset;
	int noffset;
	int ndepth;
	int count;
	size_t size;
	ulong bootfs_size = 0;
	const void *data;

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
		       FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return 0;
	}

	/* Process all image subnodes */
	for (ndepth = 0, count = 0,
	     noffset = fdt_next_node(fit, images_noffset, &ndepth);
			(noffset >= 0) && (ndepth > 0);
			noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the images parent node,
			 * i.e. component image node.
			 */
			count++;

			if (fit_image_get_data_and_size(fit, noffset, &data, &size) == 0) {
				bootfs_size = ((ulong)data+(ulong)size) - (ulong)fit;
			}
		}
	}
	printf("Bootfs size is %lu bytes\n", bootfs_size);
	return bootfs_size;
}

static int emmc_add_gpt_part( char * name, ulong size )
{
	char * current_parts = NULL;
	char * new_parts = NULL;
	int ret = -1;

	/* Read partitions into variable */
	env_set("current_parts", NULL);
	run_command("gpt read mmc 0 current_parts", 0);
	current_parts = env_get("current_parts");
	if(current_parts != NULL) {
		if(strstr(current_parts, name) ) {
			printf("%s: GPT partition:%s of size:%luMiB exists!\n", __FUNCTION__, name, size);
			return 0;
		} else {
			printf("%s: Adding GPT partition %s of size %luMiB\n", __FUNCTION__, name, size);
		}

		/* Update partitions */
		new_parts = malloc(strlen(current_parts) + 1024 );
		if( new_parts )	{
			sprintf(new_parts, "%sname=%s,size=%luMiB;", current_parts, name, size);
			ret = env_set("current_parts", new_parts);
			free(new_parts);
			ret = run_command("gpt write mmc 0 $current_parts", 0);
			ret = run_command("gpt verify mmc 0 $current_parts", 0);
		} else {
			printf("%s: Error allocating memory for partition string!\n", __FUNCTION__);
		}
	}

	//env_set("current_parts", NULL);
	return ret;

}

/* This function will create GPT partitions if it detects partition sizes specfied via
 * uboot environment variables. The env variable name has to be in the format:
 * 	<partition_name>_vol_size=<size in MiB>
 * The function will then create GPT partition with name <partition_name> of size <size>
 *
 * NOTE: For NAND, equivalent ubi volumes are also created dynamically based on the same
 * uboot env variables. However, for NAND the volumes are created in the mount-fs.sh linux
 * startup script instead of in uboot proper.
 */
#define VOL_SIZE_STR "_vol_size="
static int emmc_do_gpt_fixup(void)
{
	env_t *ep;
	char *envbuf = NULL;
	int ret = -2;
	int elen;
	char *config;
	char * token = NULL;
        char * token2 = NULL;
        char * name;
        int value;
	ulong token_len = 0;

	config = env_get("env_boot_magic");
	if (NULL == config)
	{
		printf("env_boot_magic missing in the env\n");
		return -1;
	}
	elen = simple_strtoul(config, NULL, 0);
	elen = max(elen, CONFIG_ENV_SIZE);
	envbuf = malloc(elen);
	memset(envbuf, 0, elen);

	if (!envbuf)
	{
		printf("memory allocation failed\n");
		return -2;
	}
	ep = (env_t *) (envbuf + 8);
	ret = env_export(ep);

	/* Search environment for variable names matching <part>_vol_size */
	token = (char*)(ep->data);
        while(strlen(token) && (ret == 0) && ((ulong)token < ((ulong)envbuf+elen)))
        {
		token_len = strlen(token);
                if( (token2 = strstr(token, VOL_SIZE_STR)) )
                {
                        *token2 = '\0';
                        name = token;
                        value = atoi( (char*)((ulong)token2 + strlen(VOL_SIZE_STR)) );
                        printf("%s: Detected env defined partition:%s, of size:%dM\n", __FUNCTION__, name, value);
			ret = emmc_add_gpt_part(name, value);
			ret = 0;
                }
                token = (char*)((ulong)token + token_len + 1);
        }
	free(envbuf);
	return(ret);
}

static int do_gpt_fixup(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	return emmc_do_gpt_fixup();
}

static int emmc_load_bootfs( int img_index, uint32_t bootfs_load_addr )
{
	char cmd[128];
	ulong size;
	int ret = CMD_RET_FAILURE;
	int fit_auth= -1;

	/* Set rootfs volume id */
	int rootfs_partid = (img_index == 1?IMAGE_PART_ID_1:IMAGE_PART_ID_2)+1;

	/* Switch to userdata partition */
	if( run_command("mmc dev 0 0", 0) == 0 ) {
		//FIXME: OR skip loading if FIT is already in memory
		//FIXME: IF FIT is loaded from flash, reverify RSA of header
		sprintf(cmd, "bootfs%d", img_index);

		/* Get FDT header */
		size = sizeof(struct fdt_header);
                ret = emmc_rdwr_userdata_part( cmd, (ulong)bootfs_load_addr, size, 0 );

		/* Retrieve entire FIT header */
                ret = emmc_rdwr_userdata_part( cmd, (ulong)bootfs_load_addr,
			bcm_sec_get_reqd_load_size((void*)(uintptr_t)bootfs_load_addr), 0 );

		if (!ret) {
			fit_auth = bcm_sec_validate_fit((void*)(uintptr_t)bootfs_load_addr, 0x10000);
		}

		/* Determine size of bootfs from fdt_header and load it */
		size = emmc_get_bootfs_size((void *)(uintptr_t)bootfs_load_addr);
                ret = emmc_rdwr_userdata_part( cmd, (ulong)bootfs_load_addr, size, 0 );

		/* generate mapper parameters */
		sprintf(cmd,"/dev/mmcblk0p%d", rootfs_partid);

		/* If device mapper not being used, set rootfs_opts manually */
		if ( (bcm_handle_mapper((void*)(ulong)bootfs_load_addr, cmd, "")) != 0)
		{
			/* Set default bootargs */
			sprintf(cmd, "env set -f rootfs_opts root=/dev/mmcblk0p%d", rootfs_partid);
			run_command(cmd, 0);
		}

		if (!fit_auth) {
			bcm_board_boot_fdt_fixup_from_fit((void*)(uintptr_t)bootfs_load_addr);
		}
	}
	return ret;
}

#endif /* BCA_SDK_EMMC_CMD */

#ifdef BCA_SDK_NAND_CMD
#define EXTRA_SPC_SUFFIX_STR "_extra_space"
static uint64_t nand_get_part_extra_bytes(char* partition_prefix)
{
	char var_name[128];
	char * extra_space_str = NULL;
	uint64_t extra_bytes = 0;
	unsigned long iargs[4] = {0};
	char units[4]= {0};
	sprintf(var_name, "%s%s", partition_prefix, EXTRA_SPC_SUFFIX_STR);
	extra_space_str = env_get(var_name);
	if( extra_space_str ) {
		parse_env_nums(extra_space_str, 1, iargs, units);
		extra_bytes = ((long long)iargs[0]) << suffix2shift(units[0]);
	}
	if( extra_bytes < SIZE_OVERHEAD )
		extra_bytes = SIZE_OVERHEAD;

	return extra_bytes;
}

static uint64_t nand_get_avail_space(char * update_vol_name)
{
	int i;
	static struct ubi_device *ubi;
	static struct ubi_volume *vol;
	uint64_t vol_size = 0;
	uint64_t avail_space = 0;
	int ret = -1;

	ret = run_command("ubi part image", 0);
	if( ret == 0 ) {
		ubi = ubi_devices[0];
		for (i = 0; i < (ubi->vtbl_slots + 1); i++) {
			if (!ubi->volumes[i])
				continue;	/* Empty record */
			vol = ubi->volumes[i];
			if( strcmp(vol->name,update_vol_name) == 0 ) {
				vol_size = (uint64_t)vol->reserved_pebs * vol->usable_leb_size;
				break;
			}
		}
		avail_space = ubi->avail_pebs * ubi->leb_size + vol_size;
	}
	return avail_space;
}

static int nand_restoredefault(void)
{
	int ret = -1;
	if(run_command("ubi part image", 0) == 0) {

		/* If data exists, delete it */
		if(run_command("ubi check data", 0) == 0 ) {
			ret = run_command("ubi remove data", 0);
		} else {
			ret = 0;
		}
	} else {
		printf("Error: Cannot attach ubi!\n");
	}
	return ret;
}

static int flash_rootfs_nand( ulong addr, ulong size, int img_index )
{
	char cmd[128];
	int ret = CMD_RET_FAILURE;
	int rootfs_volid = (img_index == 1? IMAGE_VOL_ID_1:IMAGE_VOL_ID_2) + 1;
	uint64_t avail_bytes = 0;
	uint64_t extra_space_bytes = nand_get_part_extra_bytes("rootfs");

	if( !addr || !size || img_index < MIN_IMG_INDEX || img_index > MAX_IMG_INDEX )
		return CMD_RET_USAGE;

	if(run_command("ubi part image", 0) == 0) {

		/* check if new rootfs will fit in flash */
		sprintf(cmd, "rootfs%d", img_index);
		avail_bytes =  nand_get_avail_space(cmd);
		if( avail_bytes < size+extra_space_bytes ) {
			printf("Error: Insufficient space in NAND for 0x%llx Bytes . Available size is 0x%llx\n", size+extra_space_bytes, avail_bytes);
			return -1;
		}

		/* If rootfs1 exists, delete it */
		sprintf(cmd, "ubi check rootfs%d", img_index);
		if(run_command(cmd, 0) == 0) {
			sprintf(cmd, "ubi remove rootfs%d", img_index);
			run_command(cmd, 0);
		}

		/* Create rootfs volume */
		sprintf(cmd, "ubi create rootfs%d %llx  dynamic %d", img_index, size+extra_space_bytes,
								rootfs_volid);
		ret = run_command(cmd, 0);

		/* Write rootfs data volume */
		if( ret == 0 ) {
			sprintf(cmd, "ubi write %lx rootfs%d %lx\n", addr, img_index, size);
			ret = run_command(cmd, 0);
		}

		run_command("ubi detach", 0);
	}
	return ret;
}

static int flash_bootfs_nand( ulong addr, ulong size, int img_index )
{
	char cmd[128];
	int ret = CMD_RET_FAILURE;
	int bootfs_volid = (img_index == 1? IMAGE_VOL_ID_1:IMAGE_VOL_ID_2);
	int active_img_idx;
	uint64_t avail_bytes = 0;
	uint64_t extra_space_bytes = nand_get_part_extra_bytes("bootfs");

	if( !addr || !size || img_index < MIN_IMG_INDEX || img_index > MAX_IMG_INDEX )
		return CMD_RET_USAGE;

	ret = run_command("ubi part image", 0);
	if( ret ) {
		/* Attach failed, check if we are running bootstrap code */
		active_img_idx = get_active_img_idx();
		if( active_img_idx == ACTIVE_IMGIDX_BOOTSTRAP ) {
			/* We are running bootstrap image, wipe Image partition and retry */
			printf("UBI attach fails for bootstrap image --> erasing IMAGE MTD partition\n");
			run_command("mtd erase image", 0);
			ret = run_command("ubi part image", 0);
		}
	}

	if(ret == 0) {
		/* If fit1 exists, delete it */
		if(run_command("ubi check fit1", 0) == 0)
			run_command("ubi remove fit1", 0);

		/* check if new bootfs will fit in flash */
		sprintf(cmd, "bootfs%d", img_index);
		avail_bytes =  nand_get_avail_space(cmd);
		if( avail_bytes < size+extra_space_bytes ) {
			printf("Error: Insufficient space in NAND for 0x%llx Bytes . Available size is 0x%llx\n", size+extra_space_bytes, avail_bytes);
			return -1;
		}

		/* If bootfs exists, delete it */
		sprintf(cmd, "ubi check bootfs%d", img_index);
		if(run_command(cmd, 0) == 0) {
			sprintf(cmd, "ubi remove bootfs%d", img_index);
			run_command(cmd, 0);
		}

		/* Create bootfs volume */
		sprintf(cmd, "ubi create bootfs%d %llx  static %d", img_index, size+extra_space_bytes,
								bootfs_volid);
		ret = run_command(cmd, 0);

		/* Write img data */
		if( ret == 0 ) {
			sprintf(cmd, "ubi write %lx bootfs%d %lx\n", addr, img_index, size);
			ret = run_command(cmd, 0);
		}

		run_command("ubi detach", 0);
	} else {
		printf("Error! UBI attach failed!\n");
	}

	return ret;
}


static int flash_loader_nand( ulong addr, ulong size, int img_index )
{
	struct mtd_info * mtd;
	struct erase_info erase_op = {};
	loff_t off;
	int ret=0;
	size_t retlen;
	ulong remaining_bytes = size;
	mtd = get_mtd_device_nm("loader");

	if( !addr || !size )
		return CMD_RET_USAGE;

	if( !mtd ) {
		printf("ERROR: Failed to retrieve MTD device for NAND loader!\n");
		return -1;
	}

	if( size > mtd->size ) {
		printf("ERROR: loader size 0x%08lx is greater than mtd partition size 0x%08llx!\n", size, mtd->size);
		put_mtd_device(mtd);
		return -1;
	}

	(void)img_index;

	/* Block by block write of loader */
	for( off=0; off < mtd->size; off += mtd->erasesize )
	{
		if( remaining_bytes ) {
			/* Erase block */
			erase_op.mtd = mtd;
			erase_op.addr = off;
			erase_op.len = mtd->erasesize;
			erase_op.scrub = 0;
			printf("Erasing next block at 0x%08llx\n", off);
			ret = mtd_erase(mtd, &erase_op);

			/* Abort if its not a bad block error */
			if ((ret == -EIO) && erase_op.fail_addr) {
				printf("Skipping bad block at 0x%08llx\n", erase_op.fail_addr);

				/* Truncate image size if we cant fit due to bad block skip */
				if( remaining_bytes > (mtd->size - off - mtd->erasesize) )
					remaining_bytes -= (remaining_bytes < mtd->erasesize?remaining_bytes:mtd->erasesize);

				continue;
			} else if ( ret ) {
				printf("ERROR: Failed to erase NAND block 0x%08llx\n", off);
				break;
			}

			/* write block */
			printf("Writing next block at 0x%08llx\n", off);
			ret = mtd_write(mtd, off,
				(remaining_bytes < mtd->erasesize?remaining_bytes:mtd->erasesize),
				&retlen, (const uchar*)(addr));

			if( ret ) {
				printf("ERROR: Failed to write NAND block 0x%08llx\n", off);
				break;
			}

			addr += (remaining_bytes < mtd->erasesize?remaining_bytes:mtd->erasesize);
			remaining_bytes -= (remaining_bytes < mtd->erasesize?remaining_bytes:mtd->erasesize);
		}
		else
			break;
	}

	if( ret ) {
		/* Loader upgrade failed for some reason */
		printf("ERROR: Failed to flash NAND loader! re=%d\n", ret);
	} else {
		/* Loader upgrade worked */
		printf("NAND loader flashed successfully!\n");
	}

	put_mtd_device(mtd);
	return ret;
}

static int set_nand_metadata( char* metadata, int size )
{
	char cmd[128];
	int ret;
	int i;
	int volmap[] = {-1,METADATA_VOL_ID_1,METADATA_VOL_ID_2};

	ret = ubi_part("image", NULL);
	if (ret != 0) {
		return (ret);
	}
	for (i = 1; i < 3; i++) {
		sprintf(cmd, "ubi remove metadata%d", i);
		run_command(cmd, 0);
		sprintf(cmd,
			"ubi create metadata%d %d static %d", i,
			METADATA_SIZE + 1024, volmap[i]);
		run_command(cmd, 0);
		sprintf(cmd, "metadata%d", i);
		ubi_volume_write(cmd, metadata, size);
	}
	run_command("ubi detach", 0);
	return ret;
}

static int get_nand_metadata( char* metadata, int size )
{
	int ret;
	int committed = 0;
	int valid[2] = {0,0};
	int seq[2] = {-1,-1};
	int i;
	char name[128];

	ret = ubi_part("image", NULL);
	if (ret != 0) {
		return (ret);
	}
	for (i = 1; i < 3; i++) {
		sprintf(name, "metadata%d", i);
		ret = ubi_volume_read(name, metadata, size);
		printf("read from %s returned %d\n", name, ret);
		if (0 == validate_metadata(metadata, valid, &committed, seq)) {
			break;
		} else {
			printf("metadata parse error\n");
			ret = CMD_RET_FAILURE;
		}
	}
	run_command("ubi detach", 0);
	return ret;
}

static int nand_load_fit( int img_index, uint32_t fit_load_addr )
{
	char cmd[128];
	int ret = CMD_RET_FAILURE;

	if(run_command("ubi part image", 0) == 0) {
		//FIXME: OR skip loading if FIT is already in memory
		//FIXME: IF FIT is loaded from flash, reverify RSA of header
		sprintf(cmd, "ubi read %x bootfs%d", fit_load_addr, img_index);
		ret = run_command(cmd, 0);
		/* may be it is just bootstrap image */
		if (ret) {
			sprintf(cmd, "ubi read %x fit1", fit_load_addr);
			ret = run_command(cmd, 0);
		}
	}
	return ret;
}

static int nand_load_bootfs( int img_index, uint32_t bootfs_load_addr )
{
	char cmd[128];
	char device[32];
	unsigned int magic = 0;
	int ret = CMD_RET_FAILURE;
	int fit_auth= -1;

	/* Set rootfs volume id */
	int rootfs_volid = (img_index == 1?IMAGE_VOL_ID_1:IMAGE_VOL_ID_2)+1;

	if(run_command("ubi part image", 0) == 0) {
		/* Set default bootargs */
		/* Determine rootfs type */
		sprintf(cmd, "ubi read %x bootfs%d", bootfs_load_addr, img_index);
		ret = run_command(cmd, 0);
		if (!ret) {
			fit_auth = bcm_sec_validate_fit((void*)(uintptr_t)bootfs_load_addr, 0x10000);
		}

		sprintf(device,"/dev/ubiblock0_%d", rootfs_volid);
		sprintf(cmd,"ubi.mtd=image ubi.block=0,%d rootfstype=squashfs", rootfs_volid);
		if (bcm_handle_mapper((void*)(ulong)bootfs_load_addr, device, cmd) == 0)
		{
			/* mapper has set up the whole thing */
			if (!fit_auth) {
				bcm_board_boot_fdt_fixup_from_fit((void*)(uintptr_t)bootfs_load_addr);
			}
			return(0);
		}

		sprintf(cmd, "ubi read %px rootfs%d %d", &magic, img_index, (int)sizeof(unsigned int));
		if (run_command(cmd, 0) == 0) {
			if( magic == UBIFS_MAGIC ) {
				sprintf(cmd, "env set -f rootfs_opts root=ubi:rootfs%d ubi.mtd=image rootfstype=ubifs", img_index);
				run_command(cmd, 0);
			}
			else if ( magic == SQUASHFS_MAGIC ) {
				sprintf(cmd, "env set -f rootfs_opts root=/dev/ubiblock0_%d ubi.mtd=image ubi.block=0,%d rootfstype=squashfs", rootfs_volid, rootfs_volid);
				run_command(cmd, 0);
			} else {
				printf("ERROR: Invalid rootfs detected in volume rootfs1! Boot aborted!\n");
				return ret;
			}
		} else {
			printf("ERROR: Cannot determine rootfs type! Boot aborted!\n");
			return ret;
		}

		if (!fit_auth &&  magic == SQUASHFS_MAGIC ) {
			bcm_board_boot_fdt_fixup_from_fit((void*)(uintptr_t)bootfs_load_addr);
		}
	}
	return ret;
}
#endif /* BCA_SDK_NAND_CMD */

#if defined(BCA_SDK_EMMC_CMD) || defined(BCA_SDK_NAND_CMD) || defined(BCA_SDK_SPINOR_CMD) || defined(CONFIG_BCMBCA_VFBIO)
static unsigned int bcm_handle_mapper(void* fit, char *flash_device, char *flash_opts)
{
	const char *val;
	char cmd[768];
	char work[768];
	char *cp;
	int off, len;
	char dmdev[32];

 	off = fdt_path_offset(fit, "/brcm_rootfs_encrypt");
	if (off < 0) {
		return(1);
	}
	strcpy(dmdev,"/dev/dm-0");
	if ((val = fdt_getprop(fit, off, "dev", &len))) {
		strncpy(dmdev, val, len);
		dmdev[len] = 0;
	}
	val = fdt_getprop(fit, off, "type", &len);
	if (val == NULL) {
		return(1);
	}
	val = fdt_getprop(fit, off, "mapper", &len);
	if (val == NULL) {
		return(1);
	}
	strncpy(work, val, len);
	work[len] = 0;

	while ((cp = strstr(work,"%DEVICE%"))) {
		strncpy(cmd, work, cp-work);
		sprintf(&cmd[cp-work], "%s %s", \
			flash_device, cp+8);
		strcpy(work,cmd);
	}
	if ((cp = strstr(work,"%IMAGE_KEY%"))) {
		int i;
		val = NULL;
		len = 0;
		off = fdt_path_offset(gd->fdt_blob, "/trust/key_image_aes");
		if (off < 0) {
			printf("ERROR: Can't find /trust/key_image_aes node in boot DTB!\n");
		} else {
			printf("SUCCESS:Found /trust/key_image_aes node in boot DTB!\n");
		}
		val = (char*)(fdt_getprop(gd->fdt_blob, off, "value", &len));
		strncpy(cmd, work, cp-work);
		if(val != NULL)	{
			for (i = 0 ; i < len ; i++) {
			   sprintf(&cmd[cp-work+2*i], "%02x", val[i]);
			   printf("%02x", val[i]);
			}
			sprintf(&cmd[cp-work+2*i], "%s", cp+11);
			strcpy(work,cmd);
		}
	}
	sprintf(cmd, "root=%s %s dm-mod.create=\"%s\" dm-mod.waitfor=%s", dmdev, flash_opts, work, flash_device);
	debug("rootfs_opts:%s\n", cmd);
	env_set("rootfs_opts", cmd);
	return(0);
}
#endif

#if defined(BCA_SDK_SPINOR_CMD)
#define SPINOR_MTD_BLOCK_DEF_NUM		3
static int spinor_load_bootfs(uint32_t bootfs_load_addr)
{
	int ret= -1, blk = SPINOR_MTD_BLOCK_DEF_NUM;
	struct mtd_info *mtd;
	size_t retlen;
	int fit_auth=-1;
	char *rootfs_opts, *blknum;
	char opts[64];

	mtd_probe_devices();
	mtd = get_mtd_device_nm(BOOTFS_PART);
	if (IS_ERR_OR_NULL(mtd)){
		debug("%s:MTD device %s not found, ret %ld\n",__func__, BOOTFS_PART,
		   PTR_ERR(mtd));
		return ret;
	}

	ret = mtd_read(mtd,0,mtd->size,&retlen,(u_char*)(uintptr_t)bootfs_load_addr );

	if (!ret) {
		fit_auth = bcm_sec_validate_fit((void*)(uintptr_t)bootfs_load_addr, 0x10000);
	}

	/*
	 * allow user to specify which mtdblock for rootfs in env as it changes when
	 * there are multiple flashes
	 */
	rootfs_opts = env_get("rootfs_opts");
	if (rootfs_opts) {
		blknum = strstr(rootfs_opts, "mtdblock");
		if (blknum && *(blknum+8)) {
			blk = simple_strtoul(blknum+8, NULL, 10);
		}
	}
	snprintf(opts, 64, "/dev/mtdblock%d", blk);
	/* If device mapper not being used, set the rootfs_opts manually */
	if (bcm_handle_mapper((void*)(ulong)bootfs_load_addr, opts, "rootfstype=squashfs") != 0) {
		if(!rootfs_opts) {
			snprintf(opts, 64, "root=/dev/mtdblock%d rootfstype=squashfs", blk);
			env_set("rootfs_opts", opts);
		}
	}

	if (!fit_auth) {
		bcm_board_boot_fdt_fixup_from_fit((void*)(uintptr_t)bootfs_load_addr);
	}

	put_mtd_device(mtd);
	return ret;
}
static int spinor_restoredefault(void)
{
	struct mtd_info *mtd;
	struct erase_info ei;
	int ret;

	mtd_probe_devices();
	mtd = get_mtd_device_nm(DATA_PART);
	if (IS_ERR_OR_NULL(mtd)){
		printf("ERROR!!:failed to get data partition!!\n");
		return CMD_RET_FAILURE;
	}

	memset(&ei, 0, sizeof(ei));
	ei.mtd = mtd;
	ei.addr = 0;
	ei.len = mtd->size;
	ret = mtd_erase(mtd, &ei);
	if (ret){
		printf("ERROR!!: failed to restore to default!!\n");
		ret = CMD_RET_FAILURE;
	}
	else
		printf("Restore to default done.\n");

	put_mtd_device(mtd);
	return ret;
}

static int flash_loader_spinor( ulong addr, ulong size)
{
	int ret= -1;
	struct mtd_info *mtd;
	struct erase_info ei;
	size_t retlen;

	if( !addr || !size )
		return CMD_RET_USAGE;
	mtd_probe_devices();
	mtd = get_mtd_device_nm(LOADER_PART);
	if (IS_ERR_OR_NULL(mtd)){
		printf("%s:MTD device %s not found, ret %ld\n",__func__, LOADER_PART,
		   PTR_ERR(mtd));
		return ret;
	}

	printf("\nErasing MTD partition %s...\n",LOADER_PART);
	memset(&ei, 0, sizeof(ei));
	ei.mtd = mtd;
	ei.addr = 0;
	ei.len = mtd->size;
	ret = mtd_erase(mtd, &ei);
	if (ret){
		debug("%s:erase MTD device %s fail, ret %d\n",__func__, LOADER_PART,
	   		ret);
		goto done;
	}

	printf("Writing MTD partition %s...\n",LOADER_PART);
	ret = mtd_write(mtd, 0, size, &retlen, (unsigned char*)addr);
	if (ret){
		debug("%s:write MTD device %s fail, ret %d\n",__func__, LOADER_PART,
	   		ret);
	}
	debug("%s:Erase/write MTD device %s done, ret %d\n",__func__, LOADER_PART,
		ret);
done:
	put_mtd_device(mtd);
	return ret;

}

static int write_spinor_partition( char *const partitionname, ulong addr, ulong size )
{
	int ret= -1;
	struct mtd_info *mtd;
	struct erase_info ei;
	size_t retlen;

	if( !addr || !size )
		return CMD_RET_USAGE;

	mtd_probe_devices();
	mtd = get_mtd_device_nm(partitionname);
	if (IS_ERR_OR_NULL(mtd)){
		debug("%s:MTD device %s not found, ret %ld\n",__func__, partitionname,
		   PTR_ERR(mtd));
		return ret;
	}
	printf("\nErasing MTD partition %s...\n",partitionname);
	memset(&ei, 0, sizeof(ei));
	ei.mtd = mtd;
	ei.addr = 0;
	ei.len = mtd->size;
	ret = mtd_erase(mtd, &ei);
	if (ret){
		debug("%s:erase MTD device %s fail, ret %d\n",__func__, partitionname,
	   		ret);
		goto done;
	}

	printf("Writing MTD partition %s...\n",partitionname);
	ret = mtd_write(mtd, 0, size, &retlen, (u_char*)addr);
	if (ret){
		debug("%s:write MTD device %s fail, ret %d\n",__func__, partitionname,
	   		ret);
	}
done:
	put_mtd_device(mtd);
	return ret;
}

static int do_flash_spinor_binary(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	char cmd[128];
	ulong size;
	ulong addr;

	/* Download binary image */
	sprintf(cmd, "tftp %lx %s", load_addr, argv[1]);
	if( run_command(cmd, 0) == 0 ) {
		addr = env_get_hex("fileaddr", 0);
		size = env_get_hex("filesize", 0);
		write_spinor_partition(SPIFLASH_MTDNAME, addr, size);
	} else
		printf("ERROR!!: Failed to tftp spinor binary!!\n");

	return 0;
}

static int do_flash_spinor_bootfs_rootfs(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	char filename[30];
	int flash_fit=0;
	int ret = CMD_RET_FAILURE;
	char cmd[128];
	ulong size;
	ulong addr;


	/* Parse arguments */
	if ( strcmp(argv[0], "flash_spinor_fit") == 0) {
		flash_fit=1;
		sprintf(filename, "brcm_full_linux.itb");
	} else {
		sprintf(filename, "rootfs.squashfs");
	}

	sprintf(cmd, "tftp %lx %s\n", load_addr, filename);
	if( run_command(cmd, 0) == 0 ) {
		size = env_get_hex("filesize", 0);
		addr = env_get_hex("fileaddr", 0);
		if(flash_fit){
			ret = write_spinor_partition(BOOTFS_PART, addr, size);
		}
		else{
			ret = write_spinor_partition(ROOTFS_PART, addr, size);
		}
	}
	else
		printf("ERROR!!: Failed to tftp rootfs binary!!\n");

	return ret;
}
#endif /*BCA_SDK_SPINOR_CMD*/

/************************************************************
 *                    Generic Functions                     *
 ************************************************************/
#ifndef CONFIG_BCMBCA_VFBIO
static int sync_update_loader_bin_env(ulong loader_addr)
{
	env_t *ep;
	uint32_t calc_crc;
	char *envbuf = NULL;
	uint32_t *envintp;
	int ret = -2;
	int i;
	loff_t off;
	size_t rdlen;
	uint32_t magichdr[3];
	int num_env_found = 0;
	char * config;

	/* Search loader in memory for env headers */
	for (off = 0; off < (loff_t)env_boot_magic_search_size(); off += 4096) {
		rdlen = 12;
		memcpy((char*)magichdr, (char*)loader_addr + off, rdlen);
		if (magichdr[0] != BOOT_MAGIC_MAGIC) {
			continue;
		} else {
			/* Verify CRC */
			rdlen = magichdr[1];
			ep = (env_t *) ((char*)loader_addr + off + 8);
			calc_crc = the_env_crc32(0, ep->data, rdlen - 4);
			if( ep->crc == calc_crc ) {
				num_env_found++;
			} else {
				continue;
			}
		}

		/* Create synced env blob */
		if(num_env_found == 1)
		{
			/* Export environment to embed in new loader */
			envbuf = malloc(max(rdlen + 12, (size_t)(CONFIG_ENV_SIZE + 12)));
			if( !envbuf ) {
				printf("ERROR: Failed to allocate mem for env!\n");
				ret = -1;
				break;
			}
			ep = (env_t *) (envbuf + 8);
			envintp = (uint32_t *) envbuf;

			/* Delete boot magic string before exporting env */
			config = env_get("env_boot_magic");
			env_set("env_boot_magic", NULL);
			ret = env_export(ep);
			if( ret ) {
				printf("ERROR: Failed to export env!\n");
				env_set("env_boot_magic", config);
				break;
			}

			for (i = CONFIG_ENV_SIZE; i < rdlen; i++) {
				envbuf[12 + i] = 0xff;
			}
			calc_crc = the_env_crc32(0, ep->data, rdlen - 4);
			memcpy(&ep->crc, &calc_crc, sizeof(calc_crc));
			envintp[0] = BOOT_MAGIC_MAGIC;
			envintp[1] = rdlen;
		}

		/* Write synched env back to loader binary */
		printf("Updating env in loader bin at 0x%lx\n", (long)off);
		memcpy((char*)loader_addr + off, envbuf, rdlen+12);
	}

	if(envbuf)
		free(envbuf);

	if(!num_env_found)
		ret = -1;

	return ret;
}

__weak char * get_loader_media(void)
{
	int node,len;
	char *media = NULL;

	if(forced_updates && strlen(forced_boot_media)) {
		media = forced_boot_media;
		printf("WARNING: forced_updates == 1, forcing boot media to %s!\n", media);
	} else {
		node = fdt_path_offset(gd->fdt_blob, "/chosen");
		if (node < 0) {
			printf("ERROR: Can't find /chosen node in cboot DTB! Cannot determine boot media!\n");
			return media;
		}
		media = (char*)(fdt_getprop(gd->fdt_blob, node, "boot_device", &len));
	}
	return media;
}

static int strcasecmp_image_media(const char *s)
{
	char *media = NULL;
	int ret = -2;
	media = get_image_media();
	if (NULL != media) {
		ret = strcasecmp(media, s);
		free(media);
	}
	return(ret);
}

static char * get_image_media(void)
{
	char *cp = NULL;
	char *media = NULL;

	if(forced_updates && strlen(forced_image_media)) {
		/* strdup to allocate memory, mirror non forced case */
		media = strdup(forced_image_media);
		printf("WARNING: forced_updates == 1, forcing image media to %s!\n", media);
	} else {
		cp = env_get("IMAGE");
		if (NULL != cp)
		{
			unsigned long iargs[4];
			char units[4];
			int n = parse_env_string_plus_nums(cp, &media, 4, iargs, units);
			if( !n )
				printf("ERROR: Can't parse IMAGE env parameter! Cannot determine image media!\n");
		}
		else
			printf("ERROR: Can't find IMAGE env parameter! Cannot determine image media!\n");
	}

	return media;
}
#endif

static int flash_rootfs( ulong addr, ulong size , int img_index)
{
	int ret = -1;

#ifdef CONFIG_BCMBCA_VFBIO
	ret = flash_rootfs_vfbio(addr, size, img_index);
#else
	if( strcasecmp_image_media(FLASH_DEV_STR_NAND) == 0 ) {
#ifdef BCA_SDK_NAND_CMD
		ret = flash_rootfs_nand(addr, size, img_index);
#endif
	}

	if( ret && ( strcasecmp_image_media(FLASH_DEV_STR_EMMC) == 0 ) ) {
#ifdef BCA_SDK_EMMC_CMD
		ret = flash_rootfs_emmc(addr, size, img_index);
#endif
	}

	if( ret && ( strcasecmp_image_media(FLASH_DEV_STR_SPINOR) == 0 ) ) {
#ifdef BCA_SDK_SPINOR_CMD
	ret = write_spinor_partition(ROOTFS_PART, addr, size);
#endif
	}
#endif

	if( ret )
		printf("ERROR: Failed to flash bootfs binary!\n");

	return ret;

}
static int flash_bootfs( ulong addr, ulong size , int img_index)
{
	int ret = -1;

#ifdef CONFIG_BCMBCA_VFBIO
	ret = flash_bootfs_vfbio(addr, size, img_index);
#else
	if( strcasecmp_image_media(FLASH_DEV_STR_NAND) == 0 ) {
#ifdef BCA_SDK_NAND_CMD
		ret = flash_bootfs_nand(addr, size, img_index);
#endif
	}

	if( ret && ( strcasecmp_image_media(FLASH_DEV_STR_EMMC) == 0 ) ) {
#ifdef BCA_SDK_EMMC_CMD
		ret = flash_bootfs_emmc(addr, size, img_index);
#endif
	}

	if( ret && ( strcasecmp_image_media(FLASH_DEV_STR_SPINOR) == 0 ) ) {
#ifdef BCA_SDK_SPINOR_CMD
		ret = write_spinor_partition(BOOTFS_PART, addr, size);
#endif
	}
#endif

	if( ret )
		printf("ERROR: Failed to flash bootfs binary!\n");

	return ret;
}
#ifndef CONFIG_BCMBCA_VFBIO
static int flash_loader( ulong addr, ulong size , int img_index)
{
	int ret = -1;

	/* Synch uboot env to loader binary */
	if( disable_runtime_env_sync ) {
		printf("WARNING: Not synching runtime env to loader board_spl_fit_pre_load!\n");
	} else {
		if( sync_update_loader_bin_env(addr) ) {
			printf("ERROR: Could not sync runtime env to loader bin!\n");
			return ret;
		}
	}

	if( strcasecmp(get_loader_media(), FLASH_DEV_STR_NAND) == 0 ) {
#ifdef BCA_SDK_NAND_CMD
		ret = flash_loader_nand(addr, size, img_index);
#endif
	}

	if( ret && ( strcasecmp(get_loader_media(), FLASH_DEV_STR_EMMC) == 0 ) ) {
#ifdef BCA_SDK_EMMC_CMD
		ret = flash_loader_emmc(addr, size, img_index);
#endif
	}

	if( ret && ( strcasecmp(get_loader_media(), FLASH_DEV_STR_SPINOR) == 0 ) ) {
#ifdef BCA_SDK_SPINOR_CMD
		ret = flash_loader_spinor(addr, size);
#endif
	}

	if( disable_runtime_env_sync ) {
		printf("WARNING: Not updating runtime env from flashed loader!\n");
		disable_runtime_env_sync = 0;
	} else {
		/* Since sync was successful, we need to scan loader
		 * for boot magic and update runtime env */
		env_relocate();
		env_set("fileaddr", NULL);
		env_save();
	}

	if( ret )
		printf("ERROR: Failed to flash loader binary!\n");

	return ret;
}

static int set_metadata_val( int * committed, int * valid, int * seq )
{
	char* b;
	uint32_t *d, crc;
	env_t * ep;
	int i;
	int ret = -1;

	b = malloc(MAX_METADATA_SIZE);
	d = (uint32_t *) b;
	ep = (env_t *) & d[2];
	d[0] = METADATA_SIZE;
	d[1] = METADATA_SIZE;
	i =     sprintf((char*)ep->data,     "COMMITTED=%d",*committed) + 1;
	i = i + sprintf((char*)&ep->data[i], "VALID=%d,%d",valid[0],valid[1]) + 1;
	i = i + sprintf((char*)&ep->data[i], "SEQ=%d,%d",seq[0],seq[1]) + 1;
	ep->data[i] = '\0';
	crc = the_env_crc32(0, ep->data, (d[0] - 4) & 0xffff);
	/*
	 * We can't use plain crc32 because someone redefines it??
	 * crc = crc32(0, ep->data, (d[0] - 4) & 0xffff);
	 */
	memcpy(&ep->crc, &crc, sizeof(crc));

	/* Flash metadata */
	if( strcasecmp_image_media(FLASH_DEV_STR_NAND) == 0 ) {
#ifdef BCA_SDK_NAND_CMD
		ret = set_nand_metadata(b, METADATA_SIZE + 16);
#endif
	}

	if( ret && ( strcasecmp_image_media(FLASH_DEV_STR_EMMC) == 0 ) ) {
#ifdef BCA_SDK_EMMC_CMD
		ret = set_emmc_metadata(b, METADATA_SIZE + 16);
#endif
	}

	printf("Setting committed %d valid %d,%d seq %d,%d\n", *committed, valid[0], valid[1], seq[0], seq[1]);
	free(b);
	return ret;
}

static int get_metadata_val( int * committed, int * valid, int * seq )
{
	char *b = malloc(MAX_METADATA_SIZE);
	int ret = CMD_RET_FAILURE;

	/* Read metadata from flash */
	if( strcasecmp_image_media(FLASH_DEV_STR_NAND) == 0 ) {
#ifdef BCA_SDK_NAND_CMD
		ret = get_nand_metadata( b, MAX_METADATA_SIZE);
#endif
	}

	if( ret && ( strcasecmp_image_media(FLASH_DEV_STR_EMMC) == 0 ) ) {
#ifdef BCA_SDK_EMMC_CMD
		ret = get_emmc_metadata(b, MAX_METADATA_SIZE);
#endif
	}

	/* Validate metadata */
	ret = validate_metadata(b, valid, committed, seq);
	free(b);
	printf("Getting committed %d valid %d,%d seq %d,%d\n", *committed, valid[0], valid[1], seq[0], seq[1]);
	return ret;
}

#else
static int get_metadata_val(int *committed, int *valid, int *seq)
{
	char *p;
	unsigned long iargs[3];
	int n, i;
	
	n = parse_env_nums(env_get("VALID"), 3, iargs, NULL);
	if (!n) {
		return 1;
	}
	for (i = 0; i < n; i++) {
		if ((iargs[i] >= 1) && (iargs[i] <= 2)) {
			valid[iargs[i] - 1] = iargs[i];
		}
	}
	
	p = env_get("COMMITTED");
	if (!p) {
		return 1;
	}
	*committed = (*p == '1') ? 1 : (*p == '2') ? 2 : 0;

	// preset the sequence numbers incase the field doesn't exist
	if ((*committed == 1) && valid[0])
	{
		seq[0] = 1;
		if (valid[1])
			seq[1] = 0;
	}
	else if ((*committed == 2) && valid[1])
	{
		seq[1] = 1;
		if (valid[0])
			seq[0] = 0;
	}

	n = parse_env_nums(env_get("SEQ"), 3, iargs, NULL);
	if (!n) {
		return 1;
	}
	for (i = 0; i < n; i++) {
		seq[i] = iargs[i];
	}

	printf("Getting committed %d valid %d,%d seq %d,%d\n", *committed, valid[0], valid[1], seq[0], seq[1]);
	return 0;
}

static int set_metadata_val(int *committed, int *valid, int *seq)
{
	char val[100];
	
	env_set_ulong("COMMITTED", *committed);
	sprintf(val, "%d,%d", valid[0],valid[1]);
	env_set("VALID", val);
	sprintf(val, "%d,%d", seq[0],seq[1]);
	env_set("SEQ", val);
	env_save();
	
	return 0;
}
#endif

static int set_img_status( int img_index, int valid_status, int commit_status )
{
	int committed = 0;
	int valid[2] = {0,0};
	int seq[2] = {-1,-1};

#ifndef CONFIG_BCMBCA_VFBIO
	/*Return directly for SPI Nor*/
	if( strcasecmp_image_media(FLASH_DEV_STR_SPINOR) == 0)
		return 0;
#endif

	/* Get current metadata */
	get_metadata_val(&committed, valid, seq);

	/* update commit status if required */
	if( commit_status == IMG_STAT_UNCOMMIT)
		committed = 0;
	else if( commit_status == IMG_STAT_COMMIT)
		committed = img_index;

	/* update valid status and sequence number  */
	valid[img_index-1] = (valid_status==IMG_STAT_VALID?img_index:0);
	seq[img_index-1] = (seq[(img_index == 1) ? 1 : 0] + 1) % 1000; // set newly written image sequence number one greater than other image

	/* Set committed image */
	set_metadata_val(&committed, valid, seq);

	return 0;
}

int set_img_valid_status( int img_index )
{
	set_img_status(img_index, IMG_STAT_VALID, IMG_STAT_NOCOMMIT);
	return 0;
}

int commit_image( int img_index )
{
	set_img_status(img_index, IMG_STAT_VALID, IMG_STAT_COMMIT);
	return 0;
}

int get_img_index_for_upgrade(int flag)
{
	int committed = 0;
	int valid[2] = {0,0};
	int seq[2] = {-1,-1};
	(void) flag;
	int img_index = get_active_img_idx();

#ifndef CONFIG_BCMBCA_VFBIO
	/*Return 1 for SPI Nor*/
	if( strcasecmp_image_media(FLASH_DEV_STR_SPINOR) == 0 )
		return 1;
#endif

	/* If we know the active image, always write to the inactive image */
	if( img_index ) {
		return( (img_index==1)?2:1 );
	}

	/* If we cannot determine active image, then get non committed image */
	get_metadata_val(&committed, valid, seq);
	if( !committed ) {
		/* If no commited images then we will flash to the 1st img index */
		return 1;
	} else {
		/* If we have a committed image, then we will upgrade the uncommitted one */
		return (committed == 1? 2: 1);
	}
}

#ifndef CONFIG_LOAD_FIT_OFFSET
#define CONFIG_LOAD_FIT_OFFSET	SZ_16M
#endif

static int load_linux_img( int flag, int argc, char *const argv[])
{
	char cmd[128];
	const void* fdt = gd->fdt_blob;
	char *board_id  = NULL;
	int img_index = 0;
	unsigned long bootfs_load_addr = load_addr + CONFIG_LOAD_FIT_OFFSET;
	int ret = CMD_RET_FAILURE;

	if (fdt_getprop_u32_default(fdt, "/chosen", "safemode", 0)) {
		printf("Booting to Linux disabled in safe mode!!\n");
		return CMD_RET_FAILURE;
	}

	if( argc == 1 ) {
		img_index = get_active_img_idx();
	} else if( argc > 1 ) {
		if( strlen(argv[1]) == 1 ) {
			img_index = *argv[1] - '0';
			if (!img_index) { // boot currently committed image
				int committed = 0;
				int valid[2] = {0,0};
				int seq[2] = {-1,-1};

				get_metadata_val(&committed, valid, seq);
				img_index = committed;
			}
		}
		else {
			board_id = argv[1];
			img_index = get_active_img_idx();
		}

		if( argc > 2 ) {
			board_id = argv[2];
		}
	}

	if( img_index < MIN_IMG_INDEX || img_index > MAX_IMG_INDEX ) {
		printf("ERROR: Invalid Image Index specified!\n");
		return ret;
	}

	/* Load bootfs to load address */
#ifdef CONFIG_BCMBCA_VFBIO
	ret = vfbio_load_bootfs(img_index, bootfs_load_addr);
#else
	if( strcasecmp_image_media(FLASH_DEV_STR_NAND) == 0 ) {
#ifdef BCA_SDK_NAND_CMD
		ret = nand_load_bootfs(img_index, bootfs_load_addr);
#endif
	}

	if( ret && ( strcasecmp_image_media(FLASH_DEV_STR_EMMC) == 0 ) ) {
#ifdef BCA_SDK_EMMC_CMD
		ret = emmc_load_bootfs(img_index, bootfs_load_addr);
		ret = emmc_do_gpt_fixup();
#endif
	}

	if( ret && ( strcasecmp_image_media(FLASH_DEV_STR_SPINOR) == 0 ) ) {
#ifdef BCA_SDK_SPINOR_CMD
		ret = spinor_load_bootfs(bootfs_load_addr);
#endif
	}
#endif

	if( ret == 0 ) {
		if(!board_id)
			board_id = env_get("boardid");

		/* Load binaries */
		sprintf(cmd, "/configurations/conf_lx_%s", board_id);
		if(board_id && (fdt_path_offset((void *)bootfs_load_addr, cmd) >= 0))
			sprintf(cmd, "bootm start %lx#conf_lx_%s; bootm loados; bootm prep;", (unsigned long)bootfs_load_addr, board_id);
		else
			sprintf(cmd, "bootm start %lx#conf_linux; bootm loados; bootm prep;", (unsigned long)bootfs_load_addr);

      		run_command(cmd, 0);
	} else {
		printf("ERROR: Failed to load bootfs%d!\n", img_index);
	}

	return ret;
}

static int do_load(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	int ret = CMD_RET_SUCCESS;
	if( load_linux_img( flag, argc, argv ) == 0 ) {
		printf("\nAll Image components have been loaded to DDR:\n");
		printf(" - To edit FDT: 'fdt <fdt cmds> ..'\n");
		printf(" - To launch Linux: 'bootm go'\n");
	} else {
		printf("Linux image loading Failed!\n");
		ret = CMD_RET_FAILURE;
	}

	return ret;
}

static int do_boot(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	int ret;
	if( load_linux_img( flag, argc, argv ) == 0 )
		ret = run_command("bootm go\n", 0);
	else {
		printf("Linux image booting Failed!\n");
		ret = CMD_RET_FAILURE;
	}

	return ret;
}

#if !defined(CONFIG_BCMBCA_VFBIO)
static int do_flash_loader(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	char cmd[128]={'\0'};
	const char *chip_num;
	ulong size;
	ulong addr;
	int ret = CMD_RET_SUCCESS;

	/* Determine chip number */
	chip_num = strstr(CONFIG_SYS_SOC, "bcm");
	chip_num += strlen("bcm");

	if( strcasecmp(get_loader_media(), FLASH_DEV_STR_NAND) == 0 ) {
		sprintf(cmd, "tftp %lx loader_test_nand_%s.bin\n", load_addr, chip_num);
	}

	if( ( strcasecmp(get_loader_media(), FLASH_DEV_STR_EMMC) == 0 ) ) {
		sprintf(cmd, "tftp %lx loader_test_emmc_%s.bin\n", load_addr, chip_num);
	}

	if( ( strcasecmp(get_loader_media(), FLASH_DEV_STR_SPINOR) == 0 ) ) {
		sprintf(cmd, "tftp %lx loader_test_spinor_%s.bin\n", load_addr, chip_num);
	}

	if( run_command(cmd, 0) == 0 ) {
		size = env_get_hex("filesize", 0);
		addr = env_get_hex("fileaddr", 0);
		flash_loader(addr, size, 0);
	} else {
		printf("ERROR!!: Failed to tftp loader binary!!\n");
		ret = CMD_RET_FAILURE;
	}

	return ret;
}
#endif

#if defined(BCA_SDK_NAND_CMD) || defined(BCA_SDK_EMMC_CMD)
static int do_flash_bootfs_rootfs( char * bootfs_filename, char * rootfs_filename, int img_index )
{
	char cmd[128];
	ulong size;
	ulong addr;
	int ret = CMD_RET_FAILURE;

	/* Write bootfs */
	if(bootfs_filename) {
		sprintf(cmd, "tftp %lx %s", load_addr, bootfs_filename);
		if( run_command(cmd, 0) == 0 ) {
			size = env_get_hex("filesize", 0);
			addr = env_get_hex("fileaddr", 0);
			ret = flash_bootfs(addr, size, img_index);
		} else
			printf("ERROR!!: Failed to tftp FIT image!!\n");
	}

	/* Write rootfs */
	if(rootfs_filename) {
		sprintf(cmd, "tftp %lx %s\n", load_addr, rootfs_filename);

		if( run_command(cmd, 0) == 0 ) {
			size = env_get_hex("filesize", 0);
			addr = env_get_hex("fileaddr", 0);
			ret = flash_rootfs(addr, size, img_index);
		} else
			printf("ERROR!!: Failed to tftp rootfs binary!!\n");
	}

	return ret;
}

static int do_flash_bins(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	char * rootfs_fname = NULL;
	char * bootfs_fname = NULL;
	int img_index = 0;
	int ret = CMD_RET_FAILURE;

	/* Parse arguments */
	if ( (strcmp(argv[0], "flash_bootfs_raw") == 0) ) {
		bootfs_fname = argv[1];
	}
	if( (strcmp(argv[0], "flash_rootfs_raw") == 0) ) {
		rootfs_fname = argv[1];
	}

	if( argc >= 2 ) {
		if( (argc == 3) && (strlen(argv[2]) == 1) )
			img_index = *argv[2] - '0';
		else
			img_index = get_img_index_for_upgrade(0);
	} else {
		return CMD_RET_USAGE;
	}

	ret = do_flash_bootfs_rootfs(bootfs_fname, rootfs_fname, img_index);

	if( !ret )
		set_img_valid_status( img_index );

	return ret;
}
#endif

static int verify_compat_string( const char * compat_str )
{
	const char *chip_num;
	char * flash_type;
	char * token = NULL;
#ifdef BCA_SDK_NAND_CMD
	char flash_type_full[20];
 	struct mtd_info *mtd = NULL;
#endif

	/* Early return if forced updates are enabled */
	if( forced_updates ) {
		printf("WARNING: Skipping all compatibility checks due to forced updates being enabled!\n");
		return 0;
	}

	/* Determine chip number */
	chip_num = strstr(CONFIG_SYS_SOC, "bcm");
	chip_num += strlen("bcm");

	/* Determine flash type matches*/
#ifdef CONFIG_BCMBCA_VFBIO
	flash_type = strdup("vflash");
#else
	flash_type = get_image_media();
#endif

	if( strcmp(flash_type, FLASH_DEV_STR_NAND) == 0 ) {
#ifdef BCA_SDK_NAND_CMD
	  mtd = bcmbca_get_image_mtd_device();
		if (!mtd) {
			printf("ERROR: Cannot determine NAND erase block size!\n");
			free(flash_type);
			return CMD_RET_FAILURE;
		}
		sprintf(flash_type_full, "%s%d", flash_type, mtd->erasesize >> 10 );
#endif
	}

	/* Check compat string */
	token = strtok((char *)compat_str,";");
	while( token ) {
		if( strstr(token, "chip=")) {
			token += strlen("chip=");
			if( strcasecmp(token, chip_num) ) {
				printf("ERROR: Img bundle is for %s, current chip is %s\n", token, chip_num);
				free(flash_type);
				return -1;
			}
		} else if( strstr(token, "flash=")) {
			token += strlen("flash=");
			if( strcasecmp(token, flash_type) ) {
				if( strcmp(flash_type, FLASH_DEV_STR_NAND) == 0 ) {
#ifdef BCA_SDK_NAND_CMD
					if( strcasecmp(token, flash_type_full) )
					{
						printf("ERROR: Img bundle is for %s, current flash is %s\n", token, flash_type_full);
						free(flash_type);
						return -1;
					}
#endif
				} else {
					printf("ERROR: Img bundle is for %s, current flash is %s\n", token, flash_type);
					free(flash_type);
					return -1;
				}
			}
		}
		token = strtok(NULL,";");
	}
	free(flash_type);
	return 0;
}

#ifndef CONFIG_BCMBCA_VFBIO
static int update_flash_parts_from_loader_bin( ulong loader_addr, ulong loader_size ) {
	env_t *ep=NULL;
	uint32_t calc_crc;
	int ret = -2;
	loff_t off;
	size_t rdlen;
	uint32_t magichdr[3];
	char cmd[128];

	/* Search loader in memory for env headers */
	for (off = 0; off < (loff_t)env_boot_magic_search_size(); off += 4096) {
		/* Search loader bin for environment */
		rdlen = 12;
		memcpy((char*)magichdr, (char*)loader_addr + off, rdlen);
		if (magichdr[0] != BOOT_MAGIC_MAGIC) {
			continue;
		} else {
			/* Verify CRC */
			rdlen = magichdr[1];
			ep = (env_t *) ((char*)loader_addr + off + 8);
			calc_crc = the_env_crc32(0, ep->data, rdlen - 4);
			if( ep->crc == calc_crc ) {
				break;
			} else {
				continue;
			}
		}
	}

	/* Set runtime env variables based on loader's env */
	if( off < (loff_t)env_boot_magic_search_size() ) {

		/* read IMAGE variable */
		sprintf(cmd, "env import %lx - IMAGE", (unsigned long)(char*)ep);
		run_command(cmd, 0);

		/* Verify */
		if( env_get("IMAGE") == NULL ) {
			printf("ERROR! Failed to import IMAGE parameter from loader!\n");
			return ret;
		}

		/* read default_partitions */
		if( ( strcasecmp_image_media(FLASH_DEV_STR_EMMC) == 0 ) ) {
			sprintf(cmd, "env import %lx - default_partitions", (unsigned long)(char*)ep);
			run_command(cmd, 0);

			/* Verify */
			if( env_get("default_partitions") == NULL ) {
				printf("ERROR! Failed to import default_partitions parameter from loader!\n");
				return ret;
			}
		}

		/* Call partition init code */
		printf("INFO: flash partition config updated. Repartitioning flash!\n");
		ret = board_init_flash_parts(1);
	} else {
		printf("ERROR!: No environment found in loader binary!\n");
	}
	return ret;
}
#endif

int flash_upgrade_img_bundle( ulong bundle_addr , int img_index, const char * conf_name)
{
	char path[128];
	ulong addr=0;
	ulong size=0;
	ulong loader_addr=0;
	ulong loader_size=0;
	int i=0;
	int err = 0;
	char * bin_name;
	char * loader_bin_name;
	const char * compat_str;
	char *compat_copy;
	int nodeoffset = 0;
	int conf_nodeoffset = 0;
	int max_entries = sizeof(fn_table)/sizeof(flashfn_table_entry);
	int len;

	/* verify hashes of all upgrade bundle contents */
	if( fit_all_image_verify((void *)bundle_addr) ) {

		/* Get offset of config node */
		sprintf(path, "/configurations");
		nodeoffset = fdt_path_offset((void *)bundle_addr, path);
		if( nodeoffset < 0 ) {
			printf("ERROR: %s node not found in bundle!\n", path);
			return CMD_RET_FAILURE;
		}

		/* If configuration is specifed then use it */
		if( !conf_name ) {
			/* Get name of the default configuration */
			conf_name = fdt_getprop((void *)bundle_addr, nodeoffset, "default", &len);
			if( !conf_name ) {
				printf("ERROR: Default configuration not found in bundle!\n");
				return CMD_RET_FAILURE;
			}
		}

		/* Get offset of selected configurations fit node */
		sprintf(path, "/configurations/%s", conf_name);
		conf_nodeoffset = fdt_path_offset((void *)bundle_addr, path);
		if( conf_nodeoffset < 0 ) {
			printf("ERROR: %s node not found in bundle!\n", conf_name);
			return CMD_RET_FAILURE;
		}

		/* Verify compatibility */
		compat_str = fdt_getprop( (void *)bundle_addr, conf_nodeoffset, "compatible", &len);
		if( !compat_str ) {
			printf("Error: 'compatible' node not found in bundle!\n");
			return CMD_RET_FAILURE;
		}

		compat_copy = malloc(strlen(compat_str)+1);
		if(!compat_copy)
		{
			printf("Error allocating memory for compatibility string!\n");
			return CMD_RET_FAILURE;
		}
		strcpy(compat_copy, compat_str);

		if( verify_compat_string(compat_copy) ) {
			printf("Error: Bundle is not compatible with platform!\n");
			free(compat_copy);
			return CMD_RET_FAILURE;
		}
		free(compat_copy);

#ifndef CONFIG_BCMBCA_VFBIO
		/* Decide whether we want to reformat partitions based on loader
		 * binaries partition configuration configuration */
		if( NULL == env_get("IMAGE") ) {
			printf("WARNING: IMAGE env variable missing! Using IMAGE from loader binary!\n");
			if( get_binary_from_bundle(bundle_addr, conf_name, "loader",
			   &loader_bin_name, &loader_addr, &loader_size) == 0 ) {
				printf("INFO: Reformatting flash media based on new loader binary\n");
				if( update_flash_parts_from_loader_bin( loader_addr, loader_size ) ) {
					printf("ERROR: Failed to reformat flash based loader binary!\n");
					return CMD_RET_FAILURE;
				}

				/* Set flag to prevent sync of run-time env to loader bin */
				disable_runtime_env_sync = 1;
			} else {
				printf("ERROR! No loader binary AND IMAGE parameter not set!\n");
				return CMD_RET_FAILURE;
			}
		} else if (env_get_hex("reformatOnUpgrade", 0)) {
			printf("INFO: Reformatting partitions because reformatOnUpgrade == 1!\n");
			err = board_init_flash_parts(1);
			env_set("reformatOnUpgrade", NULL);
		}
#endif

		for( i=0; i<max_entries && !err; i++ ) {
			/* Parse loader only if it has not been parsed before */
			if( (strcmp(fn_table[i].name,"loader") == 0 ) && loader_addr && loader_size ) {
				bin_name = loader_bin_name;
				size = loader_size;
				addr = loader_addr;
			} else {
				if (get_binary_from_bundle( bundle_addr, conf_name, fn_table[i].name,
				   &bin_name, &addr, &size ) ) {
					continue;
				}
			}

			/* Calling flashing function */
#ifdef CONFIG_BCMBCA_VFBIO
			printf("Flashing %s (%s: 0x%lx bytes from 0x%lx) to %s",
				fn_table[i].name, bin_name, size, addr, fn_table[i].name);
			if(i < 5)
				printf("%d LUN.....\n", img_index);
			else
				printf(" LUNs.....\n");
#else
			printf("Flashing %s (%s: 0x%lx bytes from 0x%lx) to %s%d.....\n",
			   fn_table[i].name, bin_name, size, addr, fn_table[i].name,
			   (strcmp(fn_table[i].name,"loader")?img_index:0));
#endif
			err = fn_table[i].func(addr, size, img_index);
		}

		/* Reset flag */
		disable_runtime_env_sync = 0;
	} else {
		err = CMD_RET_FAILURE;
	}

	return err;
}

static int do_flash_upgrade_img (cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	char cmd[128];
	ulong bundle_addr=0;
	int err = CMD_RET_FAILURE;
	const char * conf_name = NULL;
	int img_index = 0;
	bool download = true;
	bool commit = true;
	int active_img_idx = get_active_img_idx();

	/* Parse and remove the optional skip argument */
	if ( argc > 1) {
		if ( !strcmp(argv[1], "-s")) {
			download = false;
			argc--;
			argv++;
		} else if ( !strcmp(argv[1], "-i")) {
			commit = false;
			argc--;
			argv++;
		}
	}

	if( argc == 2 ) {
		img_index = get_img_index_for_upgrade(0);
	} else if( argc > 2 ) {
		if( strlen(argv[2]) == 1 )
			img_index = *argv[2] - '0';
		else {
			conf_name = argv[2];
			img_index = get_img_index_for_upgrade(0);
		}

		if( argc > 3 ) {
			conf_name = argv[3];
		}
	}

	if( img_index < MIN_IMG_INDEX || img_index > MAX_IMG_INDEX ) {
		printf("ERROR: Invalid Image Index specified!\n");
		return CMD_RET_USAGE;
	}

	if (download)
		sprintf(cmd, "tftp %lx %s", load_addr, argv[1]);

	if( !download || run_command(cmd, 0) == 0 ) {
		bundle_addr = env_get_hex("fileaddr", 0);

		if (bundle_addr)
			/* Call main img flashing function */
			err = flash_upgrade_img_bundle(bundle_addr, img_index, conf_name);
	}

	if(err) {
		printf("ERROR: Image upgrade failed!!\n");
	} else {
		printf("INFO: Image upgrade successfull!!\n");
		if( commit ) {
			printf("INFO: Committing Image!!\n");
			commit_image( img_index );

			/* Handle bootstrap condition */
			if( active_img_idx == ACTIVE_IMGIDX_BOOTSTRAP ) {
				/* Now that we have a valid image flashed, change active image index */
				set_active_img_idx(img_index);
			}
		} else {
			printf("INFO: Not Committing Image!!\n");
			set_img_valid_status( img_index );
		}
	}

	return err;
}

static int do_restoredefault(cmd_tbl_t * cmdtp, int flag, int argc,
			   char *const argv[])
{
	int ret = -1;

#ifdef CONFIG_BCMBCA_VFBIO
	int id;
	ret = vfbio_lun_get_id("data", &id);
	if(!ret)
	{
		uint blk_size = 0;
		uint64_t lun_size = 0;

		// overwrite lun data to avoid ext4 emulation in subsequently created lun 
		vfbio_lun_get_size(id, &lun_size);
		vfbio_lun_get_blk_size(id, &blk_size);
		vfbio_lun_write(id, 0, lun_size/blk_size, (void *)load_addr);
		ret = vfbio_lun_delete(id);
	}
	else
	{
		printf("No 'data' LUN found, probably no configuration was done yet or restore defaults was already performed.\n");
		ret = 0;
	}
#else
	if( strcasecmp_image_media(FLASH_DEV_STR_NAND) == 0 ) {
#ifdef BCA_SDK_NAND_CMD
		ret = nand_restoredefault();
#endif
	}

	if( ret && ( strcasecmp_image_media(FLASH_DEV_STR_EMMC) == 0 ) ) {
#ifdef BCA_SDK_EMMC_CMD
		ret = emmc_restoredefault();
#endif
	}

	if( ret && ( strcasecmp_image_media(FLASH_DEV_STR_SPINOR) == 0 ) ) {
#ifdef BCA_SDK_SPINOR_CMD
		ret = spinor_restoredefault();
#endif
	}
#endif

	return ret;
}

#if defined(BCA_SDK_NAND_CMD) || defined(BCA_SDK_EMMC_CMD)  || defined(CONFIG_BCMBCA_VFBIO)
static int do_metadata(cmd_tbl_t * cmdtp, int flag, int argc,
			   char *const argv[])
{
	int ret = -1;
	int committed = 0;
	int valid[2] = {0,0};
	int seq[2] = {-1,-1};
	int status = get_metadata_val(&committed, valid, seq);

	if (argc == 1) {
		if (0 == status) { // show image commit, valid and sequence metadata
			printf("committed-img: %d valid-imgs: ", committed);
			if( valid[0] ) {
				printf("%d", valid[0]);
				if( valid[1] )
					printf(" & ");
			}
			if( valid[1] )
				printf("%d", valid[1]);
			printf(", seq# img 1: %d, img2: %d\n\n", seq[0], seq[1]);

			env_set_hex("metadata_committed", committed);

			ret = 0;
		} else {
			printf("metadata parse error\n");
		}
	} else if (argc == 2) { // set commit image only
		committed = atoi(argv[1]);

		if( ( committed < 0 || committed > 2 ) ||
	            ( valid[0] < 0 || valid[0] > 2 ) ||
		    ( valid[1] < 0 || valid[1] > 2 ) ||
		    ( ( committed != valid[0] ) && ( committed != valid[1] ) ) ||
		    (0 != set_metadata_val(&committed, valid, seq)) ) {
			printf("metadata parse error! Valid value for commited image is 1,2 and image must be valid\n");
		} else {
			ret = 0;
		}
	} else if (argc == 3) { // set commit and valid image
		committed = atoi(argv[1]);
		valid[0] = atoi(strtok(argv[2],","));
		valid[1] = atoi(strtok(NULL,","));

		if( ( committed < 0 || committed > 2 ) ||
	            ( valid[0] < 0 || valid[0] > 2 ) ||
		    ( valid[1] < 0 || valid[1] > 2 ) ||
		    (0 != set_metadata_val(&committed, valid, seq)) ) {
			printf("metadata parse error! Valid values for commit and valid flags are: 0,1,2\n");
		} else {
			ret = 0;
		}
	}
	return ret;
}
#endif

#if defined(CONFIG_BCM_BOOTSTATE)
static int do_activate(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	bcmbca_set_boot_reason(BCM_BOOT_REASON_ACTIVATE);
	run_command("reset", 0);
	return 0;
}
#if defined(CONFIG_BCM_BOOTSTATE_FALLBACK_SUPPORT)
static int do_check_fallback(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	int failed_count=0;
	int max_fallback_count = atoi(argv[1]);

	if(max_fallback_count > MAX_BOOT_FAILED_COUNT || max_fallback_count <= 0)
		max_fallback_count = MAX_BOOT_FAILED_COUNT;

	/* when the reset happend due to any reason other than SW, reset the clear failed count */
	if(!(bcmbca_get_reset_status() & SW_RESET_STATUS))
		bcmbca_clear_boot_failed_count();
	else
	{
		failed_count=bcmbca_get_boot_failed_count();
		printf("do_check_fallback: current failed count %d max %d\n", failed_count, max_fallback_count);
		//we are here because the last image crashed
		if((bcmbca_get_boot_reason() & BCM_BOOT_PHASE_MASK) == BCM_BOOT_PHASE_FB_UBOOT)
		{
			if(failed_count < max_fallback_count)
			{
				printf("do_check_fallback: failed count is still below threshold %d, resetting...\n", failed_count);
				bcmbca_set_boot_failed_count(failed_count+1);
				run_command("reset", 0);
			}
			else
			{
				printf("do_check_fallback: Continue..with FB image\n");
			}
		}
	}

	return 0;
}
#endif
#endif

#if !defined(CONFIG_BCMBCA_VFBIO)
static int do_force(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;
	if( argc > 1 ) {
		if( atoi(argv[1]) == 1 ) {
			if( (argc == 4)
				&& ( ( strcasecmp(argv[2], FLASH_DEV_STR_NAND) == 0 )
				|| (   strcasecmp(argv[2], FLASH_DEV_STR_SPINOR) == 0 )
				|| (   strcasecmp(argv[2], FLASH_DEV_STR_EMMC) == 0 ) )
				&& ( ( strcasecmp(argv[3], FLASH_DEV_STR_NAND) == 0 )
				|| (   strcasecmp(argv[3], FLASH_DEV_STR_SPINOR) == 0 )
				|| (   strcasecmp(argv[3], FLASH_DEV_STR_EMMC) == 0 ) )	) {
				forced_updates = 1;
				memset(forced_boot_media, '\0', sizeof(forced_boot_media));
				memset(forced_image_media, '\0', sizeof(forced_image_media));
				strncpy(forced_boot_media,argv[2], sizeof(forced_boot_media)-1);
				strncpy(forced_image_media,argv[3], sizeof(forced_image_media)-1);
			} else if(argc == 2) {
				forced_updates = 1;
			} else {
				ret = CMD_RET_USAGE;
				printf("Cannot determine boot/image flash type, please specify flash types!\n");
			}
		} else {
			forced_updates = 0;
			forced_boot_media[0] = '\0';
			forced_image_media[0] = '\0';
		}
	}

	printf("Forced Image Updates: %s, Forced flash types: Boot=%s, Image=%s\n",
		(forced_updates?"Enabled":"Disabled"),
		(forced_updates?forced_boot_media:"NOT SPECIFIED"),
		(forced_updates?forced_image_media:"NOT SPECIFIED"));

	return ret;
}
#endif

#ifdef CONFIG_BCMBCA_HTTPD
static int do_httpd_start(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
    if(!httpd_check_net_env())
        register_cli_job_cb(0, http_poll);
    return 0;
}
#endif

#ifdef CONFIG_BCMBCA_PMC
static int do_bpcm_cmd(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	int err = CMD_RET_USAGE;

#if !defined (CONFIG_SMC_BASED) || defined (CONFIG_BCMBCA_NO_SMC_BOOT)
	int on = 0, zone = -1;
	u32 addr, offset, data;

	if ((argc != 5) && (argc != 4))
		return err;

	switch(argv[1][0]) {
	case 'r':
		addr = atoi(argv[2]);
		offset = atoi(argv[3]);
		err = ReadBPCMRegister(addr, offset>>2, &data);
		printf("bpcm read data 0x%x from addr 0x%x offset 0x%x ret %d\n",
			data, addr, offset, err);
		break;

	case 'w':
		addr = atoi(argv[2]);
		offset = atoi(argv[3]);
		data = atoi(argv[4]);
		err = WriteBPCMRegister(addr, offset>>2, data);
		printf("bpcm write data 0x%x to addr 0x%x offset 0x%x ret %d\n",
			data, addr, offset, err);
		break;

	case 'p':
  		if (!strcmp(argv[2], "on"))
			on = 1;
		else if (!strcmp(argv[2], "off"))
			on = 0;
		else
			return err;

		addr = atoi(argv[3]);
		if (argc == 5)
			zone = atoi(argv[4]);

		if (zone <= -1) {
			if (on)
				err = PowerOnDevice(addr);
			else
				err = PowerOffDevice(addr, 0);
			printf("Power %s device at address 0x%x ret %d\n",
				on ? "on" : "off", addr, err);
		} else {
			if (on)
				err = PowerOnZone(addr, zone);
			else
				err = PowerOffZone(addr, zone);
			printf("Power %s zone %d at address 0x%x ret %d\n",
				on ? "on" : "off", zone, addr, err);
		}
		break;

	default:
		break;
	}
#else
    printf("This Command not supported(%s)\n",__FUNCTION__);
#endif

	return err;
}

static int do_cpufreq(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	int err = CMD_RET_USAGE, freqMHz;

	if (argc > 2)
		return err;

	err = CMD_RET_SUCCESS;
	if (argc == 1) {
		err = biu_ch_freq_get(0, (unsigned int*)&freqMHz);
		if (!err)
			printf("cpu freq is %dMHz\n", freqMHz);
		else
			err = CMD_RET_FAILURE;
	} else {
		freqMHz = atoi(argv[1]);
		freqMHz = set_cpu_freq(freqMHz);
		if (freqMHz > 0)
			printf("cpu freq set to %dMHz\n", freqMHz);
		else
			err = CMD_RET_FAILURE;
	}
	return err;
}
#endif

#ifdef CONFIG_SWREG_ADJUSTMENT
extern int swr_write_v(unsigned int ps, unsigned int reg, unsigned int val);
extern int swr_read_v(unsigned int ps, unsigned int reg);
extern void dump_swregs(int ps);
extern void list_swregs(void);

static int do_swr_cmd(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
    int err = CMD_RET_USAGE;
    int cmd = 0;
    u32 ps, reg, data;

    if (argc < 2)
        goto exit;

    if (!strcmp(argv[1], "l"))
        cmd = 0;
    else if (!strcmp(argv[1], "d"))
        cmd = 1;
    else if (!strcmp(argv[1], "r"))
        cmd = 2;
    else if (!strcmp(argv[1], "w"))
        cmd = 3;
    else
        goto exit;

    if (cmd == 2 && argc != 4)
        goto exit;

    if (cmd == 3 && argc != 5)
        goto exit;

    if (cmd == 0)
    {
        err = CMD_RET_SUCCESS;
        list_swregs();
        goto exit;
    }

    if (cmd == 1)
    {
        if (argc == 3)
            ps = atoi(argv[2]);
        else
            ps = -1;

        err = CMD_RET_SUCCESS;
        dump_swregs(ps);
        goto exit;
    }

    ps = atoi(argv[2]);
    reg = atoi(argv[3]);

    err = CMD_RET_SUCCESS;
    if (cmd == 2)
    {
        if (swr_read_v(ps, reg))
            err = CMD_RET_FAILURE;
        goto exit;
    }

    data = atoi(argv[4]);

    if (swr_write_v(ps, reg, data) == 0)
    {
        printf("Write done. Read it back\n");
        swr_read_v(ps, reg);
    }
    else
        err = CMD_RET_FAILURE;
exit:
    return err;
}
#endif

#if defined(CONFIG_BCMBCA_OTP)

#ifdef CONFIG_SMC_BASED
static const char *cmd_otp_feat_name(int feat)
{
	const char *feat_name[] = {
		[OTP_MAP_BRCM_BTRM_BOOT_ENABLE] = "otp_btrm_boot",
		[OTP_MAP_CUST_BTRM_BOOT_ENABLE] = "otp_cust_boot",
		[OTP_MAP_CUST_MFG_MRKTID] = "otp_mrkid",
		[OTP_MAP_CSEC_CHIPID] = "otp_csec_chipid",
		[SOTP_MAP_CUST_AREA] = "sotp_cust_area",
		[SOTP_MAP_CUST_NECC0] = "sotp_cust_necc0",
		[SOTP_MAP_CUST_NECC1] = "sotp_cust_necc1",
		[SOTP_MAP_CUST_NECC2] = "sotp_cust_necc2",
		[SOTP_MAP_CUST_NECC3] = "sotp_cust_necc3",
		[OTP_MAP_JTAG_PWD] = "otp_jtag_pwd",
		[OTP_MAP_LEDS] = "otp_leds_settings"
	};
	if (feat < 0 || feat >= sizeof(feat_name)/sizeof(feat_name[0]))
		return NULL;
	return feat_name[feat];
}

static int cmd_otp_feat_id(const char *name)
{
	int i;
	for (i = 0; i < OTP_MAP_MAX; i++)
	{
		const char *feat_name = cmd_otp_feat_name(i);
		if (feat_name && !strcmp(name, feat_name))
			return i;
	}
	return -1;
}

static void cmd_otp_feat_names(void)
{
	int i;
	for (i = 0; i < OTP_MAP_MAX; i++)
	{
		const char *name = cmd_otp_feat_name(i);
		if (name)
			printf("%s ", name);
	}
	printf("\n");
}
#endif

#define SDK_OTP_CMD_LEN 64
enum cmd_otp_modes{CMD_OTP_GET,CMD_OTP_SET,CDM_OTP_SEC
#ifdef CONFIG_SMC_BASED
	, CMD_OTP_COMMIT, CMD_OTP_GET_SLICE, CMD_OTP_SET_SLICE, CMD_OTP_CU_LOCK, CMD_OTP_UART_EN
#endif
};
static enum cmd_otp_modes otp_cmd_lookup(char* id)
{
	int i;
	char *cmd2idx[]={"get", "set", "secure"
#ifdef CONFIG_SMC_BASED
		, "commit", "get_slice", "set_slice", "cu_lock", "cu_lock_uart_en"
#endif
	};
	for (i = 0; i < sizeof(cmd2idx)/sizeof(cmd2idx[0]); i++) {
		if (!strcmp(id, cmd2idx[i])) {
			break;
		}
	}
	return i;
}

static int cmd_otp_fuse_verify(otp_map_feat_t id,
			const u8* data,
			u32 size)
{
	u32	*data_verify = NULL,
		size_verify = 0;
	int rc = OTP_MAP_CMN_OK;
	rc = bcm_otp_read(id, &data_verify, &size_verify);
	if (rc) {
		if (rc != OTP_HW_CMN_ERR_KEY_EMPTY) {
			goto err;
		}
		rc = OTP_MAP_CMN_OK;
	}
	if (!data || !(*data)) {
		printf("Invalid Data\n");
		goto err;
	}
	rc = bcm_otp_write(id, (const u32*)data, size);
	if (rc) {
		printf("ERROR writing otp\n");
		goto err;
	}
err:
	return rc;
}

static int cmd_otp_fuse_to_secure(void)
{
	int rc = -1;
	u32 val = 0x1, size = 4;
	if (cmd_otp_fuse_verify(OTP_MAP_BRCM_BTRM_BOOT_ENABLE, (const u8*)&val, size )) {
		goto err;
	}
	val = 0x7;
	if (cmd_otp_fuse_verify(OTP_MAP_CUST_BTRM_BOOT_ENABLE, (const u8*)&val, size )) {
		goto err;
	}
#ifdef CONFIG_SMC_BASED
	/* Commit all writes */
	if (bcm_otp_commit()) {
		goto err;
	}
#endif
	rc = 0;
err:
	return rc;
}


#ifdef CONFIG_SMC_BASED
static int cmd_otp_fuse_cu_lock(void)
{
	int rc = 0;
	u32 val = 1;
	rc = bcm_otp_write(OTP_MAP_DBG_MODE, (const u32*)&val, sizeof(u32));
	if (rc) {
		printf("ERROR writing otp\n");
		goto err;
	}
	rc = bcm_otp_write(OTP_MAP_JU_MODE, (const u32*)&val, sizeof(u32));
	if (rc) {
		printf("ERROR writing otp\n");
		goto err;
	}
err:
	return rc;
}

static int cmd_otp_fuse_uart_en(void)
{
	int rc = 0;
	u32 val = 1;
	rc = bcm_otp_write(OTP_MAP_UART_EN, (const u32*)&val, sizeof(u32));
	if (rc) {
		printf("ERROR writing otp\n");
		goto err;
	}
err:
	return rc;
}
#endif

static int do_otp_cmd(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	int rc = CMD_RET_FAILURE;
#ifndef CONFIG_SMC_BASED
	otp_map_cmn_t* obj = bcm_otp(BCM_OTP_MAP);
	otp_hw_cmn_t* dev = &obj->dev;
	if (argc < 3 || argc > 4) {
		rc = CMD_RET_USAGE;
		goto err;
	}
#endif
	switch(otp_cmd_lookup(argv[1])) {
		case CMD_OTP_GET:
			{
#ifndef CONFIG_SMC_BASED
				int row = simple_strtoul(argv[2],NULL, 0);
				u32 val = 0;

				/*printf("got row %d\n", row);*/
				if (dev->read(dev, row, &val, sizeof(u32))) {
					printf ("ERROR:otp read row %d\n",row);
					goto err;
				}
				printf("0x%x\n", val);
#else
				u8 *p_val = NULL;
				u32 val_size = 0;
				int feat = cmd_otp_feat_id(argv[2]);
				if (feat < 0) {
					printf("Valid feats: \n");
					cmd_otp_feat_names();
					break;
				}
				if (argc != 3) {
					rc = CMD_RET_USAGE;
					goto err;
				}
				printf("GET feat %s(%d) request..\n", argv[2], feat);
				if ((rc=bcm_otp_read(feat, (u32**)&p_val, &val_size))) {
					printf ("ERROR:otp read feat %s(%d) rc=%d\n", argv[2], feat, rc);
					rc = CMD_RET_FAILURE;
					goto err;
				}
				if (!p_val) {
					printf ("ERROR:otp read: internal error, return value address is NULL\n");
					break;
				}
				for (int i = 0; i < val_size; i++) 
					printf("0x%02x%c", p_val[i], (i%8 == 7) ? '\n' : ' ');
				printf("\n");
#endif
			}
			break;

#ifdef CONFIG_SMC_BASED
		case CMD_OTP_GET_SLICE:
			{
				u8 *read_buf;
				u32 slice_offset;
				u32 slice_size;
				int feat;

				if ( argc != 5) {
					printf ("otp get_slice <feat> <offset> <size>\n");
					break;
				}
				feat = cmd_otp_feat_id(argv[2]);
				if (feat < 0) {
					printf("Valid feats: \n");
					cmd_otp_feat_names();
					break;
				}
				slice_offset = simple_strtoul(argv[3], NULL, 0);
				slice_size = simple_strtoul(argv[4], NULL, 0);
				printf("otp get_slice feat %s(%d) [%u..%u]..\n", argv[2], feat, 
					slice_offset, slice_offset+slice_size-1);
				read_buf = malloc(slice_size);
				if (read_buf == NULL) {
					printf ("ERROR:otp can't allocate memory for the read buffer\n");
					break;
				}

				if ((rc=bcm_otp_read_slice(feat, read_buf, slice_offset, slice_size))) {
					printf ("ERROR:otp get_slice feat %s(%d) rc=%d\n", argv[2], feat, rc);
					free(read_buf);
					rc = CMD_RET_FAILURE;
					goto err;
				}
				for (int i = 0; i < slice_size; i++) 
					printf("0x%02x%c", read_buf[i], ((i%16)==15) ? '\n' : ' ');
				printf("\n");
				free(read_buf);
			}
			break;
#endif

	case CMD_OTP_SET:
			if (otp_cmd_lookup(argv[2]) == CDM_OTP_SEC) {
				int len;
				if (bcm_sec_state() != SEC_STATE_UNSEC) {
					printf("Operation supported in unsecure mode only\n");
					break;
				}
				if ( argc < 4) {
					printf(	"WARNING: You are attempting to set a device to MFG Secure mode.\n"\
						"This operation is irreversible. The device will be always\n"\
						"in secure mode after power cycle\n");
					len = cli_readline("Enter 'Y' to continue: ");
					if( !len || console_buffer[0] != 'Y' ) {
						printf("operation aborted.\n");
						break;
					}
				} else if (strcasecmp(argv[3], "-f")) {
        			rc = CMD_RET_USAGE;
					goto err;
				}
				if (!cmd_otp_fuse_to_secure()) {
					printf("Halted. Power cycle ...\n ");
					hang();
				}
#ifdef CONFIG_SMC_BASED
			} else if (otp_cmd_lookup(argv[2]) == CMD_OTP_CU_LOCK) {
				cmd_otp_fuse_cu_lock();
				break;
			} else if (otp_cmd_lookup(argv[2]) == CMD_OTP_UART_EN) {
				cmd_otp_fuse_uart_en();
				break;
			} else if (otp_cmd_lookup(argv[2]) == CMD_OTP_COMMIT) {
				if (argc != 3) {
					rc = CMD_RET_USAGE;
					goto err;
				}
				if (bcm_otp_commit()) {
					printf ("ERROR: failed to commit OTP transactions\n");
					goto err;
				}
				printf ("OTP transactions committed\n");
				break;
#endif
			} else if ( argc == 4) {
#ifndef CONFIG_SMC_BASED
				u32 val[2] = {0,0};
				u32 row = simple_strtoul(argv[2], NULL, 0);
				val[0] = simple_strtoul(argv[3], NULL, 0);
				if (dev->write(dev, row, &val[0], sizeof(u32))) {
					printf ("ERROR:otp write row %u val %u\n",row,val[0]);
					goto err;
				}
#else
				int feat = cmd_otp_feat_id(argv[2]);
				u8 val_buf[64];
				u32 valstr_len = strlen(argv[3]);
				if (feat < 0) {
					printf("Valid feats: \n");
					cmd_otp_feat_names();
					break;
				}
				if ((valstr_len & 1) != 0) {
					printf("ERROR: otp set value must be a hex string with even number of characters\n");
					break;
				}
				if (valstr_len > sizeof(val_buf)*2) {
					printf("ERROR: otp set value is too long\n");
					break;
				}
				if (hex2bin(val_buf, argv[3], valstr_len/2)) {
					printf("ERROR: otp set '%s' must be a hex string\n", argv[3]);
					break;
				}
				if ((rc=bcm_otp_write(feat, (uint32_t *)&val_buf[0], valstr_len/2))) {
					printf ("ERROR:otp write feat %s(%d) rc=%d\n", argv[2], feat, rc);
					rc = CMD_RET_FAILURE;
					goto err;
				}
#endif
				break;
			}
			else {
				printf("ERROR: unknown otp set subcommand '%s'\n", argv[2] ? argv[2] : "null");
				rc = CMD_RET_USAGE;
				goto err;
			}

#ifdef CONFIG_SMC_BASED
		case CMD_OTP_SET_SLICE:
			{
				u32 slice_offset;
				u8 val_buf[64];
				u32 valstr_len;
				int feat;

				if ( argc != 5) {
					printf ("otp set_slice <feat> <offset> <hexstr>\n");
					break;
				}
				feat = cmd_otp_feat_id(argv[2]);
				if (feat < 0) {
					printf("Valid feats: \n");
					cmd_otp_feat_names();
					break;
				}
				valstr_len = strlen(argv[4]);
				if ((valstr_len & 1) != 0) {
					printf("ERROR: otp set_slice value must be a hex string with even number of characters\n");
					break;
				}
				if (valstr_len > sizeof(val_buf)*2) {
					printf("ERROR: otp set_slice value is too long\n");
					break;
				}
				if (hex2bin(val_buf, argv[4], valstr_len/2)) {
					printf("ERROR: otp set_slice '%s' must be a hex string\n", argv[3]);
					break;
				}
				slice_offset = simple_strtoul(argv[3], NULL, 0);
				printf("otp set_slice feat %s(%d) [%u..%u]..\n", argv[2], feat, 
					slice_offset, slice_offset+valstr_len/2-1);
				if ((rc=bcm_otp_write_slice(feat, val_buf, slice_offset, valstr_len/2))) {
					printf ("ERROR:otp set_slice feat %s(%d) rc=%d\n", argv[2], feat, rc);
					rc = CMD_RET_FAILURE;
					goto err;
				}
			}
			break;
#endif

		default:
        	rc = CMD_RET_USAGE;
			printf("ERROR: unknown otp subcommand '%s'\n", argv[1] ? argv[1] : "null");
			goto err;
        }

        rc = CMD_RET_SUCCESS;
err:
	return rc;
}
#endif


static int do_dev_spec_key(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	int err = CMD_RET_USAGE;
#ifndef CONFIG_SMC_BASED
	int rc, len;
	u32 dev_spec_key[8];
	int i;
	bool force = false;
	char * dev_key_status = NULL;
	static bool dev_key_updated = false;
	bool is_set = false;
	bool verbose = false;

	/* If we recently updated the key, return error as key value 
	 * cannot be read  until after a reboot has been completed 
	 */
	if( dev_key_updated ) {
		printf("WARNING: Device specific key has just been updated.\n");
		printf("         Reboot is REQUIRED to access key! Quitting!\n");
		return CMD_RET_FAILURE;
	}

	/* Get the key all the time */
	rc = bcm_sec_get_dev_spec_key((char*)dev_spec_key, sizeof(u32)*8);
	if (rc) {
		printf("ERROR: Cannot retrieve device specific key. SOTP locked!\n");
		env_set("dev_spec_key_status","locked");
		return CMD_RET_FAILURE;
	}

	/* Check if the key is already set */
	for( i=0; i<8; i++ ) {
		if( dev_spec_key[i] ) {
			env_set("dev_spec_key_status","set");
			break;
		}
	}

	if ( (argc == 2) && (strcmp(argv[1], "get") == 0)) {
		/* Do nothing as we have already retrieved the key */
		verbose = true;
	} else if ( (argc >= 3) && (strcmp(argv[1], "set") == 0)) {
		is_set = true;

		/* Check if key has already been set */
		dev_key_status = env_get("dev_spec_key_status");
		if( dev_key_status ) {
			if ( ( strcmp(dev_key_status,"set") == 0 ) ) {
				printf("ERROR: device specific key already set! Quitting\n");
				return CMD_RET_FAILURE;
			} else {
				/* Clear the device specific key status flag */
				env_set("dev_spec_key_status",NULL);
			}
		}

		/* Check if -f flag is present */
		if (!strcasecmp(argv[2], "-f")) {
			force = true;
			argc--;
			argv++;
		}

		/* Check if -v flag is present */
		if (!strcasecmp(argv[2], "-v")) {
			verbose = true;
			argc--;
			argv++;
		}

		if (!strcasecmp(argv[2], "rng")) {
			/* get key from RNG */
#if defined(CONFIG_DM_RNG)	
			struct udevice *dev;
			if (uclass_get_device(UCLASS_RNG, 0, &dev) || !dev) {
				printf("No RNG device\n");
				return err;
			}

			if (dm_rng_read(dev, (void*)dev_spec_key, 32)) {
				printf("Reading RNG failed!\n");
				return err;
			} 
#endif			
		} else if ( argc == 10 ) {
			/* Parse words from cmd line */
			for(i=0; i<8; i++) {
				dev_spec_key[i] = simple_strtoul(argv[i+2],NULL, 0);
			}
		} else {
			return err;
		}

		if ( !force ) {
			printf("WARNING: You are attempting to set a new device specific key:\n");

			len = cli_readline("This operation cannot be undone, enter 'Y' to continue: ");
			if( !len || console_buffer[0] != 'Y' ) {
				printf("## Device specific key setting ABORTED! ##\n");
				printf("## Printing ATTEMPTED key values ##\n");
				rc = CMD_RET_SUCCESS;
				goto print_key;
			}
		}

		/* Write key */
		rc = bcm_sec_set_dev_spec_key((char*)dev_spec_key, sizeof(u32)*8);
		if( rc == CMD_RET_SUCCESS ) {
			dev_key_updated = true;
			env_set("dev_spec_key_status","updated");
			printf("## Device specific key setting SUCCEEDED! ##\n");
			printf("## Reboot REQUIRED to access key! ##\n");
			printf("## Printing COMMITTED key value ##\n");
		}
	} else {
		return err;
	}

print_key:
	if (rc == 0) {
		if( verbose ) {
			if( is_set )
				printf("Set Device Specific Key as:\n");
			else
				printf("Current Device Specific Key:\n");

			printf(" key(bytes)   : [");
			for( i=0; i<8; i++ ) {
				printf("%08x",cpu_to_be32(dev_spec_key[i]));
			}
			printf("]\n");
			printf(" key(LE words): ");
			for( i=0; i<8; i++ ) {
				printf("0x%08x ", dev_spec_key[i]);
			}
			printf("\n");
			printf("\n");
		}
		err = CMD_RET_SUCCESS;
	} else {
		err = CMD_RET_FAILURE;
	}

#endif /* CONFIG_SMC_BASED */
	return err;
}

static int do_sec_ser_num(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	int err = CMD_RET_USAGE;
#ifndef CONFIG_SMC_BASED
	int rc, len;
	u32 sec_ser_num[8] = {0};
	char * sec_ser_num_ptr = (char*)&sec_ser_num[0];
	char * sec_ser_num_char_array;
	char byte_buf[3] = {0};
	int i,j;
	bool force = false;
	bool is_set = false;
	bool is_odd = false;
	char * ser_num_status = NULL;
	static bool ser_num_updated = false;

	/* If we recently updated the serial num, return error as value 
	 * cannot be read  until after a reboot has been completed 
	 */
	if( ser_num_updated ) {
		printf("WARNING: Secure Serial number has just been updated.\n");
		printf("         Reboot is REQUIRED to access new value! Quitting!\n");
		return CMD_RET_FAILURE;
	}

	/* Get the serial num all the time */
	rc = bcm_sec_get_sec_ser_num((char*)sec_ser_num, sizeof(u32)*8);
	if (rc) {
		printf("ERROR: Cannot secure serial number. SOTP locked!\n");
		env_set("sec_ser_num_status","locked");
		return CMD_RET_FAILURE;
	}

	/* Check if the serial num is already set */
	for( i=0; i<8; i++ ) {
		if( sec_ser_num[i] ) {
			env_set("sec_ser_num_status","set");
			break;
		}
	}

	if ( (argc == 2) && (strcmp(argv[1], "get") == 0)) {
		/* Do nothing as we have already retrieved the serial num */
	} else if ( (argc >= 3) && (strcmp(argv[1], "set") == 0)) {
		is_set = true;

		/* Check if serial num has already been set */
		ser_num_status = env_get("sec_ser_num_status");
		if( ser_num_status ) {
			if ( ( strcmp(ser_num_status,"set") == 0 ) ) {
				printf("ERROR: secure serial number already set! Quitting\n");
				return CMD_RET_FAILURE;
			} else {
				/* Clear the secure serial number status flag */
				env_set("sec_ser_num_status",NULL);
			}
		}

		/* Parse and remove the optional FORCE argument */
		if( argc == 4 ) {
			if ( !strcasecmp(argv[2], "-f")) {
				force = true;
				argc--;
				argv++;
			}
		}

		/* Check if byte array exceeds 32 bytes OR contains an ODD number of characters */
		sec_ser_num_char_array = argv[2];

		/* Skip leading 0x if present */
		if( sec_ser_num_char_array[1] == 'x' || sec_ser_num_char_array[1] == 'X' )
			sec_ser_num_char_array += 2;

		debug("sernum:%s len:%d\n", sec_ser_num_char_array, (int)strlen(sec_ser_num_char_array));
		if( ((strlen(sec_ser_num_char_array)/2) > sizeof(sec_ser_num)) || (strlen(sec_ser_num_char_array) % 2)){
			return CMD_RET_USAGE;
		}

		/* Check if there is an odd number of digits so that we can insert an extra zero */
		if( strlen(sec_ser_num_char_array) % 2 )
			is_odd = 1;

		for(i=0,j=sizeof(sec_ser_num)-strlen(sec_ser_num_char_array)/2;
		   i<strlen(sec_ser_num_char_array) && j<sizeof(sec_ser_num); j++){
			if( is_odd && (i == 0) ) {
				byte_buf[1] = sec_ser_num_char_array[i];
				is_odd = false;
				i++;
			} else {
				memcpy(byte_buf,&sec_ser_num_char_array[i], 2);
				i += 2;
			}

			sec_ser_num_ptr[j] = simple_strtoul(byte_buf,NULL,16);
			debug("byte_buf: %s 0x%02x\n", byte_buf, sec_ser_num_ptr[j]);
		}

		if ( !force ) {
			printf("WARNING: You are attempting to set a new secure serial number.\n");
			len = cli_readline("This operation cannot be undone, enter 'Y' to continue: ");
			if( !len || console_buffer[0] != 'Y' ) {
				printf("## Secure serial number setting ABORTED! ##\n");
				printf("## Printing ATTEMPTED serial num ##\n");
				rc = CMD_RET_SUCCESS;
				goto print_ser;
			}
		}
		rc = bcm_sec_set_sec_ser_num((char*)sec_ser_num, sizeof(u32)*8);
		if( rc == CMD_RET_SUCCESS ) {
			ser_num_updated = true;
			env_set("sec_ser_num_status","updated");
			printf("## Secure serial number setting SUCCEEDED! ##\n");
			printf("## Reboot REQUIRED to access serial num! ##\n");
			printf("## Printing COMMITTED serial num ##\n");
		}
	} else {
		return err;
	}

print_ser:
	if (rc == 0) {
		if( is_set )
			printf("Set Secure Serial Number as:\n");
		else
			printf("Current Secure Serial Number:\n");

		printf(" [");
		for( i=0; i<sizeof(sec_ser_num); i++ ) {
			printf("%02x", sec_ser_num_ptr[i]);
		}
		printf("]\n");

		printf("In memory represenation:\n");
		for( i=0; i<8; i++ ) {
			printf(" [%d]0x%08x\n", i, sec_ser_num[i]);
		}
		printf("\n\n");
		err = CMD_RET_SUCCESS;
	} else {
		err = CMD_RET_FAILURE;
	}
#endif /* CONFIG_SMC_BASED */
	return err;
}
static int do_antirollback(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	int err = CMD_RET_USAGE;
#ifndef CONFIG_SMC_BASED
	int rc, len;
	u32 lvl;
	bool force = false;


	if ( (argc == 2) && (strcmp(argv[1], "get") == 0)) {
		rc = bcm_sec_get_antirollback_lvl(&lvl);
	} else if ( (argc >= 3) && (strcmp(argv[1], "set") == 0)) {
		/* Parse and remove the optional FORCE argument */
		if( argc > 3 ) {
			if ( !strcasecmp(argv[2], "-f")) {
				force = true;
				argc--;
				argv++;
			}
		}

		/* get level */
		lvl = atoi(argv[2]);
		if ( !force ) {
			printf("WARNING: You are attempting to set a new anti-rollback level.\n");
			len = cli_readline("This operation cannot be undone, enter 'Y' to continue: ");
			if( !len || console_buffer[0] != 'Y' ) {
				printf("Anti-rollback level setting aborted.\n");
				return CMD_RET_SUCCESS;
			}
		}
		rc = bcm_sec_set_antirollback_lvl(lvl);
	} else {
		return err;
	}

	if (rc == 0) {
		printf("Current anti-rollback level set to: %d\n", lvl);
		err = CMD_RET_SUCCESS;
	} else {
		err = CMD_RET_FAILURE;
	}
#endif /* CONFIG_SMC_BASED */
	return err;
}

#ifdef CONFIG_BCMBCA_XRDP_ETH

extern __weak int bcmbca_link_status(void);

static int do_wait_link_status(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
    int timeout = 0;
    int ret = 1; // by default link up

    if (argc == 2)
        timeout = atoi(argv[1]);

    if(!cli_jobs_cb)
        return CMD_RET_FAILURE;

    cli_jobs_cb(); //starting network driver initialization

    while(1)
    {
        if (argc == 2) //timeout passed to function
        {
            if(timeout-- == 0)
                break;
        }

        ret = bcmbca_link_status();
        if(ret) //link up
            break;

        mdelay(1000);
    }
    return 0;
}


extern int bcmbca_xrdp_eth_phy_status(void);
extern int bcmbca_xrdp_eth_mac_status(void);
extern int bcmbca_xrdp_eth_active_port_get(void);
extern int bcmbca_xrdp_eth_active_port_set(int port);
extern int bcmbca_xrdp_eth_env_active_port_set(int port);

static int do_eth_active_port(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
    int port;

	if (argc == 2)
    {
        port = (atoi(argv[1]));
        bcmbca_xrdp_eth_active_port_set(port);
    }

    port = bcmbca_xrdp_eth_active_port_get();
    printf("XRDP Ethernet active port is set to %d\n" ,port);

	return 0;
}

static int on_active_port(const char *name, const char *value, enum env_op op, int flags)
{
    int active_port = -1;

	if ((flags & H_INTERACTIVE) == 0)
		return 0;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:
        active_port = simple_strtoul(value, NULL, 10);
        break;
    case env_op_delete:
        break;
    default:
        break;
    }

    return bcmbca_xrdp_eth_env_active_port_set(active_port);
}
U_BOOT_ENV_CALLBACK(active_port, on_active_port);
#endif

static int do_rawimg_fixup(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
#ifdef BCA_SDK_EMMC_CMD
	if( ( strcasecmp_image_media(FLASH_DEV_STR_EMMC) == 0 ) ) {
		run_command("gpt read mmc 0 currentparts", 0);
		run_command("gpt write mmc 0 $currentparts", 0);
		run_command("gpt verify mmc 0 $currentparts", 0);
		run_command("env delete currentparts", 0);
	}
#endif
	return 0;
}

#if defined(CONFIG_DM_RNG)
static int do_kaslr_status(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	int enable_flag;
	if ( (argc == 2) && (strcmp(argv[1], "get") == 0)) {
		enable_flag = env_get_hex("kaslr_enabled", 0);
	} else if ( (argc == 3)
		    && (strcmp(argv[1], "set") == 0)
		    && (atoi(argv[2]) <= 1) ) {
		enable_flag = atoi(argv[2]);
		env_set_hex("kaslr_enabled", enable_flag);
		env_save();
	} else {
		return CMD_RET_USAGE;
	}

	printf("KASLR Status: %s\n", (enable_flag==1?"Enabled":"Disabled"));
	return CMD_RET_SUCCESS;
}
#endif

#ifdef CONFIG_BCMBCA_VFBIO
static void print_lun_info(int id)
{
		unsigned int blk_n, blk_s;

		vfbio_lun_get_blk_num(id, &blk_n);
		vfbio_lun_get_blk_size(id, &blk_s);
		printf("LUN %d (%s)\n", id, vfbio_lun_get_name(id));
		printf("\t 0x%llx(%u x %u)\n", (uint64_t)blk_n * blk_s, blk_n, blk_s);
}

static int do_vfbio(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (strncmp(argv[1], "info", 3) == 0) {
		int id = -1;
		int ret;
		uint64_t total_size, free_size;
		
		if (argc == 3)
		{
			if(!vfbio_lun_get_id(argv[2], &id))
			{
				print_lun_info(id);
			}
			else if(strncmp(argv[2], "dev", 3) == 0)
			{
				vfbio_device_get_info(&total_size, &free_size);
				printf("Flash size:	%llx (%llx free)\n", total_size, free_size);
			}
			else
			{
				printf("LUN %s doesn't exist\n", argv[2]);
			}
		}
		else
		{
			while(!(ret = vfbio_lun_get_next(id, &id)))
			{
				print_lun_info(id);
			}
		}

		return 0;
	}
	else if (strncmp(argv[1], "id", 2) == 0) {
		int id;
		if (argc < 3)
			return CMD_RET_USAGE;

		if(!vfbio_lun_get_id(argv[2], &id))
		{
			printf("LUN %d (%s)\n", id, argv[2]);
			env_set_hex("lun_id", id);
			return CMD_RET_SUCCESS;
		}

		return CMD_RET_FAILURE;
	}
	else if (strncmp(argv[1], "create", 3) == 0) {
		int id = -1;
		uint64_t size;
		uint32_t flags = 0;

		if (argc < 4)
			return CMD_RET_USAGE;

		size = simple_strtoull(argv[2], NULL, 16);
		
		if (argc >= 5)
		{
			for (int i=4; i<argc; i++)
			{
				if (!strcmp(argv[i], "-ro"))
					flags |= VFBIO_LUN_CREATE_FLAG_READ_ONLY;
				else if (!strcmp(argv[i], "-e"))
					flags |= VFBIO_LUN_CREATE_FLAG_ENCRYPTED;
				else
					id = simple_strtoul(argv[4], NULL, 10);
			}
		}
		
		if (vfbio_lun_create(argv[3], size, flags, &id))
		{
			if (argc < 5)
				printf("Cannot create LUN %s (size %sB)\n", argv[3], argv[2]);
			else
				printf("Cannot create LUN %s (ID %s, size %sB)\n", argv[3], argv[4], argv[2]);
			return CMD_RET_FAILURE;
		}
		
		return CMD_RET_SUCCESS;
	}
	else if (strncmp(argv[1], "delete", 3) == 0) {
		int id;

		if (argc < 3)
			return CMD_RET_USAGE;

		if(vfbio_lun_get_id(argv[2], &id))
			id = simple_strtoul(argv[2], NULL, 10);

		if(vfbio_lun_delete(id))
		{
			printf("Cannot delete LUN %s\n", argv[2]);
			return CMD_RET_FAILURE;
		}
		
		return CMD_RET_SUCCESS;
	}
	else if (strncmp(argv[1], "resize", 3) == 0) {
		int id = -1;
		uint64_t size;

		if (argc < 4)
			return CMD_RET_USAGE;

		if(vfbio_lun_get_id(argv[2], &id) && isdigit(*argv[2]))
			id = simple_strtoul(argv[2], NULL, 10);
		size = simple_strtoul(argv[3], NULL, 16);
		
		if(vfbio_lun_resize(id, size))
		{
			printf("Cannot resize LUN %s\n", argv[2]);
			return CMD_RET_FAILURE;
		}
		return CMD_RET_SUCCESS;
	}
	else if (strncmp(argv[1], "rename", 3) == 0) {
		int i,j;
		struct vfbio_lun_id_name *id_name;

		if ((argc < 4) || (argc & 1))
			return CMD_RET_USAGE;

		id_name = malloc(sizeof(struct vfbio_lun_id_name) * (argc-2));
		for(i=2, j=0; i<argc; i+=2, j++)
		{
			id_name[j].id = simple_strtoul(argv[i], NULL, 10);
			strcpy(id_name[j].name, argv[i+1]);
		}
		
		if(vfbio_lun_rename(j, id_name))
		{
			printf("Atomic rename failed\n");
			return CMD_RET_FAILURE;
		}

		return CMD_RET_SUCCESS;
	}
	else if (strncmp(argv[1], "read", 3) == 0) {
		int id;
		ulong addr;
		ulong blk;
		ulong cnt = UINT_MAX;
		uint blk_s;
		int ret;

		if (argc < 4)
			return CMD_RET_USAGE;

		if(vfbio_lun_get_id(argv[2], &id))
			id = simple_strtoul(argv[2], NULL, 10);
		addr = simple_strtoul(argv[3], NULL, 16);
		if (argc == 6)
		{
			blk = simple_strtoul(argv[4], NULL, 10);
			cnt = simple_strtoul(argv[5], NULL, 10);
		}
		else
		{
			blk = 0;
			if((argc == 5) && !vfbio_lun_get_blk_size(id, &blk_s))
				cnt = (simple_strtoul(argv[4], NULL, 16) + blk_s - 1) / blk_s;
		}

		printf("\nread: device %d block # "LBAFU", count %lu ... ", id, blk, cnt);

		ret = vfbio_lun_read(id, blk, cnt, (void *)addr);
		if(!ret)
			env_set_hex("fileaddr", addr);

		printf("%s\n", ret ? "ERROR" : "OK");
		return ret;
	} else if (strncmp(argv[1], "write", 3) == 0) {
		int id;
		ulong addr;
		ulong blk;
		ulong cnt;
		uint blk_s;
		int ret;

		if (argc < 5)
			return CMD_RET_USAGE;

		if(vfbio_lun_get_id(argv[2], &id))
			id = simple_strtoul(argv[2], NULL, 10);
		addr = simple_strtoul(argv[3], NULL, 16);
		if (argc == 6)
		{
			blk = simple_strtoul(argv[4], NULL, 10);
			cnt = simple_strtoul(argv[5], NULL, 10);
		}
		else
		{
			blk = 0;
			if(!vfbio_lun_get_blk_size(id, &blk_s))
			{
				cnt = (simple_strtoul(argv[4], NULL, 16) + blk_s - 1) / blk_s;
			}
			else
			{
				printf("Failed to retreive LUN %d block size, nothing to write\n", id);
				cnt = 0;
			}
		}

		printf("\nwrite: device %d block # "LBAFU", count %lu ... ", id, blk, cnt);

		ret = vfbio_lun_write(id, blk, cnt, (void *)addr);

		printf("%s\n", ret ? "ERROR" : "OK");
		return ret;
	} else if (strncmp(argv[1], "1boot", 3) == 0) {
		int id;
		unsigned int blk_s;
		void *addr = (void *)CONFIG_SYS_LOAD_ADDR;
		struct vfbio_lun_info info;
		ulong size = env_get_ulong("vf_1_boot", 10, UINT_MAX);

		if (vfbio_lun_get_id("image-pack", &id)) {
			printf("Didn't find initial bundle LUN image-pack\n");
			return 1;
		}

		if (size != UINT_MAX) {
			if (vfbio_lun_get_blk_size(id, &blk_s))
				size = UINT_MAX;
			else
				size = (size + blk_s -1)/blk_s;
		}

		printf("Reading initial bundle LUN %lu blocks...\n", size);
		if (vfbio_lun_read(id, 0, size, addr)) {
			printf("Failed to read initial bundle LUN image-pack(%d)\n", id);
			return 1;
		}

		if (!fit_all_image_verify(addr)) {
			printf("Bad hashes in initial bundle LUN image-pack(%d)\n", id);
			return 1;
		}

		if (vfbio_finish_first_boot()) {
			printf("Bad hashes in initial bundle LUN image-pack(%d)\n", id);
			return 1;
		}

#define VFBIO_DYN_LUN_MAX 223
		for(id = 0; id < VFBIO_DYN_LUN_MAX ; id++)
		{
			if (!vfbio_rpc_lun_info(id, &info) && info.n_blks)
			{
				printf("FTL managed partition is not empty, probably needed to erase it before burning initial bundle LUN\n");
				return 1;
			}
		}

		if (flash_upgrade_img_bundle((ulong)addr, 1, NULL)) {
			printf("Failed to burn initial bundle LUN image-pack(%d)\n", id);
			return 1;
		}

		commit_image(1);
		env_set("fileaddr", NULL);
		if (env_set("vf_1_boot", NULL) || env_set("once", "true") || env_save()) {
			printf("Failed to update environment burning initial bundle\n");
			return 1;
		}

		bcm_rpc_ba_report_boot_success(BA_SVC_RESET_BOOT_COUNT);
		do_reset(NULL, 0, 0, NULL);

		return 0;
	}

	return CMD_RET_USAGE;
}
#endif

static char usage[] =
#ifdef BCA_SDK_EMMC_CMD
    "\n## eMMC Specific Test Commands ##\n"
    "gpt_fixup\n"
    " - Create GPT partitions if partition sizes are specfied via special uboot\n"
    " - environment variables. The env variable name has to be in the format:\n"
    " - 	<partition_name>_vol_size=<size in MiB>\n"
#endif /* BCA_SDK_EMMC_CMD */
#ifdef BCA_SDK_SPINOR_CMD
    "\n## SPINOR Specific Test Commands ##\n"
    "flash_spinor_fit\n"
    " - Download and flash atf+uboot+vmlinux+dtb in brcm_full.itb to SPI NOR flash \n"
    "flash_spinor_rootfs\n"
    " - Download and flash rootfs in rootfs.squashfs to SPI NOR flash\n"
    "flash_spinor_upgrade [-s] <filename> [conf]\n"
    "flash_spinor_upgrade <filename> [<img> [<conf>]]\n"
    " - Download and flash an image upgrade bundle to SPI NOR flash\n"
    "flash_spinor_binary <filename> \n"
    " - Erase whole flash,download and flash a binary image to SPI NOR flash start from 0\n"
    "boot_spinor [boardid]\n"
    " - Boot linux from image in SPI NOR flash.\n"
#endif
    "\n## Generic Test Commands ##\n"
#ifndef CONFIG_BCMBCA_VFBIO
    "force \n"
    " - Without arguments, display if forced image updates are enabled\n"
    "force [force value] <[Boot Flash Type] [Image Flash Type]>\n"
    " - Enable(value=1)/Disable(value=0) forced image updates. All compatibility checks are ignored if enabled\n"
    " - If boot and image flash types are not detected, then they need to be specified as 2nd and 3rd args\n"
    " - Valid flash type values = [NAND|SPINAND|EMMC|NOR]\n"
#endif
    "restoredefault\n"
    " - Delete contents of /data so that it gets populated with default values\n"
    "flash_loader\n"
    " - Download and flash SPL+TPL in loader_test_<flash>_<chip>.bin to boot flash\n"
#if defined(BCA_SDK_NAND_CMD) || defined(BCA_SDK_EMMC_CMD)
    "flash_bootfs_raw <bootfs binary> [img]\n"
    "flash_rootfs_raw <rootfs binary> [img]\n"
    " - Download and flash raw bootfs/rootfs binary to flash \n"
    " - WARNING: These are raw writes with no content, security or compatibility checks. Only valid flag\n"
    "            in metadata is set, commit flag is NOT updated \n"
    "metadata \n"
    "metadata [committed] [valid],[valid]\n"
    " - without arguments, parse, display metadata and populate metadata_committed environment value\n"
    " - set metadata values (example \"metadata 1 1,2\" for both images valid and #1 committed\n"
    " -                              \"metadata 2\"     to set committed image only [i.e. to #2] if it's valid)\n"
    " - NOTE: Must do a sw reset for metadata changes to take affect\n"
#endif
    "flash_img_upgrade [-s] <filename> [conf]\n"
    "flash_img_upgrade [-i] <filename> [<img> [<conf>]]\n"
    " - Download and flash an image upgrade bundle via TFTP\n"
    " - NOTE: Must do a sw reset to use the new image\n"
    "load_img [boardid]\n"
    "load_img [<img> [<boardid>]]\n"
    " - Load image from flash into DDR.\n"
    "boot_img [boardid]\n"
    "boot_img [<img> [<boardid>]]\n"
    " - Boot linux from image in flash.\n"
    "[Legend - Optional Parameters]\n"
    " -s:      Skip downloading the image via tftp. Assume it is already at ${fileaddr}\n"
    " -i:      Inactive. Mark new image as inactive i.e Only mark image as valid but do not set commit flag\n"
    " img:     Image index for flashing/booting. Valid values: [1|2]. Default index is determined from metadata\n"
    " fstype:  Filesystem type for flashing. Valid values: [squashfs|ubifs|ext4]. Default is squashfs\n"
    " boardid: Boardid to determine boot configuration. If omitted default configuration node is used\n"
    " conf:    Configuration name used for flashing img bundle. If omitted default configuration node is used\n\n"
#ifdef CONFIG_BCMBCA_HTTPD
    "httpd_start \n"
    " - without arguments, start httpd server\n"
#endif
#ifdef CONFIG_BCMBCA_PMC
    "bpcm_cmd <r|w> <addr> <offset> [<data>]\n"
    " - read/write bpcm register\n"
    "bpcm_cmd p <on|off> <addr> [<zone>]\n"
    " - power on/off zone/device. set zone to -1 or omit for device\n"
    "cpufreq <freq in MHz>\n"
    " - set cpu frequency\n"
#endif
#ifdef CONFIG_SWREG_ADJUSTMENT
    "swr <r|w|d|l> [<power_supply_idx> <register> [<data>]]\n"
    " - read/write SWReg value\n"
    " - dump SWReg values for all supported power supplies\n"
    " - list all supported power supplies\n"
#endif
#if defined(CONFIG_BCMBCA_OTP)
#ifndef CONFIG_SMC_BASED
    "antirollback get\n"
    " - get current anti-rollback level\n"
    "antirollback set [-f] <new anti-rollback level to set>\n"
    " - set new anti-rollback level. [-f] will force the commit and skip warnings\n"
    "sec_ser_num get \n"
    " - get current secure serial number level ( 64 digits )\n"
    "sec_ser_num set [-f] < Hex secure serial number ( Max 64 digits )>\n"
    " - set new secure serial number . [-f] will force the commit and skip warnings\n"
    "dev_spec_key get \n"
    " - get current device specific key ( 32 bytes )\n"
    "dev_spec_key set [-f] [-v] <32-bit hex word 0>...<32-bit hex word 7> \n"
    " - set new device specific key. [-f] will force the commit and skip warnings\n"
    " - [-v] will dump the key value that was specified and committed\n"
    "dev_spec_key set [-f] [-v] rng \n"
    " - generate device specific key via RNG and set it. [-f] will force the commit\n"
    " - [-v] will dump the key value that was specified and committed\n"
    "[NOTE: antirollback, sec_ser_num and dev_spec_key ops]\n"
    "  antirollback, sec_ser_num and dev_spec_key operations require the SOTP hardware\n"
    "  and associated keyslots to be in an accesable state. Ensure that 'sotp-lock' in \n"
    "  security policy and security trust fit is configured properly.\n"
    "otp set <row> <val> fuses <val> at <row> to OTP . Warning! otp fuse operations are irreverisble\n"
    "otp get <row>       reads content at <row> from  OTP\n"
#else
    "otp set <feat_name> <hexstr> Sets OTP feat value. All values are fused by 'otp set commit' command\n"
    "otp get <feat_name> reads the feat value from  OTP\n"
    "otp set_slice <feat_name> <offset> <hexstr> Sets OTP feat value.\n"
	"                    All values are fused by 'otp set commit' command\n"
    "otp get_slice <feat_name> <offset> <size> reads the feat value from  OTP\n"
    "otp set commit      Commit all prior otp set requests to OTP. Warning! otp fuse operations are irreverisble\n"
    "otp set cu_lock     Set CU lock bit in OTP\n"
    "otp set cu_lock_uart_en  Set CU lock uart enable bit OTP\n"
#endif
    "otp set secure      Achtung!!!! This command will turn board to MFG boot secure mode.\n"
    "                    On success the board will be halted and must be power cycled.\n"
    "                    Specify -f  option to run a non-interactive mode.\n"
#endif
    ;

static char sdk_usage[] =
#ifndef CONFIG_BCMBCA_VFBIO
    "\nforce\n"
    " - Without arguments, display if forced image updates are enabled\n"
    "force [force value] <[Boot Flash Type] [Image Flash Type]>\n"
    " - Enable(value=1)/Disable(value=0) forced image updates. All compatibility checks are ignored if enabled\n"
    " - If boot and image flash types are not detected, then they need to be specified as 2nd and 3rd args\n"
    " - Valid flash type values = [NAND|SPINAND|EMMC|NOR]\n"
#endif
    "restoredefault\n"
    " - Delete contents of /data so that it gets populated with default values\n"
#if defined(BCA_SDK_NAND_CMD) || defined(BCA_SDK_EMMC_CMD)
    "flash_bootfs_raw <bootfs binary> [img]\n"
    "flash_rootfs_raw <rootfs binary> [img]\n"
    " - Download and flash raw bootfs/rootfs binary to flash \n"
    " - WARNING: These are raw writes with no content, security or compatibility checks. Only valid flag\n"
    "            in metadata is set, commit flag is NOT updated \n"
    "metadata \n"
    "metadata [committed] [valid],[valid]\n"
    " - without arguments, parse, display metadata and populate metadata_committed environment value\n"
    " - set metadata values (example \"metadata 1 1,2\" for both images valid and #1 committed\n"
    " -                              \"metadata 2\"     to set committed image only [i.e. to #2] if it's valid)\n"
    " - NOTE: Must do a sw reset for metadata changes to take affect\n"
#endif
    "flash_img_upgrade [-s] <filename> [conf]\n"
    "flash_img_upgrade [-i] <filename> [<img> [<conf>]]\n"
    " - Download and flash an image upgrade bundle via TFTP\n"
    " - NOTE: Must do a sw reset to use the new image\n"
    "load_img [boardid]\n"
    "load_img [<img> [<boardid>]]\n"
    " - Load image from flash into DDR.\n"
    "boot_img [boardid]\n"
    "boot_img [<img> [<boardid>]]\n"
    " - Boot linux from image in flash.\n"
    "[Legend - Optional Parameters]\n"
    " -s:      Skip downloading the image via tftp. Assume it is already at ${fileaddr}\n"
    " -i:      Inactive. Mark new image as inactive i.e Only mark image as valid but do not set commit flag\n"
    " img:     Image index for flashing/booting. Valid values: [1|2]. Default index is determined from metadata\n"
    " boardid: Boardid to determine boot configuration. If omitted default configuration node is used\n"
    " conf:    Configuration name used for flashing img bundle. If omitted default configuration node is used\n\n"
#ifdef CONFIG_BCMBCA_HTTPD
    "httpd_start \n"
    " - without arguments, start httpd server\n"
#endif
#ifdef CONFIG_BCMBCA_XRDP_ETH
    "active_port [port]\n"
    " - Set the active network driver's port to send packets from\n"
    " - without arguments, display the current active port\n"
    "wait_link_status [timeout]\n"
    " - wait for network phy link up\n"
    " - agument timeout is in seconds\n"
    " - without arguments returns only when link up\n"
#endif
#if defined(CONFIG_BCMBCA_OTP)
#ifndef CONFIG_SMC_BASED
    "otp set <row> <val> fuses <val> at <row> to OTP . Warning! otp fuse operations are irreverisble\n"
    "otp get <row>       reads content at <row> from  OTP\n"
#else
    "otp set <feat_name> <hexstr> Sets OTP feat value. All values are fused by 'otp set commit' command\n"
    "otp get <feat_name> reads the feat value from  OTP\n"
    "otp set_slice <feat_name> <offset> <hexstr> Sets OTP feat value.\n"
	"                    All values are fused by 'otp set commit' command\n"
    "otp get_slice <feat_name> <offset> <size> reads the feat value from  OTP\n"
    "otp set commit      Commit all prior otp set requests to OTP. Warning! otp fuse operations are irreverisble\n"
    "otp set cu_lock     Set CU lock bit in OTP\n"
    "otp set cu_lock_uart_en   Set CU lock uart enable bit OTP\n"
#endif
    "otp set secure      Achtung!!!!! This command will turn board to MFG boot secure mode.\n"
    "                    On success the board will be halted and must be power cycled.\n"
    "rawimg_fixup        Configures proper settings for an image that was written in raw form .\n"
#endif
#if defined(CONFIG_DM_RNG)
    "sdk kaslr_status set <0=Disabled|1=Enabled>\n"
    "sdk kaslr_status get\n "
    " - Enable/Disable kaslr in the next linux image to be loaded\n"
#endif
#ifdef CONFIG_BCMBCA_VFBIO
    "vfbio info [name|dev] - show specific LUN, all available LUNs or vfbio device info\n"
    "vfbio create size [name] [id] [-ro] [-e] - create new LUN by name or by name and id. -ro=read-only, -e=encrypted\n"
    "vfbio delete name|id - delete LUN\n"
    "vfbio resize name|id size - resize LUN\n"
    "vfbio rename id name [id name...] - atomically rename number of LUNs\n"
    "vfbio read name|id addr [[blk# cnt]|[size]] - read `cnt' blocks starting at block `blk#', or entire LUN or size bytes at block 0 to memory address `addr'\n"
    "vfbio write name|id addr [blk# cnt]|[size] - write `cnt' blocks starting at block `blk#' or size bytes at block 0 from memory address `addr'"
#endif
    ;

U_BOOT_CMD_WITH_SUBCMDS(bca_test, "Broadcom test commands", usage,
#if !defined(CONFIG_BCMBCA_VFBIO)
			U_BOOT_SUBCMD_MKENT(flash_loader, 1, 0, do_flash_loader),
			U_BOOT_SUBCMD_MKENT(force, 4, 0, do_force),
#endif
#ifdef BCA_SDK_EMMC_CMD
			U_BOOT_SUBCMD_MKENT(gpt_fixup, 1, 0, do_gpt_fixup),
#endif /* BCA_SDK_EMMC_CMD */
#ifdef BCA_SDK_SPINOR_CMD
			U_BOOT_SUBCMD_MKENT(flash_spinor_fit, 1, 0, do_flash_spinor_bootfs_rootfs),
			U_BOOT_SUBCMD_MKENT(flash_spinor_rootfs, 1, 0, do_flash_spinor_bootfs_rootfs),
			U_BOOT_SUBCMD_MKENT(flash_spinor_upgrade, 5, 0, do_flash_upgrade_img),
			U_BOOT_SUBCMD_MKENT(flash_spinor_binary, 2, 0, do_flash_spinor_binary),
			U_BOOT_SUBCMD_MKENT(boot_spinor, 3, 0, do_boot),
#endif
#if defined(CONFIG_BCM_BOOTSTATE)
			U_BOOT_SUBCMD_MKENT(activate, 1, 0, do_activate),
#endif /* CONFIG_BCM_BOOTSTATE */
			U_BOOT_SUBCMD_MKENT(restoredefault, 1, 0, do_restoredefault),
#if defined(BCA_SDK_NAND_CMD) || defined(BCA_SDK_EMMC_CMD)
			U_BOOT_SUBCMD_MKENT(flash_bootfs_raw, 3, 0, do_flash_bins),
			U_BOOT_SUBCMD_MKENT(flash_rootfs_raw, 3, 0, do_flash_bins),
			U_BOOT_SUBCMD_MKENT(metadata, 5, 0, do_metadata),
#endif
			U_BOOT_SUBCMD_MKENT(flash_img_upgrade, 5, 0, do_flash_upgrade_img),
			U_BOOT_SUBCMD_MKENT(boot_img, 3, 0, do_boot)
#ifdef CONFIG_BCMBCA_HTTPD
			,U_BOOT_SUBCMD_MKENT(httpd_start, 1, 0, do_httpd_start)
#endif
#ifdef CONFIG_BCMBCA_PMC
			,U_BOOT_SUBCMD_MKENT(bpcm_cmd, 5, 0, do_bpcm_cmd)
			,U_BOOT_SUBCMD_MKENT(cpufreq, 2, 0, do_cpufreq)
#endif
#ifdef CONFIG_SWREG_ADJUSTMENT
			,U_BOOT_SUBCMD_MKENT(swr, 5, 0, do_swr_cmd)
#endif
			,U_BOOT_SUBCMD_MKENT(antirollback, 4, 0, do_antirollback)
			,U_BOOT_SUBCMD_MKENT(sec_ser_num, 12, 0, do_sec_ser_num)
			,U_BOOT_SUBCMD_MKENT(dev_spec_key, 13, 0, do_dev_spec_key)
#if defined(CONFIG_BCMBCA_OTP)
			,U_BOOT_SUBCMD_MKENT(otp, 5, 0, do_otp_cmd),
#endif
    );

U_BOOT_CMD_WITH_SUBCMDS(sdk, "Broadcom SDK support commands", sdk_usage,
#if !defined(CONFIG_BCMBCA_VFBIO)
			U_BOOT_SUBCMD_MKENT(force, 4, 0, do_force),
#endif
			U_BOOT_SUBCMD_MKENT(restoredefault, 1, 0, do_restoredefault),
#if defined(BCA_SDK_NAND_CMD) || defined(BCA_SDK_EMMC_CMD) || defined(CONFIG_BCMBCA_VFBIO)
			U_BOOT_SUBCMD_MKENT(metadata, 5, 0, do_metadata),
#endif
			U_BOOT_SUBCMD_MKENT(flash_img_upgrade, 5, 0, do_flash_upgrade_img),
#if defined(CONFIG_BCMBCA_OTP)
			U_BOOT_SUBCMD_MKENT(otp, 5, 0, do_otp_cmd),
#endif
			U_BOOT_SUBCMD_MKENT(boot_img, 3, 0, do_boot),
			U_BOOT_SUBCMD_MKENT(load_img, 3, 0, do_load),
			U_BOOT_SUBCMD_MKENT(rawimg_fixup, 1, 0, do_rawimg_fixup)
#ifdef CONFIG_BCMBCA_HTTPD
			,U_BOOT_SUBCMD_MKENT(httpd_start, 1, 0, do_httpd_start)
#endif
#ifdef CONFIG_BCMBCA_XRDP_ETH
			,U_BOOT_SUBCMD_MKENT(active_port, 2, 0, do_eth_active_port)
			,U_BOOT_SUBCMD_MKENT(wait_link_status, 2, 0, do_wait_link_status)
#endif
#if defined(CONFIG_DM_RNG)
			,U_BOOT_SUBCMD_MKENT(kaslr_status, 4, 0, do_kaslr_status)
#endif
#ifdef CONFIG_BCMBCA_VFBIO
			,U_BOOT_SUBCMD_MKENT(vfbio, 14, 1, do_vfbio)
#endif

#if defined(CONFIG_BCM_BOOTSTATE) && defined(CONFIG_BCM_BOOTSTATE_FALLBACK_SUPPORT)
			,U_BOOT_SUBCMD_MKENT(check_fallback, 2, 0, do_check_fallback)
#endif
    );

