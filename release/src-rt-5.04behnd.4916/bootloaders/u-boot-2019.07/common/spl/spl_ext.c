// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <ext4fs.h>
#include <errno.h>
#include <image.h>

int spl_load_image_ext(struct spl_image_info *spl_image,
		       struct blk_desc *block_dev, int partition,
		       const char *filename)
{
	s32 err;
	struct image_header *header;
	loff_t filelen, actlen;
	disk_partition_t part_info = {};

	header = spl_get_load_buffer(-sizeof(*header), sizeof(*header));

	if (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		return -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount(0);
	if (!err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: ext4fs mount err - %d\n", __func__, err);
#endif
		goto end;
	}

	err = ext4fs_open(filename, &filelen);
	if (err < 0) {
		puts("spl: ext4fs_open failed\n");
		goto end;
	}
	err = ext4fs_read((char *)header, 0, sizeof(struct image_header), &actlen);
	if (err < 0) {
		puts("spl: ext4fs_read failed\n");
		goto end;
	}

	err = spl_parse_image_header(spl_image, header);
	if (err < 0) {
		puts("spl: ext: failed to parse image header\n");
		goto end;
	}

	err = ext4fs_read((char *)spl_image->load_addr, 0, filelen, &actlen);

end:
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	if (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
#endif

	return err < 0;
}

#ifdef CONFIG_SPL_OS_BOOT
int spl_load_image_ext_os(struct spl_image_info *spl_image,
			  struct blk_desc *block_dev, int partition)
{
	int err;
	__maybe_unused loff_t filelen, actlen;
	disk_partition_t part_info = {};
	__maybe_unused char *file;

	if (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		return -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount(0);
	if (!err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: ext4fs mount err - %d\n", __func__, err);
#endif
		return -1;
	}
#if defined(CONFIG_SPL_ENV_SUPPORT)
	file = env_get("falcon_args_file");
	if (file) {
		err = ext4fs_open(file, &filelen);
		if (err < 0) {
			puts("spl: ext4fs_open failed\n");
			goto defaults;
		}
		err = ext4fs_read((void *)CONFIG_SYS_SPL_ARGS_ADDR, 0, filelen, &actlen);
		if (err < 0) {
			printf("spl: error reading image %s, err - %d, falling back to default\n",
			       file, err);
			goto defaults;
		}
		file = env_get("falcon_image_file");
		if (file) {
			err = spl_load_image_ext(spl_image, block_dev,
						 partition, file);
			if (err != 0) {
				puts("spl: falling back to default\n");
				goto defaults;
			}

			return 0;
		} else {
			puts("spl: falcon_image_file not set in environment, falling back to default\n");
		}
	} else {
		puts("spl: falcon_args_file not set in environment, falling back to default\n");
	}

defaults:
#endif

	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	if (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((void *)CONFIG_SYS_SPL_ARGS_ADDR, 0, filelen, &actlen);
	if (err < 0) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
#endif
		return -1;
	}

	return spl_load_image_ext(spl_image, block_dev, partition,
			CONFIG_SPL_FS_LOAD_KERNEL_NAME);
}
#else
int spl_load_image_ext_os(struct spl_image_info *spl_image,
			  struct blk_desc *block_dev, int partition)
{
	return -ENOSYS;
}
#endif
