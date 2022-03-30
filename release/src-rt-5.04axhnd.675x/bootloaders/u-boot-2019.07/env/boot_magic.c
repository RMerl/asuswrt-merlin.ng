// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2019 Broadcom Corporation
 * Joel Peshkin, Broadcom Corporation, joel.peshkin@broadcom.com
 */

#define DEBUG

#include <common.h>
#include <command.h>
#include <environment.h>
#include <hexdump.h>
#include <linux/stddef.h>
#include <nand.h>
#include <mmc.h>
#include <nand.h>
#include <search.h>
#include <errno.h>
#include <bca_sdk.h>

DECLARE_GLOBAL_DATA_PTR;

__weak uint32_t env_boot_magic_search_size(void)
{
	return 2*1024*1024;
}

__weak int env_override_import(void *ep)
{
	return 0;
}

#ifdef CONFIG_SPI_FLASH
static int env_dev_read_spinor( int addr, size_t * len, char * buffer )
{
	int ret= -1;
	struct mtd_info *mtd;
	size_t retlen;	

	mtd = get_mtd_device_nm(LOADER_PART);
	if (IS_ERR_OR_NULL(mtd)){
		debug("%s:MTD device %s not found, ret %ld\n",__func__, LOADER_PART,
		   PTR_ERR(mtd));
		return ret;
	}
	ret = mtd_read(mtd,addr,*len,&retlen,buffer);
	put_mtd_device(mtd);
	return ret;
}
static int env_dev_write_spinor( int addr, size_t * len, char * buffer )
{
	int ret= -1;
	struct mtd_info *mtd;
	struct erase_info ei;
	size_t retlen;	
	u_char * tempbuff = NULL;
	size_t erase_bytes;
	u32 start_offset;
	u32 start_sector;
	u32 end_sector;
	
	mtd = get_mtd_device_nm(LOADER_PART);
	if (IS_ERR_OR_NULL(mtd)){
		debug("%s:MTD device %s not found, ret %ld\n",__func__, LOADER_PART,
	   		PTR_ERR(mtd));
		return ret;
	}

	start_sector = addr/mtd->erasesize;
	end_sector = ( addr + *len + mtd->erasesize - 1)/mtd->erasesize;
	start_offset = start_sector * mtd->erasesize;
	erase_bytes = (end_sector - start_sector)*mtd->erasesize;
	tempbuff = malloc(erase_bytes);
	if (!tempbuff){
		debug("%s malloc %d failed\n",__func__, erase_bytes);
		goto done;
 	}
	
	ret = mtd_read(mtd,start_offset,erase_bytes,&retlen,tempbuff);
	if (ret){
		debug("%s:read MTD device %s fail, ret %d\n",__func__, LOADER_PART,
	   		ret);
		goto done;
	}

	memcpy(tempbuff + addr%mtd->erasesize, buffer, *len);
	memset(&ei, 0, sizeof(ei));
	ei.mtd = mtd;
	ei.addr = start_offset;
	ei.len = erase_bytes;
	ret = mtd_erase(mtd, &ei);
	if (ret){
		debug("%s:erase MTD device %s fail, ret %d\n",__func__, LOADER_PART,
	   		ret);
		goto done;
	}

	ret = mtd_write(mtd, start_offset, erase_bytes, &retlen, tempbuff);
	if (ret){
		debug("%s:write MTD device %s fail, ret %d\n",__func__, LOADER_PART,
	   		ret);
	}

done:	
	free(tempbuff);
	put_mtd_device(mtd);
	return ret;
	
}
#endif
#ifdef CONFIG_NAND	
static int env_dev_read_nand( int addr, size_t * len, char * buffer )
{
	int ret= -1;
	struct mtd_info *mtd;
	mtd = get_nand_dev_by_index(0);
	if (!mtd)
		return ret;
	ret = nand_read(mtd, addr, len, (void *)buffer);
	return ret;
}
static int env_dev_write_nand( int addr, size_t * len, char * buffer )
{
	int blocknum, off;
	size_t blocksize;
	int ret = -1;

	struct mtd_info *mtd;
	char * block = NULL;
	mtd = get_nand_dev_by_index(0);
	if (!mtd)
		return ret;
	
	block = malloc(mtd->erasesize);
	if (!block)
		return ret;

	blocksize = mtd->erasesize;
	blocknum = addr / mtd->erasesize;
	off = addr % mtd->erasesize;
	ret = nand_read(mtd, (off_t) (blocknum * mtd->erasesize),
		      &blocksize, (void *)block);
	if (ret < 0) {
		debug("read error %d on block %d\n", ret, blocknum);
	}
	memcpy(block + off, buffer, *len);
	nand_erase(mtd, blocknum * blocksize, blocksize);
	ret = nand_write(mtd, blocknum * blocksize, &blocksize,
			       (void *)block);
	if (ret < 0) {
		debug("write error %d on block %d\n", ret, blocknum);
	}
	free(block);
	return ret;
}
#endif

