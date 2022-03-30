/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>

#include <asm/arch/ddr.h>
#include <linux/ctype.h>
#include <mtd.h>
#include <nand.h>
#include <stdlib.h>
#include <string.h>
#include <environment.h>
#include <cli.h>
#include <linux/bitops.h>
#include <linux/crc32.h>
#include <ubi_uboot.h>
#include "bca_common.h"
#include "bca_sdk.h"
#include "reimage.h"
#include "spl_env.h"

#define MSIZE(a) ( ( ((a) & 0xc000) == 0xc000 ) ? 0 : 1024 * (a)  )

static struct reimager rei = { 0 };

DECLARE_GLOBAL_DATA_PTR;

__weak void boost_cpu_clock(void)
{
}

int board_sdk_late_init_e(void)
{
	boost_cpu_clock();

	/* CUSTOMIZE -- set default behavior here */
	env_set("bootdelay", "5");
	env_set("bootcmd", "reimage auto");
	return 0;
}

void hook_dram_init(void)
{
	int shift;
	u64 size;
#if defined(CONFIG_BCM63138) || defined(CONFIG_BCM63148) || defined(CONFIG_BCM4908) || defined(CONFIG_BCM6858)
	shift =
	    (MEMC->GLB_GCFG & MEMC_GLB_GCFG_SIZE1_MASK) >>
	    MEMC_GLB_GCFG_SIZE1_SHIFT;
#else
	shift =
	    (MEMC->GLB_FSBL_STATE & MEMC_GLB_FSBL_DRAM_SIZE_MASK) >>
	    MEMC_GLB_FSBL_DRAM_SIZE_SHIFT;
#endif
	size = 1 << shift;	// in MB
	printf("DDR size from controller %dMB\n", size);
	gd->ram_size = (phys_size_t) (size << 20);
	gd->ram_base = 0;
	gd->bd->bi_dram[0].start = 0;
	gd->bd->bi_dram[0].size = (phys_size_t) gd->ram_size;
}

static int check_nv_crc(struct nvram_s *nv);

static int check_nv_crc(struct nvram_s *nv)
{
	int orig, new;
	printf("original CRC 0x%x\n", nv->ulCheckSum);
	orig = nv->ulCheckSum;
	nv->ulCheckSum = 0;
	/* use crc32_le to get standard CRC rather than the unusual one 
	 * used for uboot environment files */
	new = crc32_le(0xffffffff, nv, 1024);
	printf("computed CRC 0x%x\n", new);
	nv->ulCheckSum = orig;
	return (new == orig);
}

static int ubi_dev_scan(struct mtd_info *info, const char *vid_header_offset);

static int ubi_dev_scan(struct mtd_info *info, const char *vid_header_offset)
{
	char ubi_mtd_param_buffer[80];
	int err;

	if (!vid_header_offset)
		sprintf(ubi_mtd_param_buffer, "%s", info->name);
	else
		sprintf(ubi_mtd_param_buffer, "%s,%s", info->name,
			vid_header_offset);

	err = ubi_mtd_param_parse(ubi_mtd_param_buffer, NULL);
	if (err)
		return -err;

	err = ubi_init();
	if (err)
		return -err;

	return 0;
}

static int erase(struct mtd_info *mtd, int first, int blocks);

static int erase(struct mtd_info *mtd, int first, int blocks)
{
	struct erase_info erase_op = { };
	int ret;

	erase_op.mtd = mtd;
	erase_op.addr = first * mtd->erasesize;
	erase_op.len = blocks * mtd->erasesize;
	erase_op.scrub = 0;
	printf("Erasing %d blocks from block %d\n", blocks, first);

	while (erase_op.len) {
		ret = mtd_erase(mtd, &erase_op);

		/* Abort if its not a bad block error */
		if (ret != -EIO)
			break;

		printf("Skipping bad block at 0x%08llx\n", erase_op.fail_addr);

		/* Skip bad block and continue behind it */
		erase_op.len -= erase_op.fail_addr - erase_op.addr;
		erase_op.len -= mtd->erasesize;
		erase_op.addr = erase_op.fail_addr + mtd->erasesize;
	}

	if (ret && ret != -EIO)
		ret = -1;
	else
		ret = 0;
	return (ret);
};

static int do_nvram_parse(cmd_tbl_t * cmdtp, int flag, int argc,
			  char *const argv[]);
static int do_prepare(cmd_tbl_t * cmdtp, int flag, int argc,
		      char *const argv[]);
static int do_finish(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);

