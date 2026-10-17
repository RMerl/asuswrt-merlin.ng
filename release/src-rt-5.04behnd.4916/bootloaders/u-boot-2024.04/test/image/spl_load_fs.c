// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <blk.h>
#include <ext_common.h>
#include <ext4fs.h>
#include <fat.h>
#include <fs.h>
#include <memalign.h>
#include <spl.h>
#include <asm/io.h>
#include <linux/stat.h>
#include <test/spl.h>
#include <test/ut.h>

/**
 * create_ext2() - Create an "ext2" filesystem with a single file
 * @dst: The location of the new filesystem; MUST be zeroed
 * @size: The size of the file
 * @filename: The name of the file
 * @data_offset: Filled with the offset of the file data from @dst
 *
 * Budget mke2fs. We use 1k blocks (to reduce overhead) with a single block
 * group, which limits us to 8M of data. Almost every feature which increases
 * complexity (checksums, hash tree directories, etc.) is disabled. We do cheat
 * a little and use extents from ext4 to save having to deal with indirects, but
 * U-Boot doesn't care.
 *
 * If @dst is %NULL, nothing is copied.
 *
 * Return: The size of the filesystem in bytes
 */
static size_t create_ext2(void *dst, size_t size, const char *filename,
			  size_t *data_offset)
{
	u32 super_block = 1;
	u32 group_block = 2;
	u32 block_bitmap_block = 3;
	u32 inode_bitmap_block = 4;
	u32 inode_table_block = 5;
	u32 root_block = 6;
	u32 file_block = 7;

	u32 root_ino = EXT2_ROOT_INO;
	u32 file_ino = EXT2_BOOT_LOADER_INO;

	u32 block_size = EXT2_MIN_BLOCK_SIZE;
	u32 inode_size = sizeof(struct ext2_inode);

	u32 file_blocks = (size + block_size - 1) / block_size;
	u32 blocks = file_block + file_blocks;
	u32 inodes = block_size / inode_size;
	u32 filename_len = strlen(filename);
	u32 dirent_len = ALIGN(filename_len, sizeof(struct ext2_dirent)) +
			    sizeof(struct ext2_dirent);

	struct ext2_sblock *sblock = dst + super_block * block_size;
	struct ext2_block_group *bg = dst + group_block * block_size;
	struct ext2_inode *inode_table = dst + inode_table_block * block_size;
	struct ext2_inode *root_inode = &inode_table[root_ino - 1];
	struct ext2_inode *file_inode = &inode_table[file_ino - 1];
	struct ext4_extent_header *ext_block = (void *)&file_inode->b;
	struct ext4_extent *extent = (void *)(ext_block + 1);
	struct ext2_dirent *dot = dst + root_block * block_size;
	struct ext2_dirent *dotdot = dot + 2;
	struct ext2_dirent *dirent = dotdot + 2;
	struct ext2_dirent *last = ((void *)dirent) + dirent_len;

	/* Make sure we fit in one block group */
	if (blocks > block_size * 8)
		return 0;

	if (filename_len > EXT2_NAME_LEN)
		return 0;

	if (data_offset)
		*data_offset = file_block * block_size;

	if (!dst)
		goto out;

	sblock->total_inodes = cpu_to_le32(inodes);
	sblock->total_blocks = cpu_to_le32(blocks);
	sblock->first_data_block = cpu_to_le32(super_block);
	sblock->blocks_per_group = cpu_to_le32(blocks);
	sblock->fragments_per_group = cpu_to_le32(blocks);
	sblock->inodes_per_group = cpu_to_le32(inodes);
	sblock->magic = cpu_to_le16(EXT2_MAGIC);
	/* Done mostly so we can pretend to be (in)compatible */
	sblock->revision_level = cpu_to_le32(EXT2_DYNAMIC_REV);
	/* Not really accurate but it doesn't matter */
	sblock->first_inode = cpu_to_le32(EXT2_GOOD_OLD_FIRST_INO);
	sblock->inode_size = cpu_to_le32(inode_size);
	sblock->feature_incompat = cpu_to_le32(EXT4_FEATURE_INCOMPAT_EXTENTS);

	bg->block_id = cpu_to_le32(block_bitmap_block);
	bg->inode_id = cpu_to_le32(inode_bitmap_block);
	bg->inode_table_id = cpu_to_le32(inode_table_block);

	/*
	 * All blocks/inodes are in-use. I don't want to have to deal with
	 * endianness, so just fill everything in.
	 */
	memset(dst + block_bitmap_block * block_size, 0xff, block_size * 2);

	root_inode->mode = cpu_to_le16(S_IFDIR | 0755);
	root_inode->size = cpu_to_le32(block_size);
	root_inode->nlinks = cpu_to_le16(3);
	root_inode->blockcnt = cpu_to_le32(1);
	root_inode->flags = cpu_to_le32(EXT4_TOPDIR_FL);
	root_inode->b.blocks.dir_blocks[0] = root_block;

	file_inode->mode = cpu_to_le16(S_IFREG | 0644);
	file_inode->size = cpu_to_le32(size);
	file_inode->nlinks = cpu_to_le16(1);
	file_inode->blockcnt = cpu_to_le32(file_blocks);
	file_inode->flags = cpu_to_le32(EXT4_EXTENTS_FL);
	ext_block->eh_magic = cpu_to_le16(EXT4_EXT_MAGIC);
	ext_block->eh_entries = cpu_to_le16(1);
	ext_block->eh_max = cpu_to_le16(sizeof(file_inode->b) /
					sizeof(*ext_block) - 1);
	extent->ee_len = cpu_to_le16(file_blocks);
	extent->ee_start_lo = cpu_to_le16(file_block);

	/* I'm not sure we need these, but it can't hurt */
	dot->inode = cpu_to_le32(root_ino);
	dot->direntlen = cpu_to_le16(2 * sizeof(*dot));
	dot->namelen = 1;
	dot->filetype = FILETYPE_DIRECTORY;
	memcpy(dot + 1, ".", dot->namelen);

	dotdot->inode = cpu_to_le32(root_ino);
	dotdot->direntlen = cpu_to_le16(2 * sizeof(*dotdot));
	dotdot->namelen = 2;
	dotdot->filetype = FILETYPE_DIRECTORY;
	memcpy(dotdot + 1, "..", dotdot->namelen);

	dirent->inode = cpu_to_le32(file_ino);
	dirent->direntlen = cpu_to_le16(dirent_len);
	dirent->namelen = filename_len;
	dirent->filetype = FILETYPE_REG;
	memcpy(dirent + 1, filename, filename_len);

	last->direntlen = block_size - dirent_len;

out:
	return (size_t)blocks * block_size;
}

