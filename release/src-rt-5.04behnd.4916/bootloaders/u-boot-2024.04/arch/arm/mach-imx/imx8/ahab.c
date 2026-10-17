// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2019, 2022 NXP
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <imx_container.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <firmware/imx/sci/sci.h>
#include <asm/mach-imx/sys_proto.h>
#include <asm/arch-imx/cpu.h>
#include <asm/arch/sys_proto.h>
#include <console.h>
#include <cpu_func.h>
#include "u-boot/sha256.h"
#include <asm/mach-imx/ahab.h>

DECLARE_GLOBAL_DATA_PTR;

#define SEC_SECURE_RAM_BASE             (0x31800000UL)
#define SEC_SECURE_RAM_END_BASE         (SEC_SECURE_RAM_BASE + 0xFFFFUL)
#define SECO_LOCAL_SEC_SEC_SECURE_RAM_BASE  (0x60000000UL)

#define SECO_PT                 2U
#define AHAB_HASH_TYPE_MASK	0x00000700
#define AHAB_HASH_TYPE_SHA256	0

int ahab_auth_cntr_hdr(struct container_hdr *container, u16 length)
{
	int err;

	memcpy((void *)SEC_SECURE_RAM_BASE, (const void *)container,
	       ALIGN(length, CONFIG_SYS_CACHELINE_SIZE));

	err = sc_seco_authenticate(-1, SC_SECO_AUTH_CONTAINER,
				   SECO_LOCAL_SEC_SEC_SECURE_RAM_BASE);
	if (err)
		printf("Authenticate container hdr failed, return %d\n", err);

	return err;
}

int ahab_auth_release(void)
{
	int err;

	err = sc_seco_authenticate(-1, SC_SECO_REL_CONTAINER, 0);
	if (err)
		printf("Error: release container failed!\n");

	return err;
}

int ahab_verify_cntr_image(struct boot_img_t *img, int image_index)
{
	sc_faddr_t start, end;
	sc_rm_mr_t mr;
	int err;
	int ret = 0;

	debug("img %d, dst 0x%llx, src 0x%x, size 0x%x\n",
	      image_index, img->dst, img->offset, img->size);

	/* Find the memreg and set permission for seco pt */
	err = sc_rm_find_memreg(-1, &mr,
				img->dst & ~(CONFIG_SYS_CACHELINE_SIZE - 1),
				ALIGN(img->dst + img->size, CONFIG_SYS_CACHELINE_SIZE) - 1);

	if (err) {
		printf("Error: can't find memreg for image load address 0x%llx, error %d\n",
		       img->dst, err);
		return -ENOMEM;
	}

	err = sc_rm_get_memreg_info(-1, mr, &start, &end);
	if (!err)
		debug("memreg %u 0x%llx -- 0x%llx\n", mr, start, end);

	err = sc_rm_set_memreg_permissions(-1, mr,
					   SECO_PT, SC_RM_PERM_FULL);
	if (err) {
		printf("Set permission failed for img %d, error %d\n",
		       image_index, err);
		return -EPERM;
	}

	err = sc_seco_authenticate(-1, SC_SECO_VERIFY_IMAGE,
				   1 << image_index);
	if (err) {
		printf("Authenticate img %d failed, return %d\n",
		       image_index, err);
		ret = -EIO;
	}

	err = sc_rm_set_memreg_permissions(-1, mr,
					   SECO_PT, SC_RM_PERM_NONE);
	if (err) {
		printf("Remove permission failed for img %d, error %d\n",
		       image_index, err);
		ret = -EPERM;
	}

	return ret;
}

static inline bool check_in_dram(ulong addr)
{
	int i;
	struct bd_info *bd = gd->bd;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; ++i) {
		if (bd->bi_dram[i].size) {
			if (addr >= bd->bi_dram[i].start &&
			    addr < (bd->bi_dram[i].start + bd->bi_dram[i].size))
				return true;
		}
	}

	return false;
}