static int do_prepare(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	struct erase_info erase_op = { };
	char more_env[1024];
	int more_env_size;
	int i, ret;
	size_t sz = 0;
	int rblk, wblk;
	char *bp;
	char *cp;
	int block;
	int err;
	long offset;
	if (!rei.payload_blocks) {
		printf("parse first\n");
		return (-1);
	}
	struct mtd_info *mtd = NULL;
	wblk = rei.burn_first_start;
	bp = rei.loader_payload =
	    malloc(rei.blocksizeK * 1024 * rei.loader_blocks);
	if (!bp) {
		printf("loader payload malloc failed\n");
		return (-1);
	}
	mtd_probe_devices();
	if (rei.pure_payload_image) {
		int count = 0;
		int leb = 0;
		int offset = 0;
		int m, s;
		m = rei.pure_payload_volume_index;
		s = rei.ubi->volumes[m]->usable_leb_size;
		cp = CONFIG_SYS_LOAD_ADDR;
		for (leb = 0;
		     leb * s < 2048 + rei.blocksizeK * 1024 * rei.loader_blocks;
		     leb++) {
			err =
			    ubi_eba_read_leb(rei.ubi, rei.ubi->volumes[m], leb,
					     cp + leb * s, 0, s, 0);
		}
		memcpy(rei.loader_payload, cp + 2048,
		       rei.blocksizeK * 1024 * rei.loader_blocks);
		printf("start of image device = 0x%x\n",
		       2048 + rei.blocksizeK * 1024 * rei.loader_blocks);
		leb = (2048 + rei.blocksizeK * 1024 * rei.loader_blocks) / s;
		offset = (2048 + rei.blocksizeK * 1024 * rei.loader_blocks) % s;
		count = 0;
		mtd = get_mtd_device_nm("nand0");
		if (IS_ERR_OR_NULL(mtd)) {
			printf("failed to get mtd\n");
			return (-1);
		}
		erase(mtd, rei.erase_first_start, rei.erase_first_blocks);
		put_mtd_device(mtd);
		while (count < rei.payload_blocks * rei.blocksizeK * 1024) {
			cp = CONFIG_SYS_LOAD_ADDR;
			while (((int)cp < CONFIG_SYS_LOAD_ADDR + SZ_16M)
			       && (count <
				   rei.payload_blocks * rei.blocksizeK *
				   1024)) {
				err =
				    ubi_eba_read_leb(rei.ubi,
						     rei.ubi->volumes[m], leb,
						     cp, offset, s - offset, 0);
				printf("R");
				// printf
				// (" leb %d off 0x%x new 0x%x count %x --> %p\n",
				// leb, offset, s - offset, count, cp);
				count = count + s - offset;
				cp = cp + s - offset;
				offset = 0;
				leb++;
			}
			// printf("final cp is %p next leb %d\n", cp, leb);
			if ((int)cp > CONFIG_SYS_LOAD_ADDR + SZ_16M) {
				offset =
				    s - ((int)cp -
					 (CONFIG_SYS_LOAD_ADDR + SZ_16M));
				leb--;
				// printf
				// ("adjusted offset to 0x%x and next leb to %d\n",
				// offset, leb);
			}
			mtd = get_mtd_device_nm("nand0");
			if (IS_ERR_OR_NULL(mtd)) {
				printf("failed to get mtd\n");
				return (-1);
			}
			cp = CONFIG_SYS_LOAD_ADDR;
			while (((int)cp + mtd->erasesize) <=
			       CONFIG_SYS_LOAD_ADDR + SZ_16M) {
				if (wblk <
				    (rei.burn_first_start +
				     rei.burn_first_blocks)) {
					if (!mtd_block_isbad
					    (mtd, wblk * mtd->erasesize)) {
						i = mtd_write(mtd,
							      wblk *
							      mtd->erasesize,
							      mtd->erasesize,
							      &sz, cp);
						printf("W");
						// printf
						// (" 0x%x bytes at %p -> blk %d\n",
						// sz, cp, wblk);
						cp += sz;
					} else {
						printf("bad");
					}
					wblk++;
				} else {
					printf
					    ("\nFIXME -- remainder not implemented\n");
				}
			}
			put_mtd_device(mtd);
		}
	}

	else {
		mtd = get_mtd_device_nm("nand0");
		if (IS_ERR_OR_NULL(mtd)) {
			printf("failed to get mtd\n");
			return (-1);
		}
		// FIXME -- loop on payload until we have enough data
		//  first --> loader (DDR)
		//  next --> burn_first (until exhausted)
		//  finally --> allocate remainder and copy to DDR
		rblk = rei.payload_start;
		while (bp <
		       (rei.loader_payload +
			mtd->erasesize * rei.loader_blocks)) {
			i = mtd_read(mtd, rblk * mtd->erasesize, mtd->erasesize,
				     &sz,
				     rei.loader_payload +
				     mtd->erasesize * (rblk -
						       rei.payload_start));
			printf("read loader -> mem %d bytes ret %d\n", sz, i);
			rblk++;
			bp += sz;
		}
		printf("loader in memory buffer at 0x%x and size 0x%x\n",
		       rei.loader_payload, mtd->erasesize * rei.loader_blocks);
		erase(mtd, rei.erase_first_start, rei.erase_first_blocks);
		// rblk = rei.payload_start + rei.loader_blocks;
		block = rei.loader_blocks;
		while (block < (rei.loader_blocks + rei.payload_blocks)) {
			i = mtd_read(mtd, rblk * mtd->erasesize, mtd->erasesize,
				     &sz, (char *)CONFIG_SYS_LOAD_ADDR);
			printf("R");
			if (sz) {
				block++;
				i = 1;
				while (i) {
					if (wblk <
					    (rei.burn_first_start +
					     rei.burn_first_blocks)) {
						if (!mtd_block_isbad
						    (mtd,
						     wblk * mtd->erasesize)) {
							i = mtd_write(mtd,
								      wblk *
								      mtd->erasesize,
								      mtd->erasesize,
								      &sz,
								      (char *)
								      CONFIG_SYS_LOAD_ADDR);
							printf("W", sz, i);
						} else {
							printf("bad");
						}
						wblk++;
					} else {
						if (!rei.remaining_payload) {
							rei.remaining_payload =
							    malloc
							    (mtd->erasesize *
							     (rei.loader_blocks
							      +
							      rei.payload_blocks
							      - block + 1));
							rei.remaining_payload_len = 0;
							if (!rei.remaining_payload) {
								printf
								    ("malloc for remaining failed\n");
								return (1);
							}
						}
						memcpy(rei.remaining_payload +
						       rei.remaining_payload_len,
						       (char *)
						       CONFIG_SYS_LOAD_ADDR,
						       mtd->erasesize);
						rei.remaining_payload_len +=
						    mtd->erasesize;
						printf("M");
						i = 0;
					}
				}
			}
			rblk++;

		}
		printf("\n");
		put_mtd_device(mtd);

	}

	cp = more_env;

	cp += sprintf(cp, "ethaddr=%x:%x:%x:%x:%x:%x",
		      rei.nvram.ucaBaseMacAddr[0],
		      rei.nvram.ucaBaseMacAddr[1],
		      rei.nvram.ucaBaseMacAddr[2],
		      rei.nvram.ucaBaseMacAddr[3],
		      rei.nvram.ucaBaseMacAddr[4], rei.nvram.ucaBaseMacAddr[5]);
	*(cp++) = '\0';

	cp +=
	    sprintf(cp,
		    "bootcmd=printenv;run once;run check_flashback;printenv;sdk boot_img");
	*(cp++) = '\0';

	cp += sprintf(cp, "tries=3");
	*(cp++) = '\0';

	cp +=
	    sprintf(cp,
		    "check_flashback=test $tries -eq 0 || echo $tries ;  setexpr tries $tries - 1 ; saveenv ; test $tries -eq 0 && run do_flashback");
	*(cp++) = '\0';

	cp +=
	    sprintf(cp,
		    "do_flashback=echo here is where I would have run flashback");
	*(cp++) = '\0';

	cp += sprintf(cp, "once=sdk metadata 1 1;setenv once true;saveenv");
	*(cp++) = '\0';

	*(cp++) = '\0';
	more_env_size = cp - more_env + 1;

	for (bp = rei.loader_payload;
	     bp < rei.loader_payload + mtd->erasesize * rei.loader_blocks;
	     bp += 4096) {
		int *tenv;
		env_t *ep;
		uint32_t new, crc;
		tenv = (int *)bp;
		printf("env check at %x\r", bp - rei.loader_payload);
		if (tenv[0] == BOOT_MAGIC_MAGIC) {
			printf("\nGOT IT\n");
			ep = (env_t *) & tenv[2];
			memcpy(&crc, &ep->crc, sizeof(crc));
			/* specifically use uboot's env crc function
			 * even though we have included the standard
			 * linux crc32 */
			new = the_env_crc32(0, ep->data, tenv[1] - 4);
			if (new != crc) {
				printf("bad\n");
			} else {
				printf("good\n");

				for (i = 0; i < tenv[1] - 4; i++) {
					if ((ep->data[i] == '\0')
					    && (ep->data[i + 1] == '\0')) {
						memcpy(&ep->data[i + 1],
						       more_env, more_env_size);
						break;
					}
				}
				new = the_env_crc32(0, ep->data, tenv[1] - 4);
				memcpy(&ep->crc, &new, sizeof(new));
			}
		}
	}

	printf("\n");
	return (0);
}