/**
 * create_fat() - Create a FAT32 filesystem with a single file
 * @dst: The location of the new filesystem; MUST be zeroed
 * @size: The size of the file
 * @filename: The name of the file
 * @data_offset: Filled with the offset of the file data from @dst
 *
 * Budget mkfs.fat. We use FAT32 (so I don't have to deal with FAT12) with no
 * info sector, and a single one-sector FAT. This limits us to 64k of data
 * (enough for anyone). The filename must fit in 8.3.
 *
 * If @dst is %NULL, nothing is copied.
 *
 * Return: The size of the filesystem in bytes
 */
static size_t create_fat(void *dst, size_t size, const char *filename,
			 size_t *data_offset)
{
	u16 boot_sector = 0;
	u16 fat_sector = 1;
	u32 root_sector = 2;
	u32 file_sector = 3;

	u16 sector_size = 512;
	u32 file_sectors = (size + sector_size - 1) / sector_size;
	u32 sectors = file_sector + file_sectors;

	char *ext;
	size_t filename_len, ext_len;
	int i;

	struct boot_sector *bs = dst + boot_sector * sector_size;
	struct volume_info *vi = (void *)(bs + 1);
	__le32 *fat = dst + fat_sector * sector_size;
	struct dir_entry *dirent = dst + root_sector * sector_size;

	/* Make sure we fit in the FAT */
	if (sectors > sector_size / sizeof(u32))
		return 0;

	ext = strchr(filename, '.');
	if (ext) {
		filename_len = ext - filename;
		ext++;
		ext_len = strlen(ext);
	} else {
		filename_len = strlen(filename);
		ext_len = 0;
	}

	if (filename_len > 8 || ext_len > 3)
		return 0;

	if (data_offset)
		*data_offset = file_sector * sector_size;

	if (!dst)
		goto out;

	bs->sector_size[0] = sector_size & 0xff;
	bs->sector_size[1] = sector_size >> 8;
	bs->cluster_size = 1;
	bs->reserved = cpu_to_le16(fat_sector);
	bs->fats = 1;
	bs->media = 0xf8;
	bs->total_sect = cpu_to_le32(sectors);
	bs->fat32_length = cpu_to_le32(1);
	bs->root_cluster = cpu_to_le32(root_sector);

	vi->ext_boot_sign = 0x29;
	memcpy(vi->fs_type, "FAT32   ", sizeof(vi->fs_type));

	memcpy(dst + 0x1fe, "\x55\xAA", 2);

	fat[0] = cpu_to_le32(0x0ffffff8);
	fat[1] = cpu_to_le32(0x0fffffff);
	fat[2] = cpu_to_le32(0x0ffffff8);
	for (i = file_sector; file_sectors > 1; file_sectors--, i++)
		fat[i] = cpu_to_le32(i + 1);
	fat[i] = cpu_to_le32(0x0ffffff8);

	for (i = 0; i < sizeof(dirent->nameext.name); i++) {
		if (i < filename_len)
			dirent->nameext.name[i] = toupper(filename[i]);
		else
			dirent->nameext.name[i] = ' ';
	}

	for (i = 0; i < sizeof(dirent->nameext.ext); i++) {
		if (i < ext_len)
			dirent->nameext.ext[i] = toupper(ext[i]);
		else
			dirent->nameext.ext[i] = ' ';
	}

	dirent->start = cpu_to_le16(file_sector);
	dirent->size = cpu_to_le32(size);

out:
	return sectors * sector_size;
}

