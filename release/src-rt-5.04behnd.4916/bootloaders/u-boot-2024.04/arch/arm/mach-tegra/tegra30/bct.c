// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Ramin <raminterex@yahoo.com>
 * Copyright (c) 2022, Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <log.h>
#include <asm/arch-tegra/crypto.h>
#include "bct.h"
#include "uboot_aes.h"

/* Device with "sbk burned: false" will expose zero key */
const u8 nosbk[AES128_KEY_LENGTH] = { 0 };

/*
 * @param  bct		boot config table start in RAM
 * @param  ect		bootloader start in RAM
 * @param  ebt_size	bootloader file size in bytes
 * Return: 0, or 1 if failed
 */
static int bct_patch(u8 *bct, u8 *ebt, u32 ebt_size)
{
	struct nvboot_config_table *bct_tbl = NULL;
	u8 ebt_hash[AES128_KEY_LENGTH] = { 0 };
	u8 sbk[AES128_KEY_LENGTH] = { 0 };
	u8 *bct_hash = bct;
	bool encrypted;
	int ret;

	bct += BCT_HASH;

	ebt_size = roundup(ebt_size, EBT_ALIGNMENT);

	memcpy(sbk, (u8 *)(bct + BCT_LENGTH),
	       NVBOOT_CMAC_AES_HASH_LENGTH * 4);

	encrypted = memcmp(&sbk, &nosbk, AES128_KEY_LENGTH);

	if (encrypted) {
		ret = decrypt_data_block(bct, BCT_LENGTH, sbk);
		if (ret)
			return 1;

		ret = encrypt_data_block(ebt, ebt_size, sbk);
		if (ret)
			return 1;
	}

	ret = sign_enc_data_block(ebt, ebt_size, ebt_hash, sbk);
	if (ret)
		return 1;

	bct_tbl = (struct nvboot_config_table *)bct;

	memcpy((u8 *)&bct_tbl->bootloader[0].crypto_hash,
	       ebt_hash, NVBOOT_CMAC_AES_HASH_LENGTH * 4);
	bct_tbl->bootloader[0].entry_point = CONFIG_SPL_TEXT_BASE;
	bct_tbl->bootloader[0].load_addr = CONFIG_SPL_TEXT_BASE;
	bct_tbl->bootloader[0].length = ebt_size;

	if (encrypted) {
		ret = encrypt_data_block(bct, BCT_LENGTH, sbk);
		if (ret)
			return 1;
	}

	ret = sign_enc_data_block(bct, BCT_LENGTH, bct_hash, sbk);
	if (ret)
		return 1;

	return 0;
}

static int do_ebtupdate(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	u32 bct_addr = hextoul(argv[1], NULL);
	u32 ebt_addr = hextoul(argv[2], NULL);
	u32 ebt_size = hextoul(argv[3], NULL);

	return bct_patch((u8 *)bct_addr, (u8 *)ebt_addr, ebt_size);
}

U_BOOT_CMD(ebtupdate,	4,	0,	do_ebtupdate,
	   "update bootloader on re-crypted Tegra30 devices",
	   ""
);