static int do_finish(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	int i, ret;
	unsigned long time_start, time_mid, time_end;
	size_t sz = 0;
	int rblk, wblk;
	char *bp;
	char cmd[256];
	int block;
	long offset;
	if (!rei.payload_blocks) {
		printf("parse first\n");
		return (-1);
	}
	run_command("ubifsumount", 0);
	run_command("ubi detach", 0);
	struct mtd_info *mtd = NULL;
	mtd_probe_devices();
	mtd = get_mtd_device_nm("nand0");
	if (IS_ERR_OR_NULL(mtd)) {
		printf("failed to get mtd\n");
		return (-1);
	}
	if (!rei.loader_payload) {
		printf("loader payload isn't set\n");
		return (-1);
	}
	time_start = get_timer(0);
	erase(mtd, 0, rei.loader_blocks);
	for (i = 0; i < rei.loader_blocks; i++) {
		offset = i * mtd->erasesize;
		ret = mtd_write(mtd,
				i * mtd->erasesize,
				mtd->erasesize, &sz,
				&rei.loader_payload[i * mtd->erasesize]);
		printf("w mem->ldr %d b ret %d\n", sz, ret);
	}
	erase(mtd, rei.burn_remaining_start, rei.burn_remaining_blocks);
	wblk = rei.burn_remaining_start;
	i = 0;
	while (i < rei.remaining_payload_len) {
		printf("rp %x => %d\n", i, wblk);
		ret = mtd_write(mtd,
				wblk * mtd->erasesize,
				mtd->erasesize, &sz, &rei.remaining_payload[i]);
		i += sz;
		wblk++;
	}

	erase(mtd, rei.erase_last_start, rei.erase_last_blocks);
	put_mtd_device(mtd);
	time_mid = get_timer(0);
	sprintf(cmd, "%s:%lld(loader),%lld@%lld(image)",
		"brcmnand.0",
		(long long)(rei.loader_blocks * (long long)mtd->erasesize),
		(long long)(mtd->size -
			    (rei.loader_blocks + 8) * mtd->erasesize),
		(long long)(rei.loader_blocks * mtd->erasesize));
	run_command("mtdparts delall", 0);
	env_set("mtdparts", cmd);
	run_command("mtdparts", 0);
	if (rei.preserved_data_max_len) {
		run_command("ubi part image", 0);

		sprintf(cmd, "ubi create transition 0x%x dynamic 33",
			rei.preserved_data_max_len);
		run_command(cmd, 0);
		sprintf(cmd, "ubi write 0x%x transition 0x%x",
			rei.preserved_data, rei.preserved_data_max_len);
		run_command(cmd, 0);
	}

	time_end = get_timer(0);
	run_command("ubi detach", 0);
	printf("start %ld\nsafe  %ld\ndone  %\ld\nHZ %d\n", time_start,
	       time_mid, time_end, CONFIG_SYS_HZ);
	return (0);

}

