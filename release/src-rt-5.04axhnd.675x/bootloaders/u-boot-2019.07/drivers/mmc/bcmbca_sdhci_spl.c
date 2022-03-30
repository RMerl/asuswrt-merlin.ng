// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019  Broadcom Ltd
 *
 * Author: Farhan Ali <farhan.ali@broadcom.com>
 */

#include <common.h>
#include <mmc.h>
#include <malloc.h>

char temp_buff[512];

/* Copy over data from a specified offset */
void mmc_spl_load_image(uint32_t offs, unsigned int size, void *vdst)
{
	uint blk_start, blk_cnt, err;
	char * datap = vdst;
	int i=0;
	uint init_offset = 0;
	uint orig_size = size;

	struct mmc *mmc = find_mmc_device(0);
	struct blk_desc * block_dev = mmc_get_blk_desc(mmc);
	if (!mmc) {
		puts("spl: mmc device not found!!\n");
		hang();
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return;
	}
	
	if( mmc->read_bl_len > sizeof(temp_buff) )
	{
		puts("MMC temp buffer is smaller than read_bl_len\n");
		hang();
	}

	blk_start = offs/mmc->read_bl_len;
	init_offset = offs % mmc->read_bl_len;
	blk_cnt = (init_offset+size)/mmc->read_bl_len + ((init_offset+size)%mmc->read_bl_len?1:0);

	debug("Addr:0x%08x Len:0x%08x Startb:0x%08x Numb:0x%08x Endb:0x%08x\n"
		, offs, size, blk_start, blk_cnt			
		,((offs+size)/mmc->read_bl_len + ((offs+size)%mmc->read_bl_len?1:0)));

	for( i=0; i<blk_cnt; i++ )
	{
		err = blk_dread(block_dev, blk_start, 1,
				temp_buff);
		if (err != 1) 
		{
			puts("spl: mmc read failed!!\n");
			hang();
		} 
		else
		{
			/* All data in current block, copy it over */
			if( (init_offset + size) < mmc->read_bl_len )
			{
				memcpy(datap, temp_buff+init_offset, size);
				size = 0;
				init_offset = 0;
			}
			else
			{
				/* Data spans blocks, copy all data in this block */
				memcpy(datap, temp_buff+init_offset, mmc->read_bl_len-init_offset);
				size -= mmc->read_bl_len-init_offset;
				datap += mmc->read_bl_len-init_offset;
				init_offset = 0;
			}

			blk_start++;
		}
	}
}

