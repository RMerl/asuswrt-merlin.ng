// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
 */

#include <common.h>
#include <blk.h>
#include <command.h>
#include <console.h>
#include <display_options.h>
#include <memalign.h>
#include <mmc.h>
#include <part.h>
#include <sparse_format.h>
#include <image-sparse.h>

static int curr_device = -1;

static void print_mmcinfo(struct mmc *mmc)
{
	int i;

	printf("Device: %s\n", mmc->cfg->name);
	printf("Manufacturer ID: %x\n", mmc->cid[0] >> 24);
	if (IS_SD(mmc)) {
		printf("OEM: %x\n", (mmc->cid[0] >> 8) & 0xffff);
		printf("Name: %c%c%c%c%c \n", mmc->cid[0] & 0xff,
		(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
		(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);
	} else {
		printf("OEM: %x\n", (mmc->cid[0] >> 8) & 0xff);
		printf("Name: %c%c%c%c%c%c \n", mmc->cid[0] & 0xff,
		(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
		(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff,
		(mmc->cid[2] >> 24));
	}

	printf("Bus Speed: %d\n", mmc->clock);
#if CONFIG_IS_ENABLED(MMC_VERBOSE)
	printf("Mode: %s\n", mmc_mode_name(mmc->selected_mode));
	mmc_dump_capabilities("card capabilities", mmc->card_caps);
	mmc_dump_capabilities("host capabilities", mmc->host_caps);
#endif
	printf("Rd Block Len: %d\n", mmc->read_bl_len);

	printf("%s version %d.%d", IS_SD(mmc) ? "SD" : "MMC",
			EXTRACT_SDMMC_MAJOR_VERSION(mmc->version),
			EXTRACT_SDMMC_MINOR_VERSION(mmc->version));
	if (EXTRACT_SDMMC_CHANGE_VERSION(mmc->version) != 0)
		printf(".%d", EXTRACT_SDMMC_CHANGE_VERSION(mmc->version));
	printf("\n");

	printf("High Capacity: %s\n", mmc->high_capacity ? "Yes" : "No");
	puts("Capacity: ");
	print_size(mmc->capacity, "\n");

	printf("Bus Width: %d-bit%s\n", mmc->bus_width,
			mmc->ddr_mode ? " DDR" : "");

#if CONFIG_IS_ENABLED(MMC_WRITE)
	puts("Erase Group Size: ");
	print_size(((u64)mmc->erase_grp_size) << 9, "\n");
#endif

	if (!IS_SD(mmc) && mmc->version >= MMC_VERSION_4_41) {
		bool has_enh = (mmc->part_support & ENHNCD_SUPPORT) != 0;
		bool usr_enh = has_enh && (mmc->part_attr & EXT_CSD_ENH_USR);
		ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);
		u8 wp;
		int ret;

#if CONFIG_IS_ENABLED(MMC_HW_PARTITIONING)
		puts("HC WP Group Size: ");
		print_size(((u64)mmc->hc_wp_grp_size) << 9, "\n");
#endif

		puts("User Capacity: ");
		print_size(mmc->capacity_user, usr_enh ? " ENH" : "");
		if (mmc->wr_rel_set & EXT_CSD_WR_DATA_REL_USR)
			puts(" WRREL\n");
		else
			putc('\n');
		if (usr_enh) {
			puts("User Enhanced Start: ");
			print_size(mmc->enh_user_start, "\n");
			puts("User Enhanced Size: ");
			print_size(mmc->enh_user_size, "\n");
		}
		puts("Boot Capacity: ");
		print_size(mmc->capacity_boot, has_enh ? " ENH\n" : "\n");
		puts("RPMB Capacity: ");
		print_size(mmc->capacity_rpmb, has_enh ? " ENH\n" : "\n");

		for (i = 0; i < ARRAY_SIZE(mmc->capacity_gp); i++) {
			bool is_enh = has_enh &&
				(mmc->part_attr & EXT_CSD_ENH_GP(i));
			if (mmc->capacity_gp[i]) {
				printf("GP%i Capacity: ", i+1);
				print_size(mmc->capacity_gp[i],
					   is_enh ? " ENH" : "");
				if (mmc->wr_rel_set & EXT_CSD_WR_DATA_REL_GP(i))
					puts(" WRREL\n");
				else
					putc('\n');
			}
		}
		ret = mmc_send_ext_csd(mmc, ext_csd);
		if (ret)
			return;
		wp = ext_csd[EXT_CSD_BOOT_WP_STATUS];
		for (i = 0; i < 2; ++i) {
			printf("Boot area %d is ", i);
			switch (wp & 3) {
			case 0:
				printf("not write protected\n");
				break;
			case 1:
				printf("power on protected\n");
				break;
			case 2:
				printf("permanently protected\n");
				break;
			default:
				printf("in reserved protection state\n");
				break;
			}
			wp >>= 2;
		}
	}
}

static struct mmc *__init_mmc_device(int dev, bool force_init,
				     enum bus_mode speed_mode)
{
	struct mmc *mmc;
	mmc = find_mmc_device(dev);
	if (!mmc) {
		printf("no mmc device at slot %x\n", dev);
		return NULL;
	}

	if (!mmc_getcd(mmc))
		force_init = true;

	if (force_init)
		mmc->has_init = 0;

	if (IS_ENABLED(CONFIG_MMC_SPEED_MODE_SET))
		mmc->user_speed_mode = speed_mode;

	if (mmc_init(mmc))
		return NULL;

#ifdef CONFIG_BLOCK_CACHE
	struct blk_desc *bd = mmc_get_blk_desc(mmc);
	blkcache_invalidate(bd->uclass_id, bd->devnum);
#endif

	return mmc;
}

static struct mmc *init_mmc_device(int dev, bool force_init)
{
	return __init_mmc_device(dev, force_init, MMC_MODES_END);
}

static int do_mmcinfo(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	struct mmc *mmc;

	if (curr_device < 0) {
		if (get_mmc_num() > 0)
			curr_device = 0;
		else {
			puts("No MMC device available\n");
			return CMD_RET_FAILURE;
		}
	}

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	print_mmcinfo(mmc);
	return CMD_RET_SUCCESS;
}

#if CONFIG_IS_ENABLED(CMD_MMC_RPMB)
static int confirm_key_prog(void)
{
	puts("Warning: Programming authentication key can be done only once !\n"
	     "         Use this command only if you are sure of what you are doing,\n"
	     "Really perform the key programming? <y/N> ");
	if (confirm_yesno())
		return 1;

	puts("Authentication key programming aborted\n");
	return 0;
}

static int do_mmcrpmb_key(struct cmd_tbl *cmdtp, int flag,
			  int argc, char *const argv[])
{
	void *key_addr;
	struct mmc *mmc = find_mmc_device(curr_device);

	if (argc != 2)
		return CMD_RET_USAGE;

	key_addr = (void *)hextoul(argv[1], NULL);
	if (!confirm_key_prog())
		return CMD_RET_FAILURE;
	if (mmc_rpmb_set_key(mmc, key_addr)) {
		printf("ERROR - Key already programmed ?\n");
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

static int do_mmcrpmb_read(struct cmd_tbl *cmdtp, int flag,
			   int argc, char *const argv[])
{
	u16 blk, cnt;
	void *addr;
	int n;
	void *key_addr = NULL;
	struct mmc *mmc = find_mmc_device(curr_device);

	if (argc < 4)
		return CMD_RET_USAGE;

	addr = (void *)hextoul(argv[1], NULL);
	blk = hextoul(argv[2], NULL);
	cnt = hextoul(argv[3], NULL);

	if (argc == 5)
		key_addr = (void *)hextoul(argv[4], NULL);

	printf("\nMMC RPMB read: dev # %d, block # %d, count %d ... ",
	       curr_device, blk, cnt);
	n =  mmc_rpmb_read(mmc, addr, blk, cnt, key_addr);

	printf("%d RPMB blocks read: %s\n", n, (n == cnt) ? "OK" : "ERROR");
	if (n != cnt)
		return CMD_RET_FAILURE;
	return CMD_RET_SUCCESS;
}

static int do_mmcrpmb_write(struct cmd_tbl *cmdtp, int flag,
			    int argc, char *const argv[])
{
	u16 blk, cnt;
	void *addr;
	int n;
	void *key_addr;
	struct mmc *mmc = find_mmc_device(curr_device);

	if (argc != 5)
		return CMD_RET_USAGE;

	addr = (void *)hextoul(argv[1], NULL);
	blk = hextoul(argv[2], NULL);
	cnt = hextoul(argv[3], NULL);
	key_addr = (void *)hextoul(argv[4], NULL);

	printf("\nMMC RPMB write: dev # %d, block # %d, count %d ... ",
	       curr_device, blk, cnt);
	n =  mmc_rpmb_write(mmc, addr, blk, cnt, key_addr);

	printf("%d RPMB blocks written: %s\n", n, (n == cnt) ? "OK" : "ERROR");
	if (n != cnt)
		return CMD_RET_FAILURE;
	return CMD_RET_SUCCESS;
}

static int do_mmcrpmb_counter(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	unsigned long counter;
	struct mmc *mmc = find_mmc_device(curr_device);

	if (mmc_rpmb_get_counter(mmc, &counter))
		return CMD_RET_FAILURE;
	printf("RPMB Write counter= %lx\n", counter);
	return CMD_RET_SUCCESS;
}

static struct cmd_tbl cmd_rpmb[] = {
	U_BOOT_CMD_MKENT(key, 2, 0, do_mmcrpmb_key, "", ""),
	U_BOOT_CMD_MKENT(read, 5, 1, do_mmcrpmb_read, "", ""),
	U_BOOT_CMD_MKENT(write, 5, 0, do_mmcrpmb_write, "", ""),
	U_BOOT_CMD_MKENT(counter, 1, 1, do_mmcrpmb_counter, "", ""),
};

static int do_mmcrpmb(struct cmd_tbl *cmdtp, int flag,
		      int argc, char *const argv[])
{
	struct cmd_tbl *cp;
	struct mmc *mmc;
	char original_part;
	int ret;

	cp = find_cmd_tbl(argv[1], cmd_rpmb, ARRAY_SIZE(cmd_rpmb));

	/* Drop the rpmb subcommand */
	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	if (!(mmc->version & MMC_VERSION_MMC)) {
		printf("It is not an eMMC device\n");
		return CMD_RET_FAILURE;
	}
	if (mmc->version < MMC_VERSION_4_41) {
		printf("RPMB not supported before version 4.41\n");
		return CMD_RET_FAILURE;
	}
	/* Switch to the RPMB partition */
#ifndef CONFIG_BLK
	original_part = mmc->block_dev.hwpart;
#else
	original_part = mmc_get_blk_desc(mmc)->hwpart;
#endif
	if (blk_select_hwpart_devnum(UCLASS_MMC, curr_device, MMC_PART_RPMB) !=
	    0)
		return CMD_RET_FAILURE;
	ret = cp->cmd(cmdtp, flag, argc, argv);

	/* Return to original partition */
	if (blk_select_hwpart_devnum(UCLASS_MMC, curr_device, original_part) !=
	    0)
		return CMD_RET_FAILURE;
	return ret;
}
#endif

static int do_mmc_read(struct cmd_tbl *cmdtp, int flag,
		       int argc, char *const argv[])
{
	struct mmc *mmc;
	u32 blk, cnt, n;
	void *addr;

	if (argc != 4)
		return CMD_RET_USAGE;

	addr = (void *)hextoul(argv[1], NULL);
	blk = hextoul(argv[2], NULL);
	cnt = hextoul(argv[3], NULL);

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	printf("\nMMC read: dev # %d, block # %d, count %d ... ",
	       curr_device, blk, cnt);

	n = blk_dread(mmc_get_blk_desc(mmc), blk, cnt, addr);
	printf("%d blocks read: %s\n", n, (n == cnt) ? "OK" : "ERROR");

	return (n == cnt) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

#if CONFIG_IS_ENABLED(CMD_MMC_SWRITE)
static lbaint_t mmc_sparse_write(struct sparse_storage *info, lbaint_t blk,
				 lbaint_t blkcnt, const void *buffer)
{
	struct blk_desc *dev_desc = info->priv;

	return blk_dwrite(dev_desc, blk, blkcnt, buffer);
}

static lbaint_t mmc_sparse_reserve(struct sparse_storage *info,
				   lbaint_t blk, lbaint_t blkcnt)
{
	return blkcnt;
}

static int do_mmc_sparse_write(struct cmd_tbl *cmdtp, int flag,
			       int argc, char *const argv[])
{
	struct sparse_storage sparse;
	struct blk_desc *dev_desc;
	struct mmc *mmc;
	char dest[11];
	void *addr;
	u32 blk;

	if (argc != 3)
		return CMD_RET_USAGE;

	addr = (void *)hextoul(argv[1], NULL);
	blk = hextoul(argv[2], NULL);

	if (!is_sparse_image(addr)) {
		printf("Not a sparse image\n");
		return CMD_RET_FAILURE;
	}

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	printf("\nMMC Sparse write: dev # %d, block # %d ... ",
	       curr_device, blk);

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return CMD_RET_FAILURE;
	}

	dev_desc = mmc_get_blk_desc(mmc);
	sparse.priv = dev_desc;
	sparse.blksz = 512;
	sparse.start = blk;
	sparse.size = dev_desc->lba - blk;
	sparse.write = mmc_sparse_write;
	sparse.reserve = mmc_sparse_reserve;
	sparse.mssg = NULL;
	sprintf(dest, "0x" LBAF, sparse.start * sparse.blksz);

	if (write_sparse_image(&sparse, dest, addr, NULL))
		return CMD_RET_FAILURE;
	else
		return CMD_RET_SUCCESS;
}
#endif

#if CONFIG_IS_ENABLED(MMC_WRITE)
static int do_mmc_write(struct cmd_tbl *cmdtp, int flag,
			int argc, char *const argv[])
{
	struct mmc *mmc;
	u32 blk, cnt, n;
	void *addr;

	if (argc != 4)
		return CMD_RET_USAGE;

	addr = (void *)hextoul(argv[1], NULL);
	blk = hextoul(argv[2], NULL);
	cnt = hextoul(argv[3], NULL);

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	printf("\nMMC write: dev # %d, block # %d, count %d ... ",
	       curr_device, blk, cnt);

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return CMD_RET_FAILURE;
	}
	n = blk_dwrite(mmc_get_blk_desc(mmc), blk, cnt, addr);
	printf("%d blocks written: %s\n", n, (n == cnt) ? "OK" : "ERROR");

	return (n == cnt) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

static int do_mmc_erase(struct cmd_tbl *cmdtp, int flag,
			int argc, char *const argv[])
{
	struct mmc *mmc;
	u32 blk, cnt, n;

	if (argc != 3)
		return CMD_RET_USAGE;

	blk = hextoul(argv[1], NULL);
	cnt = hextoul(argv[2], NULL);

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	printf("\nMMC erase: dev # %d, block # %d, count %d ... ",
	       curr_device, blk, cnt);

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return CMD_RET_FAILURE;
	}
	n = blk_derase(mmc_get_blk_desc(mmc), blk, cnt);
	printf("%d blocks erased: %s\n", n, (n == cnt) ? "OK" : "ERROR");

	return (n == cnt) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}
#endif

static int do_mmc_rescan(struct cmd_tbl *cmdtp, int flag,
			 int argc, char *const argv[])
{
	struct mmc *mmc;

	if (argc == 1) {
		mmc = init_mmc_device(curr_device, true);
	} else if (argc == 2) {
		enum bus_mode speed_mode;

		speed_mode = (int)dectoul(argv[1], NULL);
		mmc = __init_mmc_device(curr_device, true, speed_mode);
	} else {
		return CMD_RET_USAGE;
	}

	if (!mmc)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_mmc_part(struct cmd_tbl *cmdtp, int flag,
		       int argc, char *const argv[])
{
	struct blk_desc *mmc_dev;
	struct mmc *mmc;

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	mmc_dev = blk_get_devnum_by_uclass_id(UCLASS_MMC, curr_device);
	if (mmc_dev != NULL && mmc_dev->type != DEV_TYPE_UNKNOWN) {
		part_print(mmc_dev);
		return CMD_RET_SUCCESS;
	}

	puts("get mmc type error!\n");
	return CMD_RET_FAILURE;
}

static int do_mmc_dev(struct cmd_tbl *cmdtp, int flag,
		      int argc, char *const argv[])
{
	int dev, part = 0, ret;
	struct mmc *mmc;

	if (argc == 1) {
		dev = curr_device;
		mmc = init_mmc_device(dev, true);
	} else if (argc == 2) {
		dev = (int)dectoul(argv[1], NULL);
		mmc = init_mmc_device(dev, true);
	} else if (argc == 3) {
		dev = (int)dectoul(argv[1], NULL);
		part = (int)dectoul(argv[2], NULL);
		if (part > PART_ACCESS_MASK) {
			printf("#part_num shouldn't be larger than %d\n",
			       PART_ACCESS_MASK);
			return CMD_RET_FAILURE;
		}
		mmc = init_mmc_device(dev, true);
	} else if (argc == 4) {
		enum bus_mode speed_mode;

		dev = (int)dectoul(argv[1], NULL);
		part = (int)dectoul(argv[2], NULL);
		if (part > PART_ACCESS_MASK) {
			printf("#part_num shouldn't be larger than %d\n",
			       PART_ACCESS_MASK);
			return CMD_RET_FAILURE;
		}
		speed_mode = (int)dectoul(argv[3], NULL);
		mmc = __init_mmc_device(dev, true, speed_mode);
	} else {
		return CMD_RET_USAGE;
	}

	if (!mmc)
		return CMD_RET_FAILURE;

	ret = blk_select_hwpart_devnum(UCLASS_MMC, dev, part);
	printf("switch to partitions #%d, %s\n",
	       part, (!ret) ? "OK" : "ERROR");
	if (ret)
		return 1;

	curr_device = dev;
	if (mmc->part_config == MMCPART_NOAVAILABLE)
		printf("mmc%d is current device\n", curr_device);
	else
		printf("mmc%d(part %d) is current device\n",
		       curr_device, mmc_get_blk_desc(mmc)->hwpart);

	return CMD_RET_SUCCESS;
}

static int do_mmc_list(struct cmd_tbl *cmdtp, int flag,
		       int argc, char *const argv[])
{
	print_mmc_devices('\n');
	return CMD_RET_SUCCESS;
}

#if CONFIG_IS_ENABLED(MMC_HW_PARTITIONING)
static void parse_hwpart_user_enh_size(struct mmc *mmc,
				       struct mmc_hwpart_conf *pconf,
				       char *argv)
{
	int i, ret;

	pconf->user.enh_size = 0;

	if (!strcmp(argv, "-"))	{ /* The rest of eMMC */
		ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);
		ret = mmc_send_ext_csd(mmc, ext_csd);
		if (ret)
			return;
		/* The enh_size value is in 512B block units */
		pconf->user.enh_size =
			((ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT + 2] << 16) +
			(ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT + 1] << 8) +
			ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT]) * 1024 *
			ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] *
			ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
		pconf->user.enh_size -= pconf->user.enh_start;
		for (i = 0; i < ARRAY_SIZE(mmc->capacity_gp); i++) {
			/*
			 * If the eMMC already has GP partitions set,
			 * subtract their size from the maximum USER
			 * partition size.
			 *
			 * Else, if the command was used to configure new
			 * GP partitions, subtract their size from maximum
			 * USER partition size.
			 */
			if (mmc->capacity_gp[i]) {
				/* The capacity_gp is in 1B units */
				pconf->user.enh_size -= mmc->capacity_gp[i] >> 9;
			} else if (pconf->gp_part[i].size) {
				/* The gp_part[].size is in 512B units */
				pconf->user.enh_size -= pconf->gp_part[i].size;
			}
		}
	} else {
		pconf->user.enh_size = dectoul(argv, NULL);
	}
}