static void set_rei_ranges(int i);

static void set_rei_ranges(int i)
{
	char *cp;
	struct nvram_s *nv;
	nv = &rei.nvram;
	printf("ranges for image %d\n", i);
	rei.erase_first_start = nv->ulNandPartOfsKb[3 - i] / rei.blocksizeK;
	rei.erase_first_blocks = nv->ulNandPartSizeKb[3 - i] / rei.blocksizeK;
	if (rei.erase_first_start < rei.loader_blocks) {
		rei.burn_first_start = rei.loader_blocks;
		rei.burn_first_blocks =
		    rei.erase_first_blocks -
		    (rei.loader_blocks - rei.erase_first_start);
	} else {
		rei.burn_first_start = rei.erase_first_start;
		rei.burn_first_blocks = rei.erase_first_blocks;
	}
	if (cp = env_get("burn_first_blocks")) {
		rei.burn_first_blocks = simple_strtoul(cp, NULL, 0);
	}
	printf("rei.erase_first_start=%d\n", rei.erase_first_start);
	printf("rei.erase_first_blocks=%d\n", rei.erase_first_blocks);
	printf("rei.burn_first_start=%d\n", rei.burn_first_start);
	printf("rei.burn_first_blocks=%d\n", rei.burn_first_blocks);
	rei.burn_remaining_start = nv->ulNandPartOfsKb[i] / rei.blocksizeK;
	rei.burn_remaining_blocks = nv->ulNandPartSizeKb[i] / rei.blocksizeK;
	if (rei.burn_remaining_start < rei.loader_blocks) {
		rei.burn_remaining_start = rei.loader_blocks;
		rei.burn_remaining_blocks =
		    rei.burn_remaining_blocks -
		    (rei.loader_blocks - rei.burn_remaining_start);
	}
}