typedef size_t (*create_fs_t)(void *, size_t, const char *, size_t *);

static int spl_test_fs(struct unit_test_state *uts, const char *test_name,
		       create_fs_t create)
{
	const char *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	struct blk_desc *dev_desc;
	char *data_write, *data_read;
	void *fs;
	size_t fs_size, fs_data, fs_blocks, data_size = SPL_TEST_DATA_SIZE;
	loff_t actread;

	fs_size = create(NULL, data_size, filename, &fs_data);
	ut_assert(fs_size);
	fs = calloc(fs_size, 1);
	ut_assertnonnull(fs);

	data_write = fs + fs_data;
	generate_data(data_write, data_size, test_name);
	ut_asserteq(fs_size, create(fs, data_size, filename, NULL));

	dev_desc = blk_get_devnum_by_uclass_id(UCLASS_MMC, 0);
	ut_assertnonnull(dev_desc);
	ut_asserteq(512, dev_desc->blksz);
	fs_blocks = fs_size / dev_desc->blksz;
	ut_asserteq(fs_blocks, blk_dwrite(dev_desc, 0, fs_blocks, fs));

	/* We have to use malloc so we can call virt_to_phys */
	data_read = malloc_cache_aligned(data_size);
	ut_assertnonnull(data_read);
	ut_assertok(fs_set_blk_dev_with_part(dev_desc, 0));
	ut_assertok(fs_read("/" CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
			    virt_to_phys(data_read), 0, data_size, &actread));
	ut_asserteq(data_size, actread);
	ut_asserteq_mem(data_write, data_read, data_size);

	free(data_read);
	free(fs);
	return 0;
}

static int spl_test_ext(struct unit_test_state *uts)
{
	return spl_test_fs(uts, __func__, create_ext2);
}
SPL_TEST(spl_test_ext, DM_FLAGS);

static int spl_test_fat(struct unit_test_state *uts)
{
	spl_fat_force_reregister();
	return spl_test_fs(uts, __func__, create_fat);
}
SPL_TEST(spl_test_fat, DM_FLAGS);

static bool spl_mmc_raw;

u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	return spl_mmc_raw ? MMCSD_MODE_RAW : MMCSD_MODE_FS;
}