#ifdef CONFIG_MMC

#define ENV_DEV_MMC_READ 		0
#define ENV_DEV_MMC_WRITE		1
#define ENV_DEV_MMC_DEV_NUM 		0
#define ENV_DEV_MMC_USERDATA_PART 	0

static int env_dev_read_write_mmc( int addr, size_t * len, char * buffer, int mode)
{
	uint64_t blocksize, start_blocknum, start_offset, num_blocks;
	int ret = -1;
	char * block_data_ptr = NULL;
	struct mmc *mmc = NULL;
	struct blk_desc * block_dev = NULL;
	int  mmc_boot_part = 0;

	/* Initialize MMC */
	mmc = find_mmc_device(ENV_DEV_MMC_DEV_NUM);
	if (!mmc)
		return ret;
	mmc_init(mmc);

	/* Get active boot partition */
	mmc_boot_part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);

	/* Default to first boot partition if nothing is set */
	if( !mmc_boot_part )
		mmc_boot_part = 1;

	/* Point to BOOT partition */
	blk_select_hwpart_devnum(IF_TYPE_MMC, ENV_DEV_MMC_DEV_NUM, mmc_boot_part);

	/* Get block device */
	block_dev = mmc_get_blk_desc(mmc);
	if (!block_dev)
		goto mmc_err;
	
	/* Calculate block numbers and offsets */
	blocksize = mmc->read_bl_len;
	start_blocknum = addr/blocksize; 
	start_offset = addr %  blocksize;
	num_blocks = (start_offset+*len)/blocksize + ((start_offset+*len)%blocksize?1:0);
	block_data_ptr = memalign(ARCH_DMA_MINALIGN, num_blocks*blocksize);
	if( !block_data_ptr ) {
		printf("%s: ERROR! Cannot allocate memory for blockdata\n", __FUNCTION__);
		goto mmc_err;
	}

	ret = blk_dread(block_dev, start_blocknum, num_blocks, (void *)block_data_ptr);
	if (ret < 0) {
		printf("%s: read error %d on block %llu, num_blocks %llu\n", __FUNCTION__, ret, start_blocknum, num_blocks);
		goto mmc_err;
	}
		
	if( mode == ENV_DEV_MMC_WRITE ) {
		memcpy(block_data_ptr + start_offset, buffer, *len);
		ret = blk_dwrite(block_dev, start_blocknum, num_blocks, (void *)block_data_ptr);

		if (ret < 0) {
			printf("%s: write error %d on block %llu, num_blocks %llu\n", __FUNCTION__, ret, start_blocknum, num_blocks);
			goto mmc_err;
		}
	} else	{
		memcpy(buffer, block_data_ptr + start_offset, *len);
	}	

	ret = 0;
mmc_err:
	if( block_data_ptr )
		free(block_data_ptr);

	/* Point to USERDATA partition */
	blk_select_hwpart_devnum(IF_TYPE_MMC, ENV_DEV_MMC_DEV_NUM, ENV_DEV_MMC_USERDATA_PART);
	return ret;
}
#endif	

static int get_env_flashtype( char ** flashtype )
{
	*flashtype = get_loader_media();
	return 0;
}

static int env_dev_read( int addr, size_t * len, char * buffer )
{
	char * pflashtype_str = NULL;
	if( get_env_flashtype(&pflashtype_str) == 0 ) {
		if( pflashtype_str != NULL ) {
#ifdef CONFIG_NAND	
			if( strcasecmp(pflashtype_str,FLASH_DEV_STR_NAND) == 0 )  {
				return (env_dev_read_nand(addr, len, buffer));
			}
#endif
		
#ifdef CONFIG_MMC		
			if( strcasecmp(pflashtype_str,FLASH_DEV_STR_EMMC) == 0 ) {
				return (env_dev_read_write_mmc(addr, len, buffer, ENV_DEV_MMC_READ));
			}
#endif		
		
#ifdef CONFIG_SPI_FLASH		
			if( strcasecmp(pflashtype_str,FLASH_DEV_STR_SPINOR) == 0 ) {
				return (env_dev_read_spinor(addr, len, buffer));
			}
#endif		

			printf("%s: Error: Unsupported env storage media %s!\n", __FUNCTION__, pflashtype_str);
		} else {
			printf("%s: Error: Environment storage media invalid!\n", __FUNCTION__);
		}
	} else {
		printf("%s: Error: Cannot find environment storage media!\n", __FUNCTION__);
	}
	return -1;
}