static int do_nvram_parse(cmd_tbl_t * cmdtp, int flag, int argc,
			  char *const argv[])
{
	// int *haystack = (int *)0x1000000;
	int i, j, k, m;
	int ret = 0;
	u_char *cp;
	int blocksize;
	int err;
	long int needle;
	char split[128];
	int payload_loader_size = 0;
	int payload_size = 0;
	struct nvram_s *nv;
	u_char tmp[2048];
	// char cmd[256];
	long int *ip;
	struct mtd_info *mtd;
	size_t sz = 0;
	int crc;

	memcpy(&needle, NVRAM_MAGIC, 8);
	mtd_probe_devices();
	mtd = get_mtd_device_nm("nand0");
	if (IS_ERR_OR_NULL(mtd)) {
		printf("failed to get mtd\n");
		return (-1);
	}
	i = mtd_read(mtd, 0, 2 << 20, &sz, (u_char *) CONFIG_SYS_LOAD_ADDR);
	nv = 0x1010580;
	if (!check_nv_crc(nv)) {
		nv = NULL;
		for (ip = (long int *)CONFIG_SYS_LOAD_ADDR;
		     ip < (long int *)(CONFIG_SYS_LOAD_ADDR + SZ_2M);
		     ip = ip + (2048 / sizeof(long int))) {
			if (*ip == needle) {
				printf("got it %p\n", ip);
				nv = &ip[1];
				if (check_nv_crc(nv)) {
					printf("nvram CRC OK\n");
				} else {
					printf("nvram CRC failed\n");
				}
			}
		}
	}
	if (!nv) {
		printf("failed\n");
		return (1);
	}
	memcpy(&rei.nvram, nv, sizeof(rei.nvram));
	printf("version %d\n", nv->version);
	memset(tmp, 0, 24);
	strncpy(tmp, nv->boardid, 16);
	printf("boardid %s\n", tmp);
	printf("bootline %s\n", nv->bootline);
	printf("afeids %x,%x\n", nv->afeId[0], nv->afeId[1]);
	printf("MCB %x\n", nv->ulMemoryConfig);
	for (i = 0; i < NP_TOTAL; i++) {
		printf("part %d offset %dK size %dK\n", i,
		       nv->ulNandPartOfsKb[i], nv->ulNandPartSizeKb[i]);
	}
	memset(tmp, 0, 24);
	strncpy(tmp, nv->szVoiceBoardId, 16);
	printf("voiceboardid %s\n", tmp);

	blocksize = mtd->erasesize;
	rei.blocksizeK = mtd->erasesize >> 10;
	put_mtd_device(mtd);
	env_set("mtdids", "nand0=brcmnand.0");
	sprintf(tmp, "%s:%dK@%dK(nvram)ro,%dK@%dK(image1)ro,%dK@%dK(image2)ro,",
		"brcmnand.0",
		nv->ulNandPartSizeKb[0], nv->ulNandPartOfsKb[0],
		nv->ulNandPartSizeKb[1], nv->ulNandPartOfsKb[1],
		nv->ulNandPartSizeKb[2], nv->ulNandPartOfsKb[2]
	    );
	k = nv->ulNandPartOfsKb[3]
	    - MSIZE(nv->part_info[0].size)
	    - MSIZE(nv->part_info[1].size)
	    - MSIZE(nv->part_info[2].size);
	for (i = 0; i < 3; i++) {
		j = MSIZE(nv->part_info[i].size);
		printf("misc%d size is %dK\n", i + 1, j);
		if (j) {
			sprintf(tmp + strlen(tmp), "%dK@%dK(misc%d),",
				j, k, i + 1);
			k = k + j;
		}
	}
	sprintf(tmp + strlen(tmp), "%dK@%dK(data)",
		nv->ulNandPartSizeKb[3], nv->ulNandPartOfsKb[3]
	    );
	printf("setting mtdparts to %s\n", tmp);
	env_set("mtdparts", tmp);
	run_command("mtdparts", 0);
	sprintf(tmp, "0x%x", nv->ulNandPartSizeKb[0] * 1024);
	env_set("nvram_size", tmp);
	run_command("nand info", 0);
	printf("erase block %d\n", blocksize);
	strcpy(split, MASQ_SPLIT_A);
	strcat(split, MASQ_SPLIT_B);
	mtd_probe_devices();
	for (i = 1; i < 3; i++) {
		sprintf(tmp, "image%d", i);
		mtd = get_mtd_device_nm(tmp);
		if (IS_ERR_OR_NULL(mtd)) {
			printf("failed to get mtd\n");
			return (-1);
		}
		for (j = 0; j < 0x1000000/blocksize; j++) {
			// printf("read i=%d j=%d\n",i,j);
			ret = mtd_read(mtd, j * blocksize, 2048, &sz, tmp);
			// printf("ret %d read %d\n",ret,sz);
			if (0 == strncmp(split, tmp, strlen(split) + 1)) {
				printf("found image%d block %d\n", i, j);
				cp = &tmp[strlen(split) + 1];
				payload_loader_size =
				    simple_strtoul(cp, NULL, 0);
				cp = cp + strlen(cp) + 1;
				payload_size = simple_strtoul(cp, NULL, 0);
				printf("loader size %d payload size %d\n",
				       payload_loader_size, payload_size);
				rei.loader_blocks =
				    payload_loader_size / blocksize;
				rei.payload_blocks = payload_size / blocksize;
				rei.split_image_start =
				    (nv->ulNandPartOfsKb[i] / rei.blocksizeK);
				rei.split_image_end =
				    (nv->ulNandPartOfsKb[i] +
				     nv->ulNandPartSizeKb[i]) / rei.blocksizeK -
				    1;
				rei.payload_start =
				    (nv->ulNandPartOfsKb[i] / rei.blocksizeK) +
				    j + 1;
/////
				set_rei_ranges(i);
				i = j = 30;
				break;
			}

		}
		put_mtd_device(mtd);
		/* the following will probably be removed ...  reimage will always be packaged as a split image */
		/* if we do keep it, after attaching, it needs to check for volumes other than the rootfs */
		/* or the number of volumes */
		if ((0 == 1) && (i < 3)) {
			if (rei.ubi)
				ubi_exit();

			rei.ubi = NULL;
			/* didn't find marker yet */
			err = ubi_dev_scan(mtd, NULL);
			if (err) {
				printf("UBI init error %d\n", err);
				printf
				    ("Please check, if the correct MTD partition is used (size big enough?)\n");
			}

			rei.ubi = ubi_devices[0];

			sprintf(tmp, "image%d", i);
			if (rei.ubi) {
				printf("part %s attaches as ubi\n", tmp);
				for (m = 0; m < (rei.ubi->vtbl_slots + 1); m++) {
					if (!rei.ubi->volumes[m])
						continue;	/* Empty record */
					printf("name %s\n",
					       rei.ubi->volumes[m]->name);
					if (0 ==
					    strcmp("payload",
						   rei.ubi->volumes[m]->name)) {
						/* found payload volume */
						rei.pure_payload_image = i;
						rei.pure_payload_volume_index =
						    m;
						i = 30;
					}
				}
			}
		}
		if (rei.pure_payload_image) {
			int m, s, t;
			m = rei.pure_payload_volume_index;
			s = rei.ubi->volumes[m]->usable_leb_size;
			err =
			    ubi_eba_read_leb(rei.ubi, rei.ubi->volumes[m],
					     0, CONFIG_SYS_LOAD_ADDR, 0, s, 0);
			cp = CONFIG_SYS_LOAD_ADDR;
			t = simple_strtoul(cp, &cp, 0);
			rei.loader_blocks = t / blocksize;
			printf("loader size %d is %d blocks\n", t, blocksize);
			cp++;
			t = simple_strtoul(cp, &cp, 0);
			rei.payload_blocks = t / blocksize;
			printf("payload size %d is %d blocks\n", t, blocksize);
			set_rei_ranges(rei.pure_payload_image);

		}

	}

	nv = &rei.nvram;
	rei.erase_last_start =
	    (nv->ulNandPartOfsKb[2] + nv->ulNandPartSizeKb[2]) / rei.blocksizeK;
	rei.erase_last_blocks =
	    ((nv->ulNandPartOfsKb[3] +
	      nv->ulNandPartSizeKb[3]) / rei.blocksizeK) - rei.erase_last_start;
	return (ret);
}