static int parse_hwpart_user(struct mmc *mmc, struct mmc_hwpart_conf *pconf,
			     int argc, char *const argv[])
{
	int i = 0;

	memset(&pconf->user, 0, sizeof(pconf->user));

	while (i < argc) {
		if (!strcmp(argv[i], "enh")) {
			if (i + 2 >= argc)
				return -1;
			pconf->user.enh_start =
				dectoul(argv[i + 1], NULL);
			parse_hwpart_user_enh_size(mmc, pconf, argv[i + 2]);
			i += 3;
		} else if (!strcmp(argv[i], "wrrel")) {
			if (i + 1 >= argc)
				return -1;
			pconf->user.wr_rel_change = 1;
			if (!strcmp(argv[i+1], "on"))
				pconf->user.wr_rel_set = 1;
			else if (!strcmp(argv[i+1], "off"))
				pconf->user.wr_rel_set = 0;
			else
				return -1;
			i += 2;
		} else {
			break;
		}
	}
	return i;
}

static int parse_hwpart_gp(struct mmc_hwpart_conf *pconf, int pidx,
			   int argc, char *const argv[])
{
	int i;

	memset(&pconf->gp_part[pidx], 0, sizeof(pconf->gp_part[pidx]));

	if (1 >= argc)
		return -1;
	pconf->gp_part[pidx].size = dectoul(argv[0], NULL);

	i = 1;
	while (i < argc) {
		if (!strcmp(argv[i], "enh")) {
			pconf->gp_part[pidx].enhanced = 1;
			i += 1;
		} else if (!strcmp(argv[i], "wrrel")) {
			if (i + 1 >= argc)
				return -1;
			pconf->gp_part[pidx].wr_rel_change = 1;
			if (!strcmp(argv[i+1], "on"))
				pconf->gp_part[pidx].wr_rel_set = 1;
			else if (!strcmp(argv[i+1], "off"))
				pconf->gp_part[pidx].wr_rel_set = 0;
			else
				return -1;
			i += 2;
		} else {
			break;
		}
	}
	return i;
}