int authenticate_os_container(ulong addr)
{
	struct container_hdr *phdr;
	int i, ret = 0;
	int err;
	u16 length;
	struct boot_img_t *img;
	unsigned long s, e;
#ifdef CONFIG_ARMV8_CE_SHA256
	u8 hash_value[SHA256_SUM_LEN];
#endif

	if (addr % 4) {
		puts("Error: Image's address is not 4 byte aligned\n");
		return -EINVAL;
	}

	if (!check_in_dram(addr)) {
		puts("Error: Image's address is invalid\n");
		return -EINVAL;
	}

	phdr = (struct container_hdr *)addr;
	if (!valid_container_hdr(phdr)) {
		printf("Error: Wrong container header\n");
		return -EFAULT;
	}

	if (!phdr->num_images) {
		printf("Error: Wrong container, no image found\n");
		return -EFAULT;
	}

	length = phdr->length_lsb + (phdr->length_msb << 8);

	debug("container length %u\n", length);

	err = ahab_auth_cntr_hdr(phdr, length);
	if (err) {
		ret = -EIO;
		goto exit;
	}

	/* Copy images to dest address */
	for (i = 0; i < phdr->num_images; i++) {
		img = (struct boot_img_t *)(addr +
					    sizeof(struct container_hdr) +
					    i * sizeof(struct boot_img_t));

		debug("img %d, dst 0x%x, src 0x%lux, size 0x%x\n",
		      i, (uint32_t) img->dst, img->offset + addr, img->size);

		memcpy((void *)img->dst, (const void *)(img->offset + addr),
		       img->size);

		s = img->dst & ~(CONFIG_SYS_CACHELINE_SIZE - 1);
		e = ALIGN(img->dst + img->size, CONFIG_SYS_CACHELINE_SIZE) - 1;

		flush_dcache_range(s, e);

#ifdef CONFIG_ARMV8_CE_SHA256
		if (((img->hab_flags & AHAB_HASH_TYPE_MASK) >> 8) == AHAB_HASH_TYPE_SHA256) {
			sha256_csum_wd((void *)img->dst, img->size, hash_value, CHUNKSZ_SHA256);
			err = memcmp(&img->hash, &hash_value, SHA256_SUM_LEN);
			if (err) {
				printf("img %d hash comparison failed, error %d\n", i, err);
				ret = -EIO;
				goto exit;
			}
		} else {
#endif
			ret = ahab_verify_cntr_image(img, i);
			if (ret)
				goto exit;
#ifdef CONFIG_ARMV8_CE_SHA256
		}
#endif
	}

exit:
	ahab_auth_release();

	return ret;
}