static int do_preserve_allocate(cmd_tbl_t * cmdtp, int flag, int argc,
				char *const argv[]);

static int do_preserve_save(cmd_tbl_t * cmdtp, int flag, int argc,
			    char *const argv[]);
static int do_preserve_allocate(cmd_tbl_t * cmdtp, int flag, int argc,
				char *const argv[])
{
	int len;
	char *cp;
	len = simple_strtoul(argv[1], NULL, 0);
	cp = malloc(len);
	if (!cp) {
		printf("malloc failed\n");
		return (1);
	}
	cp[0] = '\0';
	rei.preserved_data = cp;
	rei.preserved_data_len = 0;
	rei.preserved_data_max_len = len;
	printf("allocated 0x%x bytes at %x\n", len, cp);
	return (0);
}

static int do_preserve_save(cmd_tbl_t * cmdtp, int flag, int argc,
			    char *const argv[])
{
	char *cp;
	int len;
	if (argc != 2) {
		printf("filename required\n");
		return (-1);
	}
	if (cp = env_get("filesize")) {
		len = simple_strtoul(cp, NULL, 16);
	} else {
		printf("filesize in env is not set\n");
		return (1);
	}
	if (rei.preserved_data_len + strlen(argv[1]) + 16 + len >
	    rei.preserved_data_max_len) {
		printf("allocated space exhausted\n");
		return (1);
	}
	rei.preserved_data_len +=
	    sprintf(rei.preserved_data + rei.preserved_data_len, "%s\n%d\n",
		    argv[1], len);
	memcpy(rei.preserved_data + rei.preserved_data_len,
	       CONFIG_SYS_LOAD_ADDR, len);
	rei.preserved_data_len += len;
	*((char *)(rei.preserved_data + rei.preserved_data_len)) = '\n';
	printf("preserved %d bytes as %s\n", len, argv[1]);
	return (0);
}

static int do_commit(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);
static int do_commit(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	struct mtd_info *mtd = NULL;
	long offset;
	int i;
	size_t sz = 0;
	int ret = 0;
	int rblk;
	if (!rei.loader_payload) {
		printf("prepare revertable first\n");
		return (-1);
	}
	mtd_probe_devices();
	mtd = get_mtd_device_nm("nand0");
	erase(mtd, rei.erase_first_start, rei.erase_first_blocks);
	erase(mtd, rei.erase_last_start, rei.erase_last_blocks);
	erase(mtd, 0, rei.loader_blocks);
	rblk = 0;
	for (i = 0; i < rei.loader_blocks; i++) {
		if (mtd_block_isbad(mtd, (uint64_t) i * mtd->erasesize)) {
			printf("skip bad block %d\n", i);
		} else {
			offset = i * mtd->erasesize;
			ret = mtd_write(mtd,
					i * mtd->erasesize,
					mtd->erasesize, &sz,
					&rei.loader_payload[rblk *
							    mtd->erasesize]);
			printf("w mem->ldr %d b ret %d\n", sz, ret);
			if (ret == 0) {
				rblk++;
			}
		}
	}
	put_mtd_device(mtd);
	return (ret);
}

static int do_revertable(cmd_tbl_t * cmdtp, int flag, int argc,
			 char *const argv[]);
static int do_revertable(cmd_tbl_t * cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct mtd_info *mtd = NULL;
	char *bp;
	int rblk;
	int i, ret = 0;
	size_t sz = 0;
	char more_env[1024];
	int more_env_size;
	// parse nvram
	if (!rei.payload_blocks) {
		printf("parse first\n");
		return (-1);
	}
	mtd_probe_devices();
	mtd = get_mtd_device_nm("nand0");
	if (IS_ERR_OR_NULL(mtd)) {
		printf("failed to get mtd\n");
		return (-1);
	}
	erase(mtd, rei.split_image_start,
	      rei.payload_start - rei.split_image_start);
	// read loader to ddr
	bp = rei.loader_payload =
	    malloc(rei.blocksizeK * 1024 * rei.loader_blocks);
	if (!bp) {
		printf("loader payload malloc failed\n");
		return (-1);
	}
	rblk = rei.payload_start;
	while (bp < (rei.loader_payload + mtd->erasesize * rei.loader_blocks)) {
		i = mtd_read(mtd, rblk * mtd->erasesize, mtd->erasesize,
			     &sz,
			     rei.loader_payload +
			     mtd->erasesize * (rblk - rei.payload_start));
		printf("read loader -> mem %d bytes ret %d\n", (int)sz, i);
		rblk++;
		bp += sz;
	}
	// erase start of reimage image to end of loader
	erase(mtd, rei.payload_start, rei.loader_blocks);
	// preserve files
	// update loader in ddr (env)
	reimage_env_append(&rei);
	put_mtd_device(mtd);
	return (ret);
}