static int do_mmc_hwpartition(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	struct mmc *mmc;
	struct mmc_hwpart_conf pconf = { };
	enum mmc_hwpart_conf_mode mode = MMC_HWPART_CONF_CHECK;
	int i, r, pidx;

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	if (IS_SD(mmc)) {
		puts("SD doesn't support partitioning\n");
		return CMD_RET_FAILURE;
	}

	if (argc < 1)
		return CMD_RET_USAGE;
	i = 1;
	while (i < argc) {
		if (!strcmp(argv[i], "user")) {
			i++;
			r = parse_hwpart_user(mmc, &pconf, argc - i, &argv[i]);
			if (r < 0)
				return CMD_RET_USAGE;
			i += r;
		} else if (!strncmp(argv[i], "gp", 2) &&
			   strlen(argv[i]) == 3 &&
			   argv[i][2] >= '1' && argv[i][2] <= '4') {
			pidx = argv[i][2] - '1';
			i++;
			r = parse_hwpart_gp(&pconf, pidx, argc-i, &argv[i]);
			if (r < 0)
				return CMD_RET_USAGE;
			i += r;
		} else if (!strcmp(argv[i], "check")) {
			mode = MMC_HWPART_CONF_CHECK;
			i++;
		} else if (!strcmp(argv[i], "set")) {
			mode = MMC_HWPART_CONF_SET;
			i++;
		} else if (!strcmp(argv[i], "complete")) {
			mode = MMC_HWPART_CONF_COMPLETE;
			i++;
		} else {
			return CMD_RET_USAGE;
		}
	}

	puts("Partition configuration:\n");
	if (pconf.user.enh_size) {
		puts("\tUser Enhanced Start: ");
		print_size(((u64)pconf.user.enh_start) << 9, "\n");
		puts("\tUser Enhanced Size: ");
		print_size(((u64)pconf.user.enh_size) << 9, "\n");
	} else {
		puts("\tNo enhanced user data area\n");
	}
	if (pconf.user.wr_rel_change)
		printf("\tUser partition write reliability: %s\n",
		       pconf.user.wr_rel_set ? "on" : "off");
	for (pidx = 0; pidx < 4; pidx++) {
		if (pconf.gp_part[pidx].size) {
			printf("\tGP%i Capacity: ", pidx+1);
			print_size(((u64)pconf.gp_part[pidx].size) << 9,
				   pconf.gp_part[pidx].enhanced ?
				   " ENH\n" : "\n");
		} else {
			printf("\tNo GP%i partition\n", pidx+1);
		}
		if (pconf.gp_part[pidx].wr_rel_change)
			printf("\tGP%i write reliability: %s\n", pidx+1,
			       pconf.gp_part[pidx].wr_rel_set ? "on" : "off");
	}

	if (!mmc_hwpart_config(mmc, &pconf, mode)) {
		if (mode == MMC_HWPART_CONF_COMPLETE)
			puts("Partitioning successful, "
			     "power-cycle to make effective\n");
		return CMD_RET_SUCCESS;
	} else {
		puts("Failed!\n");
		return CMD_RET_FAILURE;
	}
}
#endif

