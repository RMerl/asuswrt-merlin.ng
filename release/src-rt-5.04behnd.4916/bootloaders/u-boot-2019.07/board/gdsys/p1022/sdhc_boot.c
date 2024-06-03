/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <mmc.h>
#include <malloc.h>

/*
 * The environment variables are written to just after the u-boot image
 * on SDCard, so we must read the MBR to get the start address and code
 * length of the u-boot image, then calculate the address of the env.
 */
#define ESDHC_BOOT_IMAGE_SIZE	0x48
#define ESDHC_BOOT_IMAGE_ADDR	0x50

int mmc_get_env_addr(struct mmc *mmc, int copy, u32 *env_addr)
{
	u8 *tmp_buf;
	u32 blklen, code_offset, code_len, n;

	blklen = mmc->read_bl_len;
	tmp_buf = malloc(blklen);
	if (!tmp_buf)
		return 1;

	/* read out the first block, get the config data information */
	n = mmc->block_dev.block_read(&mmc->block_dev, 0, 1, tmp_buf);
	if (!n) {
		free(tmp_buf);
		return 1;
	}

	/* Get the Source Address, from offset 0x50 */
	code_offset = *(u32 *)(tmp_buf + ESDHC_BOOT_IMAGE_ADDR);

	/* Get the code size from offset 0x48 */
	code_len = *(u32 *)(tmp_buf + ESDHC_BOOT_IMAGE_SIZE);

	*env_addr = code_offset + code_len;

	free(tmp_buf);

	return 0;
}