static int do_read_recovery(cmd_tbl_t * cmdtp, int flag, int argc,
			    char *const argv[]);
static int do_read_recovery(cmd_tbl_t * cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct mtd_info *mtd = NULL;
	char *bp;
	int rblk;
	int blocks;
	int pages;
	int chunk = 0;
	size_t sz = 0;
	loff_t off;
	int i, j, n, z, ret = 0;
	u32 *up32;
	int block = 0;
	int page = 0;
	struct mtd_oob_ops oob_ops;
	u32 spare[200];
	mtd = get_mtd_device_nm("nand0");
	blocks = (mtd->size / mtd->erasesize);
	pages = (mtd->erasesize / mtd->writesize);
	rei.recovery_chunks_list = CONFIG_SYS_LOAD_ADDR;
	rei.recovery_data_buf =
	    CONFIG_SYS_LOAD_ADDR + (blocks * pages +
				    1) * sizeof(struct recovery_chunks);
	rei.recovery_data_len = 0;
	if (!rei.recovery_chunks_list) {
		printf("recovery chunks  malloc failed\n");
		return (-1);
	}
	printf("Flash device is %d blocks of %d pages of %d bytes\n", blocks,
	       pages, mtd->writesize);
	while (block < blocks) {
		if ((block >= rei.split_image_start)
		    && (block <= rei.split_image_end)) {
			// printf("block %d is part of reimage\n", block);
			block++;
			continue;
		}
		off =
		    (uint64_t) page *mtd->writesize +
		    (uint64_t) block *mtd->erasesize;
		if (page == 0) {
			if (mtd_block_isbad
			    (mtd, (uint64_t) block * mtd->erasesize)) {
				/* check for bad block */
				printf("block %d is bad\n", block);
				block++;
				continue;
			}
		}
		sz = 0;
		/* read to buffer ... may not keep it */
		bp = rei.recovery_data_buf + rei.recovery_data_len;
		oob_ops.mode = MTD_OPS_PLACE_OOB;
		oob_ops.len = mtd->writesize;
		oob_ops.retlen = 0;
		oob_ops.ooblen = mtd->oobsize;
		oob_ops.oobretlen = 0;
		oob_ops.ooboffs = 0;
		oob_ops.datbuf = bp;
		oob_ops.oobbuf = spare;
		i = mtd_read_oob(mtd, off, &oob_ops);
		sz = oob_ops.retlen;

		n = 0;
		j = 0;
		if (i != 0) {
			printf("%d/%d return %d sz %d\n", block, page, i, sz);
		}
		if (page == 0) {
			//printf("%d/%d  ", block, page, i, sz);
			printf(".");
			// printf("%d/%d r %d sz %d", block, page, i, sz);
		}
		while ((n < 2) && (j < sz)) {
			up32 = bp + j;
			n = n + 32 - generic_hweight32(*up32);
			j = j + 4;
			/* check if not blank ... otherwise read oob too */
		}
		if (page == 0) {
			// printf(" zeros %d", n);
		}
		z = 0;
		if (n < 2) {
			/* check for zeros in oob */
			for (j = 0; j < oob_ops.oobretlen >> 2; j++) {
				z = z + 32 - generic_hweight32(spare[j]);
			}
			if (page == 0) {
				// printf(" spare zeros %d\n", z);
			}
		} else {
			if (page == 0) {
				// printf("\n");
			}
		}
		/* deal with this page */
		// if (n + z > 2) {
		if ((i == 0) && (n != 0)) {
			// not blank
			rei.recovery_chunks_list[chunk].flashpage =
			    block * pages + page;
			rei.recovery_chunks_list[chunk].size = mtd->writesize;
			rei.recovery_chunks_list[chunk].type = 0x00;
			chunk++;
			rei.recovery_data_len += mtd->writesize;
		}
		page = (page + 1) % pages;
		if (page == 0) {
			block++;
		}
	}
	rei.recovery_chunks_list[chunk].type = 0x7fffffff;
	printf("chunks %d len 0x%x\n", chunk, rei.recovery_data_len);
	// load old blocks (everything but reimage image) to ddr
	put_mtd_device(mtd);
	return (ret);
}

	// erase everything except the reimage payload (after loader)
	// burn loader
	// attach image
	// store preserved files
	// store old blocks

static int do_flashback(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[]);
static int do_flashback(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	struct mtd_info *mtd = NULL;
	long offset;
	int i;
	size_t sz = 0;
	int blocks, pages;
	int ret = 0;
	char *bp;
	int chunk;
	if (rei.recovery_data_len == 0) {
		printf("read recovery chunks first\n");
		return (2);
	}
	mtd = get_mtd_device_nm("nand0");
	blocks = (mtd->size / mtd->erasesize);
	pages = (mtd->erasesize / mtd->writesize);
	erase(mtd, 0, blocks);
	chunk = 0;
	bp = rei.recovery_data_buf;
	while (rei.recovery_chunks_list[chunk].type < 0x1000) {
		i = mtd_write(mtd,
			      rei.recovery_chunks_list[chunk].flashpage *
			      mtd->writesize, mtd->writesize, &sz, bp);
		bp += rei.recovery_chunks_list[chunk].size;
		chunk++;
		printf("%d ", chunk);
	}
	put_mtd_device(mtd);
}