#ifdef CONFIG_SUPPORT_EMMC_BOOT
static int do_mmc_bootbus(struct cmd_tbl *cmdtp, int flag,
			  int argc, char *const argv[])
{
	int dev;
	struct mmc *mmc;
	u8 width, reset, mode;

	if (argc != 5)
		return CMD_RET_USAGE;
	dev = dectoul(argv[1], NULL);
	width = dectoul(argv[2], NULL);
	reset = dectoul(argv[3], NULL);
	mode = dectoul(argv[4], NULL);

	mmc = init_mmc_device(dev, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	if (IS_SD(mmc)) {
		puts("BOOT_BUS_WIDTH only exists on eMMC\n");
		return CMD_RET_FAILURE;
	}

	/*
	 * BOOT_BUS_CONDITIONS[177]
	 * BOOT_MODE[4:3]
	 * 0x0 : Use SDR + Backward compatible timing in boot operation
	 * 0x1 : Use SDR + High Speed Timing in boot operation mode
	 * 0x2 : Use DDR in boot operation
	 * RESET_BOOT_BUS_CONDITIONS
	 * 0x0 : Reset bus width to x1, SDR, Backward compatible
	 * 0x1 : Retain BOOT_BUS_WIDTH and BOOT_MODE
	 * BOOT_BUS_WIDTH
	 * 0x0 : x1(sdr) or x4 (ddr) buswidth
	 * 0x1 : x4(sdr/ddr) buswith
	 * 0x2 : x8(sdr/ddr) buswith
	 *
	 */
	if (width >= 0x3) {
		printf("boot_bus_width %d is invalid\n", width);
		return CMD_RET_FAILURE;
	}

	if (reset >= 0x2) {
		printf("reset_boot_bus_width %d is invalid\n", reset);
		return CMD_RET_FAILURE;
	}

	if (mode >= 0x3) {
		printf("reset_boot_bus_width %d is invalid\n", mode);
		return CMD_RET_FAILURE;
	}

	/* acknowledge to be sent during boot operation */
	if (mmc_set_boot_bus_width(mmc, width, reset, mode)) {
		puts("BOOT_BUS_WIDTH is failed to change.\n");
		return CMD_RET_FAILURE;
	}

	printf("Set to BOOT_BUS_WIDTH = 0x%x, RESET = 0x%x, BOOT_MODE = 0x%x\n",
			width, reset, mode);
	return CMD_RET_SUCCESS;
}

static int do_mmc_boot_resize(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	int dev;
	struct mmc *mmc;
	u32 bootsize, rpmbsize;

	if (argc != 4)
		return CMD_RET_USAGE;
	dev = dectoul(argv[1], NULL);
	bootsize = dectoul(argv[2], NULL);
	rpmbsize = dectoul(argv[3], NULL);

	mmc = init_mmc_device(dev, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	if (IS_SD(mmc)) {
		printf("It is not an eMMC device\n");
		return CMD_RET_FAILURE;
	}

	if (mmc_boot_partition_size_change(mmc, bootsize, rpmbsize)) {
		printf("EMMC boot partition Size change Failed.\n");
		return CMD_RET_FAILURE;
	}

	printf("EMMC boot partition Size %d MB\n", bootsize);
	printf("EMMC RPMB partition Size %d MB\n", rpmbsize);
	return CMD_RET_SUCCESS;
}

static int mmc_partconf_print(struct mmc *mmc, const char *varname)
{
	u8 ack, access, part;

	if (mmc->part_config == MMCPART_NOAVAILABLE) {
		printf("No part_config info for ver. 0x%x\n", mmc->version);
		return CMD_RET_FAILURE;
	}

	access = EXT_CSD_EXTRACT_PARTITION_ACCESS(mmc->part_config);
	ack = EXT_CSD_EXTRACT_BOOT_ACK(mmc->part_config);
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);

	if(varname)
		env_set_hex(varname, part);

	printf("EXT_CSD[179], PARTITION_CONFIG:\n"
		"BOOT_ACK: 0x%x\n"
		"BOOT_PARTITION_ENABLE: 0x%x\n"
		"PARTITION_ACCESS: 0x%x\n", ack, part, access);

	return CMD_RET_SUCCESS;
}

static int do_mmc_partconf(struct cmd_tbl *cmdtp, int flag,
			   int argc, char *const argv[])
{
	int ret, dev;
	struct mmc *mmc;
	u8 ack, part_num, access;

	if (argc != 2 && argc != 3 && argc != 5)
		return CMD_RET_USAGE;

	dev = dectoul(argv[1], NULL);

	mmc = init_mmc_device(dev, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	if (IS_SD(mmc)) {
		puts("PARTITION_CONFIG only exists on eMMC\n");
		return CMD_RET_FAILURE;
	}

	if (argc == 2 || argc == 3)
		return mmc_partconf_print(mmc, cmd_arg2(argc, argv));

	ack = dectoul(argv[2], NULL);
	part_num = dectoul(argv[3], NULL);
	access = dectoul(argv[4], NULL);

	/* acknowledge to be sent during boot operation */
	ret = mmc_set_part_conf(mmc, ack, part_num, access);
	if (ret != 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_mmc_rst_func(struct cmd_tbl *cmdtp, int flag,
			   int argc, char *const argv[])
{
	int ret, dev;
	struct mmc *mmc;
	u8 enable;

	/*
	 * Set the RST_n_ENABLE bit of RST_n_FUNCTION
	 * The only valid values are 0x0, 0x1 and 0x2 and writing
	 * a value of 0x1 or 0x2 sets the value permanently.
	 */
	if (argc != 3)
		return CMD_RET_USAGE;

	dev = dectoul(argv[1], NULL);
	enable = dectoul(argv[2], NULL);

	if (enable > 2) {
		puts("Invalid RST_n_ENABLE value\n");
		return CMD_RET_USAGE;
	}

	mmc = init_mmc_device(dev, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	if (IS_SD(mmc)) {
		puts("RST_n_FUNCTION only exists on eMMC\n");
		return CMD_RET_FAILURE;
	}

	ret = mmc_set_rst_n_function(mmc, enable);
	if (ret != 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}
#endif
static int do_mmc_setdsr(struct cmd_tbl *cmdtp, int flag,
			 int argc, char *const argv[])
{
	struct mmc *mmc;
	u32 val;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;
	val = hextoul(argv[1], NULL);

	mmc = find_mmc_device(curr_device);
	if (!mmc) {
		printf("no mmc device at slot %x\n", curr_device);
		return CMD_RET_FAILURE;
	}
	ret = mmc_set_dsr(mmc, val);
	printf("set dsr %s\n", (!ret) ? "OK, force rescan" : "ERROR");
	if (!ret) {
		mmc->has_init = 0;
		if (mmc_init(mmc))
			return CMD_RET_FAILURE;
		else
			return CMD_RET_SUCCESS;
	}
	return ret;
}

#ifdef CONFIG_CMD_BKOPS_ENABLE
static int mmc_bkops_common(char *device, bool autobkops, bool enable)
{
	struct mmc *mmc;
	int dev;

	dev = dectoul(device, NULL);

	mmc = init_mmc_device(dev, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	if (IS_SD(mmc)) {
		puts("BKOPS_EN only exists on eMMC\n");
		return CMD_RET_FAILURE;
	}

	return mmc_set_bkops_enable(mmc, autobkops, enable);
}

static int do_mmc_bkops(struct cmd_tbl *cmdtp, int flag,
			int argc, char * const argv[])
{
	bool autobkops, enable;

	if (argc != 4)
		return CMD_RET_USAGE;

	if (!strcmp(argv[2], "manual"))
		autobkops = false;
	else if (!strcmp(argv[2], "auto"))
		autobkops = true;
	else
		return CMD_RET_FAILURE;

	if (!strcmp(argv[3], "disable"))
		enable = false;
	else if (!strcmp(argv[3], "enable"))
		enable = true;
	else
		return CMD_RET_FAILURE;

	return mmc_bkops_common(argv[1], autobkops, enable);
}

static int do_mmc_bkops_enable(struct cmd_tbl *cmdtp, int flag,
			       int argc, char * const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	return mmc_bkops_common(argv[1], false, true);
}
#endif

static int do_mmc_boot_wp(struct cmd_tbl *cmdtp, int flag,
			  int argc, char * const argv[])
{
	int err;
	struct mmc *mmc;
	int part;

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;
	if (IS_SD(mmc)) {
		printf("It is not an eMMC device\n");
		return CMD_RET_FAILURE;
	}

	if (argc == 2) {
		part = dectoul(argv[1], NULL);
		err = mmc_boot_wp_single_partition(mmc, part);
	} else {
		err = mmc_boot_wp(mmc);
	}

	if (err)
		return CMD_RET_FAILURE;
	printf("boot areas protected\n");
	return CMD_RET_SUCCESS;
}

#if CONFIG_IS_ENABLED(CMD_MMC_REG)
static int do_mmc_reg(struct cmd_tbl *cmdtp, int flag,
		      int argc, char *const argv[])
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);
	struct mmc *mmc;
	int i, ret;
	u32 off;

	if (argc < 3 || argc > 5)
		return CMD_RET_USAGE;

	mmc = find_mmc_device(curr_device);
	if (!mmc) {
		printf("no mmc device at slot %x\n", curr_device);
		return CMD_RET_FAILURE;
	}

	if (IS_SD(mmc)) {
		printf("SD registers are not supported\n");
		return CMD_RET_FAILURE;
	}

	off = simple_strtoul(argv[3], NULL, 10);
	if (!strcmp(argv[2], "cid")) {
		if (off > 3)
			return CMD_RET_USAGE;
		printf("CID[%i]: 0x%08x\n", off, mmc->cid[off]);
		if (argv[4])
			env_set_hex(argv[4], mmc->cid[off]);
		return CMD_RET_SUCCESS;
	}
	if (!strcmp(argv[2], "csd")) {
		if (off > 3)
			return CMD_RET_USAGE;
		printf("CSD[%i]: 0x%08x\n", off, mmc->csd[off]);
		if (argv[4])
			env_set_hex(argv[4], mmc->csd[off]);
		return CMD_RET_SUCCESS;
	}
	if (!strcmp(argv[2], "dsr")) {
		printf("DSR: 0x%08x\n", mmc->dsr);
		if (argv[4])
			env_set_hex(argv[4], mmc->dsr);
		return CMD_RET_SUCCESS;
	}
	if (!strcmp(argv[2], "ocr")) {
		printf("OCR: 0x%08x\n", mmc->ocr);
		if (argv[4])
			env_set_hex(argv[4], mmc->ocr);
		return CMD_RET_SUCCESS;
	}
	if (!strcmp(argv[2], "rca")) {
		printf("RCA: 0x%08x\n", mmc->rca);
		if (argv[4])
			env_set_hex(argv[4], mmc->rca);
		return CMD_RET_SUCCESS;
	}
	if (!strcmp(argv[2], "extcsd") &&
	    mmc->version >= MMC_VERSION_4_41) {
		ret = mmc_send_ext_csd(mmc, ext_csd);
		if (ret)
			return CMD_RET_FAILURE;
		if (!strcmp(argv[3], "all")) {
			/* Dump the entire register */
			printf("EXT_CSD:");
			for (i = 0; i < MMC_MAX_BLOCK_LEN; i++) {
				if (!(i % 10))
					printf("\n%03i: ", i);
				printf(" %02x", ext_csd[i]);
			}
			printf("\n");
			return CMD_RET_SUCCESS;
		}
		off = simple_strtoul(argv[3], NULL, 10);
		if (off > 512)
			return CMD_RET_USAGE;
		printf("EXT_CSD[%i]: 0x%02x\n", off, ext_csd[off]);
		if (argv[4])
			env_set_hex(argv[4], ext_csd[off]);
		return CMD_RET_SUCCESS;
	}

	return CMD_RET_FAILURE;
}
#endif

static struct cmd_tbl cmd_mmc[] = {
	U_BOOT_CMD_MKENT(info, 1, 0, do_mmcinfo, "", ""),
	U_BOOT_CMD_MKENT(read, 4, 1, do_mmc_read, "", ""),
	U_BOOT_CMD_MKENT(wp, 2, 0, do_mmc_boot_wp, "", ""),
#if CONFIG_IS_ENABLED(MMC_WRITE)
	U_BOOT_CMD_MKENT(write, 4, 0, do_mmc_write, "", ""),
	U_BOOT_CMD_MKENT(erase, 3, 0, do_mmc_erase, "", ""),
#endif
#if CONFIG_IS_ENABLED(CMD_MMC_SWRITE)
	U_BOOT_CMD_MKENT(swrite, 3, 0, do_mmc_sparse_write, "", ""),
#endif
	U_BOOT_CMD_MKENT(rescan, 2, 1, do_mmc_rescan, "", ""),
	U_BOOT_CMD_MKENT(part, 1, 1, do_mmc_part, "", ""),
	U_BOOT_CMD_MKENT(dev, 4, 0, do_mmc_dev, "", ""),
	U_BOOT_CMD_MKENT(list, 1, 1, do_mmc_list, "", ""),
#if CONFIG_IS_ENABLED(MMC_HW_PARTITIONING)
	U_BOOT_CMD_MKENT(hwpartition, 28, 0, do_mmc_hwpartition, "", ""),
#endif
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	U_BOOT_CMD_MKENT(bootbus, 5, 0, do_mmc_bootbus, "", ""),
	U_BOOT_CMD_MKENT(bootpart-resize, 4, 0, do_mmc_boot_resize, "", ""),
	U_BOOT_CMD_MKENT(partconf, 5, 0, do_mmc_partconf, "", ""),
	U_BOOT_CMD_MKENT(rst-function, 3, 0, do_mmc_rst_func, "", ""),
#endif
#if CONFIG_IS_ENABLED(CMD_MMC_RPMB)
	U_BOOT_CMD_MKENT(rpmb, CONFIG_SYS_MAXARGS, 1, do_mmcrpmb, "", ""),
#endif
	U_BOOT_CMD_MKENT(setdsr, 2, 0, do_mmc_setdsr, "", ""),
#ifdef CONFIG_CMD_BKOPS_ENABLE
	U_BOOT_CMD_MKENT(bkops-enable, 2, 0, do_mmc_bkops_enable, "", ""),
	U_BOOT_CMD_MKENT(bkops, 4, 0, do_mmc_bkops, "", ""),
#endif
#if CONFIG_IS_ENABLED(CMD_MMC_REG)
	U_BOOT_CMD_MKENT(reg, 5, 0, do_mmc_reg, "", ""),
#endif
};

static int do_mmcops(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_mmc, ARRAY_SIZE(cmd_mmc));

	/* Drop the mmc command */
	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	if (curr_device < 0) {
		if (get_mmc_num() > 0) {
			curr_device = 0;
		} else {
			puts("No MMC device available\n");
			return CMD_RET_FAILURE;
		}
	}
	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	mmc, 29, 1, do_mmcops,
	"MMC sub system",
	"info - display info of the current MMC device\n"
	"mmc read addr blk# cnt\n"
	"mmc write addr blk# cnt\n"
#if CONFIG_IS_ENABLED(CMD_MMC_SWRITE)
	"mmc swrite addr blk#\n"
#endif
	"mmc erase blk# cnt\n"
	"mmc rescan [mode]\n"
	"mmc part - lists available partition on current mmc device\n"
	"mmc dev [dev] [part] [mode] - show or set current mmc device [partition] and set mode\n"
	"  - the required speed mode is passed as the index from the following list\n"
	"    [MMC_LEGACY, MMC_HS, SD_HS, MMC_HS_52, MMC_DDR_52, UHS_SDR12, UHS_SDR25,\n"
	"    UHS_SDR50, UHS_DDR50, UHS_SDR104, MMC_HS_200, MMC_HS_400, MMC_HS_400_ES]\n"
	"mmc list - lists available devices\n"
	"mmc wp [PART] - power on write protect boot partitions\n"
	"  arguments:\n"
	"   PART - [0|1]\n"
	"       : 0 - first boot partition, 1 - second boot partition\n"
	"         if not assigned, write protect all boot partitions\n"
#if CONFIG_IS_ENABLED(MMC_HW_PARTITIONING)
	"mmc hwpartition <USER> <GP> <MODE> - does hardware partitioning\n"
	"  arguments (sizes in 512-byte blocks):\n"
	"   USER - <user> <enh> <start> <cnt> <wrrel> <{on|off}>\n"
	"	: sets user data area attributes\n"
	"   GP - <{gp1|gp2|gp3|gp4}> <cnt> <enh> <wrrel> <{on|off}>\n"
	"	: general purpose partition\n"
	"   MODE - <{check|set|complete}>\n"
	"	: mode, complete set partitioning completed\n"
	"  WARNING: Partitioning is a write-once setting once it is set to complete.\n"
	"  Power cycling is required to initialize partitions after set to complete.\n"
#endif
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	"mmc bootbus <dev> <boot_bus_width> <reset_boot_bus_width> <boot_mode>\n"
	" - Set the BOOT_BUS_WIDTH field of the specified device\n"
	"mmc bootpart-resize <dev> <boot part size MB> <RPMB part size MB>\n"
	" - Change sizes of boot and RPMB partitions of specified device\n"
	"mmc partconf <dev> [[varname] | [<boot_ack> <boot_partition> <partition_access>]]\n"
	" - Show or change the bits of the PARTITION_CONFIG field of the specified device\n"
	"   If showing the bits, optionally store the boot_partition field into varname\n"
	"mmc rst-function <dev> <value>\n"
	" - Change the RST_n_FUNCTION field of the specified device\n"
	"   WARNING: This is a write-once field and 0 / 1 / 2 are the only valid values.\n"
#endif
#if CONFIG_IS_ENABLED(CMD_MMC_RPMB)
	"mmc rpmb read addr blk# cnt [address of auth-key] - block size is 256 bytes\n"
	"mmc rpmb write addr blk# cnt <address of auth-key> - block size is 256 bytes\n"
	"mmc rpmb key <address of auth-key> - program the RPMB authentication key.\n"
	"mmc rpmb counter - read the value of the write counter\n"
#endif
	"mmc setdsr <value> - set DSR register value\n"
#ifdef CONFIG_CMD_BKOPS_ENABLE
	"mmc bkops-enable <dev> - enable background operations handshake on device\n"
	"   WARNING: This is a write-once setting.\n"
	"mmc bkops <dev> [auto|manual] [enable|disable]\n"
	" - configure background operations handshake on device\n"
#endif
#if CONFIG_IS_ENABLED(CMD_MMC_REG)
	"mmc reg read <reg> <offset> [env] - read card register <reg> offset <offset>\n"
	"                                    (optionally into [env] variable)\n"
	" - reg: cid/csd/dsr/ocr/rca/extcsd\n"
	" - offset: for cid/csd [0..3], for extcsd [0..511,all]\n"
#endif
	);

/* Old command kept for compatibility. Same as 'mmc info' */
U_BOOT_CMD(
	mmcinfo, 1, 0, do_mmcinfo,
	"display MMC info",
	"- display info of the current MMC device"
);