static int env_dev_write( int addr, size_t * len, char * buffer )
{
	char * pflashtype_str = NULL;
	if( get_env_flashtype(&pflashtype_str) == 0 )
	{
		if( pflashtype_str != NULL ) {
#ifdef CONFIG_NAND	
			if( strcasecmp(pflashtype_str,FLASH_DEV_STR_NAND) == 0 ) {
				return (env_dev_write_nand(addr, len, buffer));
			}
#endif
			
#ifdef CONFIG_MMC		
			if( strcasecmp(pflashtype_str,FLASH_DEV_STR_EMMC) == 0 ) {
				return (env_dev_read_write_mmc(addr, len, buffer, ENV_DEV_MMC_WRITE));
			}
#endif		

#ifdef CONFIG_SPI_FLASH		
			if( strcasecmp(pflashtype_str,FLASH_DEV_STR_SPINOR) == 0 ) {
				return (env_dev_write_spinor(addr, len, buffer));
			}
#endif		
		}
	}
	printf("%s: Error: Cannot find environment storage media!\n", __FUNCTION__);
	return -1;
}

static int env_boot_magic_load(void)
{

	uint32_t crc, new;
	int ret;
	loff_t off, copies[10];
	int copies_found = 0;
	int found_size;
	size_t rdlen;
	env_t *ep;
	uint32_t magichdr[3];
	char *ebuff = NULL;

	printf("ENV_BOOT_MAGIC_LOAD\n");

	for (off = 0; off < (loff_t)env_boot_magic_search_size(); off += 4096) {
		rdlen = 12;
		env_dev_read( off, &rdlen, (void *)magichdr);
		if (magichdr[0] != BOOT_MAGIC_MAGIC) {
			continue;
		}
		printf("found magic at %lx\n", (long)off);
		rdlen = magichdr[1] + 12;
		if (ebuff == NULL) {
			ebuff = malloc(max((int)rdlen, CONFIG_ENV_SIZE + 12));
		}
		env_dev_read( off, &rdlen, (void *)ebuff);
		ep = (env_t *) & ebuff[8];
		memcpy(&crc, &ep->crc, sizeof(crc));

		new = crc32(0, ep->data, rdlen - 16);
		if (new != crc) {
			debug("CRC mismatch len = %d\n", (int)rdlen - 16);
			debug("computed %x \n", new);
			debug("expected %x \n", crc);
		} else {
			debug("good crc\n");
			gd->env_valid = ENV_VALID;
			/* FIXME -- may need to grow instead of shrink */
			debug("resize from %ld to %d\n", rdlen - 12,
			      CONFIG_ENV_SIZE);
			new = crc32(0, ep->data, CONFIG_ENV_SIZE - 4);
			memcpy(&ep->crc, &new, sizeof(new));
			copies[copies_found++] = off;
			found_size = rdlen - 12;
			if (!env_override_import((void *)ep))
			{
				ret = env_import((void *)ep, 1);
			}
		}
		free(ebuff);
		ebuff = NULL;
		if (NULL != env_get("env_boot_magic")) {
			break;
		}
	}

	if (copies_found > 0) {
		if (NULL == env_get("env_boot_magic")) {
			char found[256];
			int i;
			sprintf(found, "%d@", found_size);
			for (i = 0; i < copies_found; i++) {
				sprintf(found + strlen(found), "0x%llx,",
					copies[i]);
			}
			found[strlen(found) - 1] = '\0';
			env_set("env_boot_magic", found);
			env_set("env_boot_magic_updated", "1");
		}
		return (ret);
	} else {
		set_default_env("import not done", 0);
		return (-1);
	}

}

static int env_boot_magic_save(void)
{
	env_t *ep;
	char *envbuf = NULL;
	uint32_t *envintp;
	char *c;
	int ret = -2;
	int elen;
	size_t wr_len;
	char *config;
	uint32_t new;
	int i;
	char *found = NULL;

	config = env_get("env_boot_magic");
	if (NULL == config)
		return (-1);
	elen = simple_strtoul(config, NULL, 0);
	found = malloc(strlen(config) + 1);
	if (!found)
		goto err;
	strcpy(found, config);
	envbuf = malloc(max(elen + 12, CONFIG_ENV_SIZE + 12));
	ep = (env_t *) (envbuf + 8);
	envintp = (uint32_t *) envbuf;
	ret = env_export(ep);
	for (i = CONFIG_ENV_SIZE; i < elen; i++) {
		envbuf[12 + i] = 0xff;
	}
	new = crc32(0, ep->data, elen - 4);
	memcpy(&ep->crc, &new, sizeof(new));
	envintp[0] = BOOT_MAGIC_MAGIC;
	envintp[1] = elen;
	strtok(found, "@");
	while ((c = strtok(NULL, ","))) {
		i = simple_strtoul(c, NULL, 0);
		printf("save to %x\n", i);
		wr_len = elen+12;
                ret = env_dev_write( i, &wr_len, envbuf );
	}

err:	free(envbuf);
	free(found);
	return ret;

}

U_BOOT_ENV_LOCATION(boot_magic) = {
	.location = ENVL_BOOT_MAGIC,
	.load = env_boot_magic_load,
	.save = env_save_ptr(env_boot_magic_save), 
	ENV_NAME("BOOT_MAGIC")
};