static int do_store_preserved(cmd_tbl_t * cmdtp, int flag, int argc,
			      char *const argv[]);
static int do_store_preserved(cmd_tbl_t * cmdtp, int flag, int argc,
			      char *const argv[])
{
	char cmd[256];
	struct mtd_info *mtd = NULL;
	mtd = get_mtd_device_nm("nand0");
	if (IS_ERR_OR_NULL(mtd)) {
		printf("failed to get mtd\n");
		return (-1);
	}
	sprintf(cmd, "%s:%lld(loader),%lld@%lld(image)",
		"brcmnand.0",
		(long long)(rei.loader_blocks * (long long)mtd->erasesize),
		(long long)(mtd->size -
			    (rei.loader_blocks + 8) * mtd->erasesize),
		(long long)(rei.loader_blocks * mtd->erasesize));
	run_command("mtdparts delall", 0);
	env_set("mtdparts", cmd);
	run_command("mtdparts", 0);
	run_command("ubi part image", 0);
	if (rei.preserved_data_max_len) {
		sprintf(cmd, "ubi create transition 0x%x dynamic 33",
			rei.preserved_data_max_len);
		run_command(cmd, 0);
		sprintf(cmd, "ubi write 0x%x transition 0x%x",
			rei.preserved_data, rei.preserved_data_max_len);
		run_command(cmd, 0);
	}
	if (rei.recovery_data_len) {
		sprintf(cmd, "ubi create recovery 0x%x dynamic 34",
			rei.recovery_data_len + (void *)rei.recovery_data_buf -
			(void *)rei.recovery_chunks_list);
		run_command(cmd, 0);
		sprintf(cmd, "ubi write 0x%x recovery 0x%x",
			rei.recovery_chunks_list,
			rei.recovery_data_len + (void *)rei.recovery_data_buf -
			(void *)rei.recovery_chunks_list);
		run_command(cmd, 0);
	}
}

void reimage_splice_env(struct reimager *r, char *more_env, int more_env_size)
{
	char *bp;
	int i;
	for (bp = r->loader_payload;
	     bp <
	     r->loader_payload + r->blocksizeK * 1024 * r->loader_blocks;
	     bp += 4096) {
		int *tenv;
		env_t *ep;
		uint32_t new, crc;
		tenv = (int *)bp;
		printf("env check at %x\r", bp - r->loader_payload);
		if (tenv[0] == BOOT_MAGIC_MAGIC) {
			printf("\nGOT IT\n");
			ep = (env_t *) & tenv[2];
			memcpy(&crc, &ep->crc, sizeof(crc));
			/* specifically use uboot's env crc function
			 * even though we have included the standard
			 * linux crc32 */
			new = the_env_crc32(0, ep->data, tenv[1] - 4);
			if (new != crc) {
				printf("bad\n");
			} else {
				printf("good\n");

				for (i = 0; i < tenv[1] - 4; i++) {
					if ((ep->data[i] == '\0')
					    && (ep->data[i + 1] == '\0')) {
						memcpy(&ep->data[i + 1],
						       more_env, more_env_size);
						break;
					}
				}
				new = the_env_crc32(0, ep->data, tenv[1] - 4);
				memcpy(&ep->crc, &new, sizeof(new));
			}
		}
	}
}

static char usage[] = "line 1...\n" "line 2...\n";

U_BOOT_CMD_WITH_SUBCMDS(safeimage, "safe reimage commands", usage,
			U_BOOT_SUBCMD_MKENT(commit, 5, 0, do_commit),
			U_BOOT_SUBCMD_MKENT(flashback, 5, 0, do_flashback),
			U_BOOT_SUBCMD_MKENT(read_recovery, 5, 0,
					    do_read_recovery),
			U_BOOT_SUBCMD_MKENT(revertable, 5, 0, do_revertable),
			U_BOOT_SUBCMD_MKENT(nvram, 1, 0, do_nvram_parse),
			U_BOOT_SUBCMD_MKENT(store_preserved, 1, 0,
					    do_store_preserved));

U_BOOT_CMD_WITH_SUBCMDS(reimage, "reimage commands", usage,
			U_BOOT_SUBCMD_MKENT(auto, 1, 0, do_reimage_auto),
			U_BOOT_SUBCMD_MKENT(finish, 5, 0, do_finish),
			U_BOOT_SUBCMD_MKENT(prepare, 5, 0, do_prepare),
			U_BOOT_SUBCMD_MKENT(nvram, 1, 0, do_nvram_parse));

U_BOOT_CMD_WITH_SUBCMDS(preserve, "preserve data commands", usage,
			U_BOOT_SUBCMD_MKENT(allocate, 5, 0,
					    do_preserve_allocate),
			U_BOOT_SUBCMD_MKENT(save, 5, 0, do_preserve_save));