static int do_authenticate(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	ulong addr;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = hextoul(argv[1], NULL);

	printf("Authenticate OS container at 0x%lx\n", addr);

	if (authenticate_os_container(addr))
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static void display_life_cycle(u16 lc)
{
	printf("Lifecycle: 0x%04X, ", lc);
	switch (lc) {
	case 0x1:
		printf("Pristine\n\n");
		break;
	case 0x2:
		printf("Fab\n\n");
		break;
	case 0x8:
		printf("Open\n\n");
		break;
	case 0x20:
		printf("NXP closed\n\n");
		break;
	case 0x80:
		printf("OEM closed\n\n");
		break;
	case 0x100:
		printf("Partial field return\n\n");
		break;
	case 0x200:
		printf("Full field return\n\n");
		break;
	case 0x400:
		printf("No return\n\n");
		break;
	default:
		printf("Unknown\n\n");
		break;
	}
}

#define AHAB_AUTH_CONTAINER_REQ 0x87
#define AHAB_VERIFY_IMAGE_REQ 0x88

#define AHAB_NO_AUTHENTICATION_IND 0xee
#define AHAB_BAD_KEY_HASH_IND 0xfa
#define AHAB_INVALID_KEY_IND 0xf9
#define AHAB_BAD_SIGNATURE_IND 0xf0
#define AHAB_BAD_HASH_IND 0xf1

static void display_ahab_auth_event(u32 event)
{
	u8 cmd = (event >> 16) & 0xff;
	u8 resp_ind = (event >> 8) & 0xff;

	switch (cmd) {
	case AHAB_AUTH_CONTAINER_REQ:
		printf("\tCMD = AHAB_AUTH_CONTAINER_REQ (0x%02X)\n", cmd);
		printf("\tIND = ");
		break;
	case AHAB_VERIFY_IMAGE_REQ:
		printf("\tCMD = AHAB_VERIFY_IMAGE_REQ (0x%02X)\n", cmd);
		printf("\tIND = ");
		break;
	default:
		return;
	}

	switch (resp_ind) {
	case AHAB_NO_AUTHENTICATION_IND:
		printf("AHAB_NO_AUTHENTICATION_IND (0x%02X)\n\n", resp_ind);
		break;
	case AHAB_BAD_KEY_HASH_IND:
		printf("AHAB_BAD_KEY_HASH_IND (0x%02X)\n\n", resp_ind);
		break;
	case AHAB_INVALID_KEY_IND:
		printf("AHAB_INVALID_KEY_IND (0x%02X)\n\n", resp_ind);
		break;
	case AHAB_BAD_SIGNATURE_IND:
		printf("AHAB_BAD_SIGNATURE_IND (0x%02X)\n\n", resp_ind);
		break;
	case AHAB_BAD_HASH_IND:
		printf("AHAB_BAD_HASH_IND (0x%02X)\n\n", resp_ind);
		break;
	default:
		printf("Unknown Indicator (0x%02X)\n\n", resp_ind);
		break;
	}
}

static int do_ahab_status(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	int err;
	u8 idx = 0U;
	u32 event;
	u16 lc;

	err = sc_seco_chip_info(-1, &lc, NULL, NULL, NULL);
	if (err) {
		printf("Error in get lifecycle\n");
		return -EIO;
	}

	display_life_cycle(lc);

	err = sc_seco_get_event(-1, idx, &event);
	while (!err) {
		printf("SECO Event[%u] = 0x%08X\n", idx, event);
		display_ahab_auth_event(event);

		idx++;
		err = sc_seco_get_event(-1, idx, &event);
	}

	if (idx == 0)
		printf("No SECO Events Found!\n\n");

	return 0;
}

int ahab_close(void)
{
	int err;
	u16 lc;

	err = sc_seco_chip_info(-1, &lc, NULL, NULL, NULL);
	if (err != SC_ERR_NONE) {
		printf("Error in get lifecycle\n");
		return -EIO;
	}

	if (lc != 0x20) {
		puts("Current lifecycle is NOT NXP closed, can't move to OEM closed\n");
		display_life_cycle(lc);
		return -EPERM;
	}

	err = sc_seco_forward_lifecycle(-1, 16);
	if (err != SC_ERR_NONE) {
		printf("Error in forward lifecycle to OEM closed\n");
		return -EIO;
	}

	return 0;
}

static int confirm_close(void)
{
	puts("Warning: Please ensure your sample is in NXP closed state, "
	     "OEM SRK hash has been fused, \n"
	     "         and you are able to boot a signed image successfully "
	     "without any SECO events reported.\n"
	     "         If not, your sample will be unrecoverable.\n"
	     "\nReally perform this operation? <y/N>\n");

	if (confirm_yesno())
		return 1;

	puts("Ahab close aborted\n");
	return 0;
}

static int do_ahab_close(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int confirmed = argc >= 2 && !strcmp(argv[1], "-y");
	int err;

	if (!confirmed && !confirm_close())
		return -EACCES;

	err = ahab_close();
	if (err) {
		printf("Change to OEM closed failed\n");
		return err;
	}

	printf("Change to OEM closed successfully\n");

	return 0;
}

U_BOOT_CMD(auth_cntr, CONFIG_SYS_MAXARGS, 1, do_authenticate,
	   "autenticate OS container via AHAB",
	   "addr\n"
	   "addr - OS container hex address\n"
);

U_BOOT_CMD(ahab_status, CONFIG_SYS_MAXARGS, 1, do_ahab_status,
	   "display AHAB lifecycle and events from seco",
	   ""
);

U_BOOT_CMD(ahab_close, CONFIG_SYS_MAXARGS, 1, do_ahab_close,
	   "Change AHAB lifecycle to OEM closed",
	   ""
);