static int spl_test_mmc_fs(struct unit_test_state *uts, const char *test_name,
			   enum spl_test_image type, create_fs_t create_fs,
			   bool blk_mode)
{
	const char *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	struct blk_desc *dev_desc;
	size_t fs_size, fs_data, img_size, img_data,
	       plain_size = SPL_TEST_DATA_SIZE;
	struct spl_image_info info_write = {
		.name = test_name,
		.size = type == LEGACY_LZMA ? lzma_compressed_size :
					      plain_size,
	}, info_read = { };
	struct disk_partition part = {
		.start = 1,
		.sys_ind = 0x83,
	};
	struct spl_image_loader *loader =
		SPL_LOAD_IMAGE_GET(0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
	struct spl_boot_device bootdev = {
		.boot_device = loader->boot_device,
	};
	void *fs;
	char *data, *plain;

	img_size = create_image(NULL, type, &info_write, &img_data);
	ut_assert(img_size);
	fs_size = create_fs(NULL, img_size, filename, &fs_data);
	ut_assert(fs_size);
	fs = calloc(fs_size, 1);
	ut_assertnonnull(fs);

	data = fs + fs_data + img_data;
	if (type == LEGACY_LZMA) {
		plain = malloc(plain_size);
		ut_assertnonnull(plain);
		generate_data(plain, plain_size, "lzma");
		memcpy(data, lzma_compressed, lzma_compressed_size);
	} else {
		plain = data;
		generate_data(plain, plain_size, test_name);
	}
	ut_asserteq(img_size, create_image(fs + fs_data, type, &info_write,
					   NULL));
	ut_asserteq(fs_size, create_fs(fs, img_size, filename, NULL));

	dev_desc = blk_get_devnum_by_uclass_id(UCLASS_MMC, 0);
	ut_assertnonnull(dev_desc);

	ut_asserteq(512, dev_desc->blksz);
	part.size = fs_size / dev_desc->blksz;
	ut_assertok(write_mbr_partitions(dev_desc, &part, 1, 0));
	ut_asserteq(part.size, blk_dwrite(dev_desc, part.start, part.size, fs));

	spl_mmc_raw = false;
	if (blk_mode)
		ut_assertok(spl_blk_load_image(&info_read, &bootdev, UCLASS_MMC,
					       0, 1));
	else
		ut_assertok(loader->load_image(&info_read, &bootdev));
	if (check_image_info(uts, &info_write, &info_read))
		return CMD_RET_FAILURE;
	if (type == LEGACY_LZMA)
		ut_asserteq(plain_size, info_read.size);
	ut_asserteq_mem(plain, phys_to_virt(info_write.load_addr), plain_size);

	if (type == LEGACY_LZMA)
		free(plain);
	free(fs);
	return 0;
}

static int spl_test_blk(struct unit_test_state *uts, const char *test_name,
			enum spl_test_image type)
{
	spl_fat_force_reregister();
	if (spl_test_mmc_fs(uts, test_name, type, create_fat, true))
		return CMD_RET_FAILURE;

	return spl_test_mmc_fs(uts, test_name, type, create_ext2, true);
}
SPL_IMG_TEST(spl_test_blk, LEGACY, DM_FLAGS);
SPL_IMG_TEST(spl_test_blk, LEGACY_LZMA, DM_FLAGS);
SPL_IMG_TEST(spl_test_blk, IMX8, DM_FLAGS);
SPL_IMG_TEST(spl_test_blk, FIT_EXTERNAL, DM_FLAGS);
SPL_IMG_TEST(spl_test_blk, FIT_INTERNAL, DM_FLAGS);

static int spl_test_mmc_write_image(struct unit_test_state *uts, void *img,
				    size_t img_size)
{
	struct blk_desc *dev_desc;
	size_t img_blocks;

	dev_desc = blk_get_devnum_by_uclass_id(UCLASS_MMC, 0);
	ut_assertnonnull(dev_desc);

	img_blocks = DIV_ROUND_UP(img_size, dev_desc->blksz);
	ut_asserteq(img_blocks, blk_dwrite(dev_desc,
					   CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR,
					   img_blocks, img));

	spl_mmc_raw = true;
	return 0;
}

static int spl_test_mmc(struct unit_test_state *uts, const char *test_name,
			enum spl_test_image type)
{
	spl_mmc_clear_cache();
	spl_fat_force_reregister();

	if (spl_test_mmc_fs(uts, test_name, type, create_ext2, false))
		return CMD_RET_FAILURE;

	if (spl_test_mmc_fs(uts, test_name, type, create_fat, false))
		return CMD_RET_FAILURE;

	return do_spl_test_load(uts, test_name, type,
				SPL_LOAD_IMAGE_GET(0, BOOT_DEVICE_MMC1,
						   spl_mmc_load_image),
				spl_test_mmc_write_image);
}
SPL_IMG_TEST(spl_test_mmc, LEGACY, DM_FLAGS);
SPL_IMG_TEST(spl_test_mmc, LEGACY_LZMA, DM_FLAGS);
SPL_IMG_TEST(spl_test_mmc, IMX8, DM_FLAGS);
SPL_IMG_TEST(spl_test_mmc, FIT_EXTERNAL, DM_FLAGS);
SPL_IMG_TEST(spl_test_mmc, FIT_INTERNAL, DM_FLAGS);
